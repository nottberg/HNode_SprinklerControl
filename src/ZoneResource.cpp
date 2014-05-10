#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include "REST/REST.hpp"
#include "ZoneManager.hpp"
#include "ZoneResource.hpp"

ZoneListResource::ZoneListResource()
{
    setURLPattern( "/zones", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ZoneListResource::~ZoneListResource()
{

}

void
ZoneListResource::setZoneManager( ZoneManager *zoneMgr )
{
    zoneManager = zoneMgr;
}

void 
ZoneListResource::restGet( RESTRequest *request )
{
    std::string rspData;

    std::cout << "ZoneListResource::restGet" << std::endl;

    rspData = "<hnode-zonelist>";

    printf( "ZoneListResource: zoneManager - 0x%lx\n", (unsigned long) zoneManager );

    if( zoneManager )
    {
        for( int index = 0; index < zoneManager->getZoneCount(); index++ )
        {
            Zone *zoneObj = zoneManager->getZoneByIndex( index );
            printf( "Output Zone: %d\n", index );
            rspData += "<zoneid>" + zoneObj->getID() + "</zoneid>";
        }
    }

    rspData += "</hnode-zonelist>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

ZoneResource::ZoneResource()
{
    setURLPattern( "/zones/{zoneid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT) ); 
}

ZoneResource::~ZoneResource()
{

}

void
ZoneResource::setZoneManager( ZoneManager *zoneMgr )
{
    zoneManager = zoneMgr;
}

void 
ZoneResource::restGet( RESTRequest *request )
{
    std::string zoneID;
    std::string rspData;
    Zone *zoneObj;

    if( request->getParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    zoneObj = zoneManager->getZoneByID( zoneID );

    if( zoneObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<zone id=\"";
    rspData += zoneID;
    rspData += "\">";

    rspData += "<name>";
    rspData += zoneObj->getName();
    rspData += "</name>";

    rspData += "<desc>";
    rspData += zoneObj->getDescription();
    rspData += "</desc>";

    rspData += "<map-path-id>";
    rspData += zoneObj->getMapPathID();
    rspData += "</map-path-id>";

    rspData += "</zone>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
ZoneResource::restPut( RESTRequest *request )
{
    std::string zoneID;
    std::string stateStr;
    Zone *zoneObj;
    bool result = true;

    if( request->getParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    zoneObj = zoneManager->getZoneByID( zoneID );

    if( zoneObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }    

    if( request->getParameter( "state", stateStr ) )
    {
        printf("Failed to find 'state' parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    if( stateStr == "on" )
    {
        printf( "Turn Zone On: %s\n", zoneID.c_str() );
        //result = zoneObj->setStateOn("REST Zone Interface");
    }
    else if( stateStr == "off" )
    {
        printf( "Turn Zone Off: %s\n", zoneID.c_str() );
        //result = zoneObj->setStateOff("REST Zone Interface");
    }
    else
    {
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    if( result == true )
    {
        request->setResponseCode( REST_HTTP_RCODE_INTERNAL_ERROR );
        request->sendResponse(); 
    }
    else
    {
        request->setResponseCode( REST_HTTP_RCODE_OK );
        request->sendResponse();
    }
}

ZoneDiagramResource::ZoneDiagramResource()
{
    setURLPattern( "/maps/zone", (REST_RMETHOD_T)(REST_RMETHOD_GET) ); 
}

ZoneDiagramResource::~ZoneDiagramResource()
{

}

void
ZoneDiagramResource::setZoneManager( ZoneManager *zoneMgr )
{
    zoneManager = zoneMgr;
}

void 
ZoneDiagramResource::restGet( RESTRequest *request )
{
    std::string rspData;
    Zone *zoneObj;

    RESTRepresentation *rspRep = request->getOutboundRepresentation();

    printf( "Zone Diagram Resource GET\n" );

    // Open the file with the map svg
    std::ifstream in( "/etc/hnode/irrigation/map/zone_map.svg", std::ios::in | std::ios::binary );

    // Return an error if the open failed.
    if( !in )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    printf( "Zone Diagram Resource -- Reading file\n" );

    // Determine the length of the data, 
    // and preallocate space in the string
    in.seekg(0, std::ios::end);
    rspData.resize(in.tellg());

    // Back up and read the data
    in.seekg(0, std::ios::beg);
    in.read(&rspData[0], rspData.size());

    // Close file
    in.close();

    // Send the data back to the requestor    
    rspRep->appendData( rspData.c_str(), rspData.size() );
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}


