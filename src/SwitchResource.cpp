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
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
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

    rspData += "<state>";
    if( swObj->isStateOn() )
        rspData += "on";
    else
        rspData += "off";
    rspData += "</state>";

    rspData += "</switch>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
SwitchResource::restPut( RESTRequest *request )
{
    std::string swID;
    std::string stateStr;
    Switch *swObj;
    bool result = true;

    if( request->getParameter( "switchid", swID ) )
    {
        printf("Failed to look up switchid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
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

    if( request->getParameter( "state", stateStr ) )
    {
        printf("Failed to find 'state' parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    if( stateStr == "on" )
    {
        printf( "Turn Switch On: %s\n", swID.c_str() );
        result = swObj->setStateOn();
    }
    else if( stateStr == "off" )
    {
        printf( "Turn Switch Off: %s\n", swID.c_str() );
        result = swObj->setStateOff();
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

SwitchActivityLogResource::SwitchActivityLogResource()
{
    setURLPattern( "/switches/{switchid}/activity", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

SwitchActivityLogResource::~SwitchActivityLogResource()
{
}

void 
SwitchActivityLogResource::setSwitchManager( SwitchManager *swMgr )
{
    swManager = swMgr;
}

void 
SwitchActivityLogResource::restGet( RESTRequest *request )
{
    std::string swID;
    std::string rspData;
    Switch *swObj;

    if( request->getParameter( "switchid", swID ) )
    {
        printf("Failed to look up switchid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
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

    rspData = "<switch-log id=\"";
    rspData += swID;
    rspData += "\">";

    rspData += "<event>";
    rspData += "<tStamp>1383143803</tStamp>";
    rspData += "<rStamp>400</rStamp>";
    rspData += "<msg>Switched to On.</msg>";
    rspData += "<origin>192.168.1.2</origin>";
    rspData += "<old-state>off</old-state>";
    rspData += "<new-state>on</new-state>";
    rspData += "</event>";

    rspData += "<event>";
    rspData += "<tStamp>1383143843</tStamp>";
    rspData += "<rStamp>500</rStamp>";
    rspData += "<msg>Switched to Off.</msg>";
    rspData += "<origin>192.168.1.2</origin>";
    rspData += "<old-state>on</old-state>";
    rspData += "<new-state>off</new-state>";
    rspData += "</event>";

    rspData += "</switch-log>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}


