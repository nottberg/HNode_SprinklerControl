#include <string>
#include <list>
#include <vector>
#include <map>
#include <string.h>

#include "Switch.hpp"
#include "SwitchManager.hpp"

#include "I2CExpander.hpp"
#include "MCP23008Expander.hpp"

SwitchManager::SwitchManager()
{
    //std::vector<Switch> gpioList;
    //std::vector<I2CExpander> i2cExpanderList;
    cfgPath = "/etc/hnode";
}

SwitchManager::~SwitchManager()
{
}

void 
SwitchManager::setConfigurationPath( std::string cfgPathStr )
{
    cfgPath = cfgPathStr;
}

bool
SwitchManager::getAttribute( xmlNode *elem, std::string attrName, std::string &result )
{
    xmlChar *attrValue;
    
    // Start with a clear result
    result.clear();

    // Get the type attribute
    attrValue = xmlGetProp( elem, (xmlChar *)attrName.c_str() );

    if( attrValue == NULL )
    {
        return true;
    }    

    result = (char*)attrValue;

    xmlFree( attrValue );

    return false;
}

bool 
SwitchManager::addI2CExpander( xmlDocPtr doc, xmlNode *devElem )
{
    xmlNode *gpioElem;
    xmlNode *curElem;
    xmlChar *attrValue;

    I2CExpander *expdObj;

    std::string typeStr;
    std::string idStr;

    // Get the type attribute
    if( getAttribute( devElem, "type", typeStr ) )
    {
        return true;
    }
    
    // Create the appropriate object based on device type
    if( typeStr == MCP23008Expander::getDeviceTypeStr() )
    {
        expdObj = new MCP23008Expander();
    }

    // Grab the required id attribute
    if( getAttribute( devElem, "id", idStr ) )
    {
        return true;
    }   

    expdObj->setID( idStr );

    // Let the I2C object parse the rest of the contents
    if( expdObj->setFromConfiguration( doc, devElem ) )
    {
        return true;
    }
    
    // Add the expander object to the list
    i2cExpanderList.push_back( expdObj );

    // Add a reference to all of the expanders 
    // switch object to our master list.
    for( int index = 0; index < expdObj->getSwitchCount(); index++ )
    {
        Switch *swObj = expdObj->getSwitchByIndex( index );
         
        switchList.push_back( swObj );
    }

#if 0
    // Find the device list element
    devListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( curElem->name, "gpio-device-list" ) == 0) ) 
        {
            printf("devListNode found\n");
            devListElem = curElem;
            break;
        }
    }

    if( devListElem == NULL )
    {
        fprintf( stderr, "Could not find devlist element\n" );
	    return true;
    } 

    // Parse the elements under the device list
    for( curElem = devListElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( curElem->name, "i2c-expander" ) == 0 )
        {

        }
    }

    xmlFreeDoc(doc);
#endif
    return false;
}

bool 
SwitchManager::loadConfiguration()
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *devListElem;
    xmlNode    *devElem;
    xmlNode    *gpioElem;
    xmlNode    *curElem;

    std::string filePath;

    filePath = cfgPath + "/switch/hnode_switch_config.xml";

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
	    return true;
    }

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Make sure it has the expected root tag
    if( strcmp( (char *)rootElem->name, "hnode-switch-cfg" ) != 0 )
    {
        fprintf( stderr, "Root tag didn't match expected: %s\n", rootElem->name );
	    return true;
    } 

    // Find the device list element
    devListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "switch-device-list" ) == 0) ) 
        {
            printf("devListNode found\n");
            devListElem = curElem;
            break;
        }
    }

    if( devListElem == NULL )
    {
        fprintf( stderr, "Could not find devlist element\n" );
	    return true;
    } 

    // Parse the elements under the device list
    for( curElem = devListElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "i2c-expander" ) == 0 )
        {
            addI2CExpander( doc, curElem );
        }
    }

    // Build the global list of Switch Objects
    //for( std::vector<I2CExpander *>::iterator it = i2cExpanderList.begin() ; it != i2cExpanderList.end(); ++it)
    //{
    //    for( int index = 0; index < (*it)->getSwitchCount(); index++ )
    //    {
    //        printf( "SwitchManager: adding switch - %d\n", index );
    //        Switch *swObj = (*it)->getSwitchByIndex( index );
    //        switchList.push_back( swObj );
    //    }
    //}

    // Free the config document
    xmlFreeDoc(doc);

    return false;
}

int 
SwitchManager::getSwitchCount()
{
    return switchList.size();
}

Switch *
SwitchManager::getSwitchByIndex( int index )
{
    return switchList[ index ];
}

Switch *
SwitchManager::getSwitchByID( std::string switchID )
{
    Switch *swObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<Switch *>::iterator it = switchList.begin() ; it != switchList.end(); ++it)
    {
        if( switchID == (*it)->getID() )
        {
            swObj = *it;
        }
    }

    return swObj;
}


