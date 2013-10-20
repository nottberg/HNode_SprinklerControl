#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "ZoneManager.hpp"

Zone::Zone()
{

}

Zone::~Zone()
{

}

ZoneManager::ZoneManager()
{

}

ZoneManager::~ZoneManager()
{

}

void
ZoneManager::setConfigurationPath( std::string cfgPath )
{

}

bool
ZoneManager::loadConfiguration()
{
#if 0
    xmlDocPtr doc; /* the resulting document tree */

    doc = xmlReadFile( cfgPath.c_str(), NULL, 0 );
    if( doc == NULL ) 
    {
        fprintf( stderr, "Failed to parse %s\n", cfgPath.c_str() );
	    return true;
    }
    xmlFreeDoc(doc);
#endif
    return false;
}

bool 
ZoneManager::start()
{
    return false;
}


