#include <string>
#include <iostream>

#include "REST/REST.hpp"
#include "SwitchManager.hpp"
#include "SwitchResource.hpp"

SwitchListResource::SwitchListResource()
{
    setURLPattern( "/switches", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

SwitchListResource::~SwitchListResource()
{

}

void
SwitchListResource::setSwitchManager( SwitchManager *swMgr )
{
    swManager = swMgr;
}

void 
SwitchListResource::restGet( RESTRequest *request )
{
    std::string rspData;

    std::cout << "SwitchListResource::restGet" << std::endl;

    rspData = "<hnode-swlist>";

    printf( "SwitchListResource: swManager - 0x%x\n", swManager );

    if( swManager )
    {
        for( int index = 0; index < swManager->getSwitchCount(); index++ )
        {
            Switch *swObj = swManager->getSwitchByIndex( index );
            printf( "Output Switch: %d\n", index );
            rspData += "<swid>" + swObj->getID() + "</swid>";
        }
    }

    rspData += "</hnode-swlist>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

SwitchResource::SwitchResource()
{
    setURLPattern( "/switches/{switchid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT) ); 
}

SwitchResource::~SwitchResource()
{

}

void
SwitchResource::setSwitchManager( SwitchManager *swMgr )
{
    swManager = swMgr;
}

void 
SwitchResource::restGet( RESTRequest *request )
{
    std::string swID;
    std::string rspData;
    Switch *swObj;

    if( request->getParameter( "switchid", swID ) )
    {
        printf("Failed to look up switchid parameter\n");
        return;
    }

    printf( "URL SwitchID: %s\n", swID.c_str() );

    swObj = swManager->getSwitchByID( swID );

    if( swObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<switch id=\"";
    rspData += swID;
    rspData += "\">";

    rspData += "<name>";
    rspData += swObj->getName();
    rspData += "</name>";

    rspData += "<desc>";
    rspData += swObj->getDescription();
    rspData += "</desc>";

    rspData += "</switch>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
SwitchResource::restPut( RESTRequest *request )
{

}

