#include <string.h>

#include "I2CExpander.hpp"

I2CExpander::I2CExpander()
{
    deviceIndex = 0;
    busAddress  = 0;
}

I2CExpander::~I2CExpander()
{

}

void 
I2CExpander::setI2CAddressing( int i2cDeviceIndex, int i2cBusAddress )
{
    deviceIndex = i2cDeviceIndex;
    busAddress  = i2cBusAddress;
}

int 
I2CExpander::getDeviceNumber()
{
    return deviceIndex;
}

int 
I2CExpander::getBusAddress()
{
    return busAddress;
}

bool 
I2CExpander::setFromConfiguration( xmlDocPtr doc, xmlNode *i2cElem )
{
    xmlNode *i2cAddrElem;
    xmlNode *switchMapElem;
    xmlNode *curElem;

    int devNum;
    int addrNum;

    // Find the address element
    i2cAddrElem = NULL;
    for( curElem = i2cElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "i2c-addr" ) == 0) ) 
        {
            printf("i2cAddr found\n");
            i2cAddrElem = curElem;
            break;
        }
    }

    if( i2cAddrElem == NULL )
    {
        fprintf( stderr, "Could not find i2cAddr element\n" );
	    return true;
    } 

    // Get bus address attributes
    if( getAttributeInt( i2cAddrElem, "devnum", devNum ) )
    {
        return true;
    }
    
    if( getAttributeInt( i2cAddrElem, "busaddr", addrNum ) )
    {
        return true;
    }

    // Setup the bus address information
    setI2CAddressing( devNum, addrNum );

    // Find the gpio Map
    switchMapElem = NULL;
    for( curElem = i2cElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "switch-map" ) == 0) ) 
        {
            printf("switch-map found\n");
            switchMapElem = curElem;
            break;
        }
    }

    if( switchMapElem != NULL )
    {
        setSwitchMapFromConfiguration( doc, switchMapElem );
    } 

    // Done
    return false;
}

bool
I2CExpander::getAttributeInt( xmlNode *elem, std::string attrName, int &result )
{
    xmlChar *attrValue;
    
    // Start with a clear result
    result = 0;

    // Get the type attribute
    attrValue = xmlGetProp( elem, (xmlChar *)attrName.c_str() );

    if( attrValue == NULL )
    {
        return true;
    }    

    result = strtol( (char*)attrValue, NULL, 0 );

    xmlFree( attrValue );

    return false;
}


