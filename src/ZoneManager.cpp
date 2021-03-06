#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

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

void 
Zone::setName( std::string nameValue )
{
    name = nameValue;
}

void 
Zone::setDescription( std::string descValue )
{
    desc = descValue;
}

void 
Zone::setMapPathID( std::string pathID )
{
    mapPathID = pathID;
}

void 
Zone::setDiagramLocate( std::string filename )
{
    diagramLocate = filename;
}

void 
Zone::setDiagramActive( std::string filename )
{
    diagramActive = filename;
}

std::string 
Zone::getID()
{
    return id;
}

std::string 
Zone::getName()
{
    return name;
}

std::string 
Zone::getDescription()
{
    return desc;
}

std::string 
Zone::getMapPathID()
{
    return mapPathID;
}


bool 
Zone::getDiagramLocate( std::string &diagramData )
{
    std::ifstream in;

    in.open( diagramLocate.c_str(), std::ios::in | std::ios::binary );

    if( !in )
    {
        return true;
    }

    in.seekg( 0, std::ios::end );
    diagramData.resize( in.tellg() );
    in.seekg( 0, std::ios::beg );
    in.read( &diagramData[0], diagramData.size() );
    in.close();

    return false;
}

bool
Zone::getDiagramActive( std::string &diagramData )
{
    std::ifstream in;

    in.open( diagramActive.c_str(), std::ios::in | std::ios::binary );

    if( !in )
    {
        return true;
    }

    in.seekg( 0, std::ios::end );
    diagramData.resize( in.tellg() );
    in.seekg( 0, std::ios::beg );
    in.read( &diagramData[0], diagramData.size() );
    in.close();

    return false;
}

void 
Zone::clearSwitchList()
{
    valveList.clear();
}

void 
Zone::addSwitch( Switch *swObj )
{
    valveList.push_back( swObj );
}

bool 
Zone::isStateOn()
{
    bool result = false;

    // Search through the switch list to see if any of the switches are on.
    for( std::vector<Switch *>::iterator it = valveList.begin() ; it != valveList.end(); ++it)
    {
        if( (*it)->isStateOn() == true )
        {
            result = true;
        }
    }    

    return result;
}

void 
Zone::setStateOn( std::string origin )
{
    printf( "Zone::setStateOn: %s\n", name.c_str() );

    // Search through the switch list for the right ID
    for( std::vector<Switch *>::iterator it = valveList.begin() ; it != valveList.end(); ++it)
    {
        (*it)->setStateOn( origin );
    }    
}

void 
Zone::setStateOff( std::string origin )
{
    printf( "Zone::setStateOff: %s\n", name.c_str() );

    // Search through the switch list for the right ID
    for( std::vector<Switch *>::iterator it = valveList.begin() ; it != valveList.end(); ++it)
    {
        (*it)->setStateOff( origin );
    }
}

ZoneManager::ZoneManager()
{
    cfgPath = "/etc/hnode";
    switchManager = NULL;
}

ZoneManager::~ZoneManager()
{

}

void 
ZoneManager::setSwitchManager( SwitchManager *sMgr )
{
    switchManager = sMgr;
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
ZoneManager::getChildContent( xmlNode *elem, std::string childName, std::string &result )
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

bool
ZoneManager::getDiagrams( xmlNode *elem, Zone *zoneObj )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlNode *diagramsElem;
    std::string filename;

    // Find the diagram element
    diagramsElem = NULL;
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "diagrams" ) == 0) ) 
        {
            diagramsElem = curElem;
            break;
        }
    }
 
    if(diagramsElem == NULL )
    {
        return true;
    }

    // Find the paths element
    for( curElem = diagramsElem->children; curElem; curElem = curElem->next ) 
    {
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        if( strcmp( (char *)curElem->name, "locate" ) == 0 ) 
        {
            contentValue = xmlNodeGetContent( curElem );
            filename = cfgPath + "/irrigation/zone_diagram/" + (char *)contentValue;
            zoneObj->setDiagramLocate( filename );
            xmlFree( contentValue );
        }
        else if( strcmp( (char *)curElem->name, "active" ) == 0 ) 
        {
            contentValue = xmlNodeGetContent( curElem );
            filename = cfgPath + "/irrigation/zone_diagram/" + (char *)contentValue;
            zoneObj->setDiagramActive( filename );
            xmlFree( contentValue );
        }
    }

    return false;
}

bool
ZoneManager::getValves( xmlNode *elem, Zone *zoneObj )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlNode *valvesElem;
    std::string result;

    // Find the diagram element
    valvesElem = NULL;
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "valve-list" ) == 0) ) 
        {
            valvesElem = curElem;
            break;
        }
    }
 
    if(valvesElem == NULL )
    {
        return true;
    }

    // Find the paths element
    for( curElem = valvesElem->children; curElem; curElem = curElem->next ) 
    {
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        if( strcmp( (char *)curElem->name, "switch-ref" ) == 0 ) 
        {
            contentValue = xmlNodeGetContent( curElem );
            result = (char*)contentValue;

            Switch *swObj = switchManager->getSwitchByID( result );
            zoneObj->addSwitch( swObj );

            xmlFree( contentValue );
        }
    }

    return false;
}

bool 
ZoneManager::addZone( xmlDocPtr doc, xmlNode *zoneElem )
{
    xmlNode *gpioElem;
    xmlNode *curElem;
    xmlChar *attrValue;

    Zone *zoneObj;

    std::string tmpStr;
    std::string idStr;

    // Allocate a 
    zoneObj = new Zone;

    // Grab the required id attribute
    if( getAttribute( zoneElem, "id", idStr ) )
    {
        return true;
    }   

    zoneObj->setID( idStr );

    // Grab the required id attribute
    if( getAttribute( zoneElem, "name", tmpStr ) )
    {
        return true;
    }   

    zoneObj->setName( tmpStr );

    // Get the description string.
    if( getChildContent( zoneElem, "desc", tmpStr ) == false )
    {
        zoneObj->setDescription( tmpStr );
    }

    // Get the map path.
    if( getChildContent( zoneElem, "map-path-id", tmpStr ) == false )
    {
        zoneObj->setMapPathID( tmpStr );
    }

    // 
    //getDiagrams( zoneElem, zoneObj );

    //
    getValves( zoneElem, zoneObj ); 

    // Add the expander object to the list
    zoneList.push_back( zoneObj );

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
    if( strcmp( (char *)rootElem->name, "hnode-irrigation-zone-cfg" ) != 0 )
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

    // Parse the elements under the zone list
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

int 
ZoneManager::getZoneCount()
{
    return zoneList.size();
}

Zone *
ZoneManager::getZoneByIndex( int index )
{
    return zoneList[ index ];
}

Zone *
ZoneManager::getZoneByID( std::string zoneID )
{
    Zone *zoneObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<Zone *>::iterator it = zoneList.begin() ; it != zoneList.end(); ++it)
    {
        if( zoneID == (*it)->getID() )
        {
            zoneObj = *it;
        }
    }

    return zoneObj;
}

/*
void 
ZoneManager::turnOnZoneById( std::string zoneID )
{
    Zone *zone = getZoneByID( zoneID );

    if( zone )
    {
        zone->setStateOn( 
        Switch *swObj = switchManager->getSwitchByID( zone-> );

        swObj->setStateOn( "zoneManager" ); 
    }
}

void 
ZoneManager::turnOffZoneById( std::string zoneID )
{
    Zone *zone = getZoneByID( zoneID );

    if( zone )
    {

    }
}
*/
