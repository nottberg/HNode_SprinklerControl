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
    rspRep->appendData( rspData.c_str(), rspData.size() );

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

    if( request->getParameter( "zonegroupid", zgID ) )
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

/*
    rspData += "<name>";
    rspData += zgObj->getName();
    rspData += "</name>";

    rspData += "<desc>";
    rspData += zgObj->getDescription();
    rspData += "</desc>";
*/
    rspData += "</schedule-zone-group>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

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

    if( request->getParameter( "zonegroupid", zgID ) )
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
    rspRep->appendData( rspData.c_str(), rspData.size() );

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

    if( request->getParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    if( request->getParameter( "zoneid", zoneID ) )
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
/*
    rspData += "<desc>";
    rspData += zgObj->getDescription();
    rspData += "</desc>";
*/
    rspData += "</schedule-zone-rule>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}






ScheduleRuleListResource::ScheduleRuleListResource()
{
    setURLPattern( "/schedule/rules", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
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
    std::string rspData;

    std::cout << "ScheduleRuleListResource::restGet" << std::endl;

    rspData = "<hnode-schedule-rule-id-list>";

    if( schManager )
    {
        printf( "Event Rule count: %d\n", schManager->getEventRuleCount() );

        for( unsigned int index = 0; index < schManager->getEventRuleCount(); index++ )
        {
            ScheduleEventRule *erObj = schManager->getEventRuleByIndex( index );

            rspData += "<id>";
            rspData += erObj->getID();
            rspData += "</id>";
        }
    }

    rspData += "</hnode-schedule-rule-id-list>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}



ScheduleRuleResource::ScheduleRuleResource()
{
    setURLPattern( "/schedule/rules/{ruleid}", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
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
    std::string erID;
    std::string rspData;
    ScheduleEventRule *erObj;

    if( request->getParameter( "ruleid", erID ) )
    {
        printf("Failed to look up ruleid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL EventRuleID: %s\n", erID.c_str() );

    erObj = schManager->getEventRuleByID( erID );

    if( erObj == NULL )
    {
        request->setResponseCode( REST_HTTP_RCODE_NOT_FOUND );
        request->sendResponse();
        return;
    }

    rspData = "<schedule-event-rule id=\"";
    rspData += erID;
    rspData += "\">";

    rspData += "<enabled>";
    if( erObj->getEnabled() == true )
        rspData += "true";
    else
        rspData += "false";
    rspData += "</enabled>";

    rspData += "<name>";
    rspData += erObj->getName();
    rspData += "</name>";

    rspData += "<desc>";
    rspData += erObj->getDescription();
    rspData += "</desc>";

    rspData += "<url>";
    rspData += erObj->getURL();
    rspData += "</url>";

    rspData += "<zone-group-id>";
    rspData += erObj->getZoneGroupID();
    rspData += "</zone-group-id>";

    rspData += "<trigger-group-id>";
    rspData += erObj->getTriggerGroupID();
    rspData += "</trigger-group-id>";

    rspData += "</schedule-event-rule>";

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->appendData( rspData.c_str(), rspData.size() );

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

#if 0
    if( request->getParameter( "startTime", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    if( request->getParameter( "endTime", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }
#endif

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
    rspRep->appendData( rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

