#include <string.h>

#include "Switch.hpp"

SwitchDevice::SwitchDevice()
{
}

SwitchDevice::~SwitchDevice()
{
}

void 
SwitchDevice::setID( std::string idStr )
{
    id = idStr;
}

std::string 
SwitchDevice::getID()
{
    return id;
}

void  
SwitchDevice::clearSwitchList()
{
    SwitchList.clear();
}

int   
SwitchDevice::appendSwitch( std::string defaultId, std::string addressStr )
{
    Switch swObj( this );

    swObj.setID( defaultId );
    swObj.setAddress( addressStr );

    printf( "AppendSwitch: %s, %s\n", defaultId.c_str(), addressStr.c_str() );

    SwitchList.push_back( swObj );

    return SwitchList.size();
}
 
int   
SwitchDevice::getSwitchCount()
{
    printf( "getSwitchCount: %ld\n", SwitchList.size() );

    return SwitchList.size();
}

Switch *
SwitchDevice::getSwitchByIndex(int Index)
{
    return &SwitchList[ Index ];
}

bool 
SwitchDevice::setSwitchFromConfiguration( xmlDocPtr doc, xmlNode *switchElem )
{
    int index;
    std::string idStr;
    std::string nameStr;
    std::string descStr;
    std::string tmpStr;

    Switch *swObj;

    // Get the index for the switch value.
    if( getAttributeInt( switchElem, "index", index ) )
    {
        return true;
    }

    printf( "switch index: %d", index );

    // Look up the Switch Record
    swObj = getSwitchByIndex( index );    

    if( swObj == NULL )
    {
        return true;
    }

    // Indicate that this switch object had a mapping entry
    swObj->setMapped();

    // Get the id string for this switch
    if( getAttribute( switchElem, "id", idStr ) == false )
    {
        swObj->setID( idStr );
    }

    // Get the name string for this switch
    if( getAttribute( switchElem, "name", nameStr ) == false )
    {
        swObj->setName( nameStr );
    }

    // Get the name string for this switch
    if( getAttribute( switchElem, "switchable", tmpStr ) == false )
    {
        if( tmpStr == "yes" )
        {
            swObj->setSwitchable();
        }
    }

    // Get the description string.
    if( getChildContent( switchElem, "desc", descStr ) == false )
    {
        swObj->setDescription( descStr );
    }

    // Done
    return false;
}

bool 
SwitchDevice::setSwitchMapFromConfiguration( xmlDocPtr doc, xmlNode *switchMapElem )
{
    xmlNode *curElem;

    // Find the address element
    for( curElem = switchMapElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "switch" ) == 0) ) 
        {
            setSwitchFromConfiguration( doc, curElem );
        }
    }

    // Done
    return false;
}

bool
SwitchDevice::getAttributeInt( xmlNode *elem, std::string attrName, int &result )
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

bool
SwitchDevice::getAttribute( xmlNode *elem, std::string attrName, std::string &result )
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
SwitchDevice::getChildContent( xmlNode *elem, std::string childName, std::string &result )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlNode *childElem;

    // Start with a clear result
    result.clear();

    // Find the address element
    childElem = NULL;
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, childName.c_str() ) == 0) ) 
        {
            childElem = curElem;
            break;
        }
    }
 
    if(childElem == NULL )
    {
        return true;
    }

    // Get the type attribute
    contentValue = xmlNodeGetContent( childElem );

    result = (char*)contentValue;

    xmlFree( contentValue );

    return false;
}

