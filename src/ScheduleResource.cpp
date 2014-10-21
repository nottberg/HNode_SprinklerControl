#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include "REST/REST.hpp"
#include "ScheduleManager.hpp"
#include "ScheduleResource.hpp"



ScheduleZoneGroupListResource::ScheduleZoneGroupListResource()
{
    setURLPattern( "/schedule/zone-groups", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleZoneGroupListResource::~ScheduleZoneGroupListResource()
{

}

void
ScheduleZoneGroupListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneGroupListResource::restGet( RESTRequest *request )
{
    std::string rspData;

    std::cout << "ScheduleZoneGroupListResource::restGet" << std::endl;

    rspData = "<hnode-schedule-zg-id-list>";

    if( schManager )
    {
        printf( "ZoneGroup count: %d\n", schManager->getZoneGroupCount() );

        for( unsigned int index = 0; index < schManager->getZoneGroupCount(); index++ )
        {
            ScheduleZoneGroup *zgObj = schManager->getZoneGroupByIndex( index );

            rspData += "<id>";
            rspData += zgObj->getID();
            rspData += "</id>";
        }
    }

    rspData += "</hnode-schedule-zg-id-list>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}


ScheduleZoneGroupResource::ScheduleZoneGroupResource()
{
    setURLPattern( "/schedule/zone-groups/{zonegroupid}", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleZoneGroupResource::~ScheduleZoneGroupResource()
{

}

void
ScheduleZoneGroupResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneGroupResource::restGet( RESTRequest *request )
{
    std::string zgID;
    std::string rspData;
    ScheduleZoneGroup *zgObj;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    zgObj = schManager->getZoneGroupByID( zgID );

    if( zgObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<schedule-zone-group id=\"";
    rspData += zgID;
    rspData += "\">";

    rspData += "</schedule-zone-group>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

ScheduleZoneRuleListResource::ScheduleZoneRuleListResource()
{
    setURLPattern( "/schedule/zone-groups/{zonegroupid}/members", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleZoneRuleListResource::~ScheduleZoneRuleListResource()
{

}

void
ScheduleZoneRuleListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneRuleListResource::restGet( RESTRequest *request )
{
    std::string zgID;
    std::string rspData;
    ScheduleZoneGroup *zgObj;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    zgObj = schManager->getZoneGroupByID( zgID );

    if( zgObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<schedule-zone-rule-list id=\"";
    rspData += zgID;
    rspData += "\">";

    for( unsigned int index = 0; index < zgObj->getZoneRuleCount(); index++ )
    {
        ScheduleZoneRule *zrObj = zgObj->getZoneRuleByIndex( index );

        rspData += "<id>";
        rspData += zrObj->getZoneID();
        rspData += "</id>";
    }
    
    rspData += "</schedule-zone-rule-list>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}


ScheduleZoneRuleResource::ScheduleZoneRuleResource()
{
    setURLPattern( "/schedule/zone-groups/{zonegroupid}/members/{zoneid}", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleZoneRuleResource::~ScheduleZoneRuleResource()
{

}

void
ScheduleZoneRuleResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneRuleResource::restGet( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    ScheduleZoneGroup *zgObj;
    ScheduleZoneRule  *zoneRule;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    zgObj = schManager->getZoneGroupByID( zgID );

    if( zgObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    zoneRule = zgObj->getZoneRuleByID( zoneID );

    if( zoneRule == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<schedule-zone-rule id=\"";
    rspData += zoneID;
    rspData += "\">";

    rspData += "<duration>";
    rspData += zoneRule->getDuration().asTotalSeconds();
    rspData += "</duration>";

    rspData += "</schedule-zone-rule>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}






ScheduleRuleListResource::ScheduleRuleListResource()
{
    setURLPattern( "/schedule/rules", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
}

ScheduleRuleListResource::~ScheduleRuleListResource()
{

}

void
ScheduleRuleListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleRuleListResource::restGet( RESTRequest *request )
{
    std::string        rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleRuleListResource::restGet" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateScheduleRuleListContent( objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );

    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }    

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
ScheduleRuleListResource::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *templateNode;
    RESTContentNode   *objNode;
    ScheduleEventRule *ruleObj;

    std::cout << "ScheduleRuleListResource::restPost" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = ScheduleEventRule::generateCreateTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        schManager->addNewEventRule( helper->getRootNode(), &ruleObj );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( ruleObj->getID() );
}


ScheduleRuleResource::ScheduleRuleResource()
{
    setURLPattern( "/schedule/rules/{ruleid}", (REST_RMETHOD_T)( REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE ) );
}

ScheduleRuleResource::~ScheduleRuleResource()
{

}

void
ScheduleRuleResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleRuleResource::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string erID;

    if( request->getURIParameter( "ruleid", erID ) )
    {
        printf("Failed to look up ruleid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL EventRuleID: %s\n", erID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateEventRuleContent( erID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
ScheduleRuleResource::restPut( RESTRequest *request )
{
    std::string          erID;
    RESTContentHelper    *helper;
    RESTContentNode      *templateNode;
    RESTContentNode      *objNode;

    std::cout << "ScheduleRuleListResource::restPut" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        if( request->getURIParameter( "ruleid", erID ) )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 0, "A valid id must be provided as part of the URL." ); 
            return;
        }

        printf( "ScheduleRuleResource - PUT: %s\n", erID.c_str() );

        // Generate a template for acceptable data
        templateNode = ScheduleEventRule::generateUpdateTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        schManager->updateEventRule( erID, helper->getRootNode() );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleRuleResource::restDelete( RESTRequest *request )
{
    std::string erID;
    std::string rspData;
    ScheduleEventRule *erObj;

    try
    {

        // extract the ruleid parameter
        if( request->getURIParameter( "ruleid", erID ) )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 0, "A valid id must be provided as part of the URL." );
            return;
        }

        printf( "ScheduleRuleResource - DELETE: %s\n", erID.c_str() );

        // Attempt the delete operation.
        schManager->deleteEventRuleByID( erID );

    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

ScheduleCalendarEventResource::ScheduleCalendarEventResource()
{
    setURLPattern( "/calendar/events", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleCalendarEventResource::~ScheduleCalendarEventResource()
{

}

void
ScheduleCalendarEventResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleCalendarEventResource::restGet( RESTRequest *request )
{
    std::string rspData;

    std::cout << "ScheduleCalendarResource::restGet" << std::endl;

    rspData = "<hnode-schedule-event-list>";

    printf( "ScheduleCalendarResource: schManager - 0x%x\n", schManager );

    ScheduleDateTime startTime;
    startTime.getCurrentTime();
    startTime.subDays( 15 );

    ScheduleDateTime endTime;
    endTime.getCurrentTime();
    endTime.addDays( 15 );

    if( schManager )
    {
        ScheduleEventList *eventList;

        eventList = schManager->getEventsForPeriod( startTime, endTime );

        if( eventList != NULL )
        {
            printf( "EventList count: %d\n", eventList->getEventCount() );

            for( int indx = 0; indx < eventList->getEventCount(); indx++ )
            {
                ScheduleEvent *event = eventList->getEvent( indx );

                rspData += "<schedule-event>";

                rspData += "<id>";
                rspData += event->getId();
                rspData += "</id>";
       
                rspData += "<title>";
                rspData += event->getTitle();
                rspData += "</title>";

                ScheduleDateTime eventTime;

                event->getStartTime( eventTime );
                rspData += "<start-time>";
                rspData += eventTime.getExtendedISOString();
                rspData += "</start-time>";

                event->getEndTime( eventTime );
                rspData += "<end-time>";
                rspData += eventTime.getExtendedISOString();
                rspData += "</end-time>";

                rspData += "</schedule-event>";
            }

            schManager->freeScheduleEventList( eventList );
        }
    }

    rspData += "</hnode-schedule-event-list>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

