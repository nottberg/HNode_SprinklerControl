#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string.h>

#include "ZoneManager.hpp"

Zone::Zone()
{

}

Zone::~Zone()
{

}

void
Zone::setID( std::string idValue )
{
    id = idValue;
}

ZoneManager::ZoneManager()
{

}

ZoneManager::~ZoneManager()
{

}

void
ZoneManager::setConfigurationPath( std::string cfgPathStr )
{
    cfgPath = cfgPathStr;
}

bool
ZoneManager::getAttribute( xmlNode *elem, std::string attrName, std::string &result )
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
ZoneManager::addZone( xmlDocPtr doc, xmlNode *zoneElem )
{
    xmlNode *gpioElem;
    xmlNode *curElem;
    xmlChar *attrValue;

    Zone *zoneObj;

    std::string typeStr;
    std::string idStr;

    // Allocate a 
    zoneObj = new Zone;

#if 0
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
#endif

    // Grab the required id attribute
    if( getAttribute( zoneElem, "id", idStr ) )
    {
        return true;
    }   

    zoneObj->setID( idStr );

#if 0
    // Let the I2C object parse the rest of the contents
    if( expdObj->setFromConfiguration( doc, devElem ) )
    {
        return true;
    }
#endif    
    // Add the expander object to the list
    zoneList.push_back( zoneObj );

#if 0
    // Add a reference to all of the expanders 
    // switch object to our master list.
    for( int index = 0; index < expdObj->getSwitchCount(); index++ )
    {
        Zone *zoneObj = expdObj->getSwitchByIndex( index );
         
        switchList.push_back( swObj );
    }
#endif
}

bool
ZoneManager::loadConfiguration()
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *zoneListElem;
    xmlNode    *curElem;

    std::string filePath;

    filePath = cfgPath + "/irrigation/zone_config.xml";

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
	    return true;
    }

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Make sure it has the expected root tag
    if( strcmp( (char *)rootElem->name, "hnode-irr-zone-cfg" ) != 0 )
    {
        fprintf( stderr, "Root tag didn't match expected: %s\n", rootElem->name );
	    return true;
    } 

    // Find the device list element
    zoneListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "zone-list" ) == 0) ) 
        {
            printf("zone-list found\n");
            zoneListElem = curElem;
            break;
        }
    }

    if( zoneListElem == NULL )
    {
        fprintf( stderr, "Could not find zone-list element\n" );
	    return true;
    } 

    // Parse the elements under the device list
    for( curElem = zoneListElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "zone" ) == 0 )
        {
            addZone( doc, curElem );
        }
    }


    // Free the config document
    xmlFreeDoc(doc);

    return false;
}

bool 
ZoneManager::start()
{
    return false;
}


