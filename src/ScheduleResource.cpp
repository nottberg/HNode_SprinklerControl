#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include "REST/REST.hpp"
#include "ScheduleManager.hpp"
#include "ScheduleResource.hpp"


ScheduleZoneGroupListResource::ScheduleZoneGroupListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/zone-groups", mgr, SCH_ROTID_ZONEGROUP, "zone-group-list", "zg" )
{

}

ScheduleZoneGroupListResource::~ScheduleZoneGroupListResource()
{

}

ScheduleZoneGroupResource::ScheduleZoneGroupResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/zone-groups/{zonegroupid}", mgr, SCH_ROTID_ZONEGROUP )
{
    appendRelationship( "zone-group-list", "zonegroupid" );
}

ScheduleZoneGroupResource::~ScheduleZoneGroupResource()
{

}

ScheduleZoneRuleListResource::ScheduleZoneRuleListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/zone-groups/{zonegroupid}/members", mgr, SCH_ROTID_ZONERULE, "zone-rule-list", "zr" )
{
    appendRelationship( "zone-group-list", "zonegroupid" );
}

ScheduleZoneRuleListResource::~ScheduleZoneRuleListResource()
{

}

ScheduleZoneRuleResource::ScheduleZoneRuleResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/zone-groups/{zonegroupid}/members/{zoneid}", mgr, SCH_ROTID_ZONERULE )
{
    appendRelationship( "zone-group-list", "zonegroupid" );
    appendRelationship( "zone-rule-list", "zoneid" );
}

ScheduleZoneRuleResource::~ScheduleZoneRuleResource()
{

}

ScheduleTriggerGroupListResource::ScheduleTriggerGroupListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/trigger-groups", mgr, SCH_ROTID_TRIGGERGROUP, "trigger-group-list", "tg" )
{

}

ScheduleTriggerGroupListResource::~ScheduleTriggerGroupListResource()
{

}

ScheduleTriggerGroupResource::ScheduleTriggerGroupResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/trigger-groups/{triggergroupid}", mgr, SCH_ROTID_TRIGGERGROUP )
{
    appendRelationship( "trigger-group-list", "triggergroupid" );
}

ScheduleTriggerGroupResource::~ScheduleTriggerGroupResource()
{

}

ScheduleTriggerRuleListResource::ScheduleTriggerRuleListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/trigger-groups/{triggergroupid}/members", mgr, SCH_ROTID_TRIGGERRULE, "trigger-rule-list", "tr" )
{
    appendRelationship( "trigger-group-list", "triggergroupid" );
}

ScheduleTriggerRuleListResource::~ScheduleTriggerRuleListResource()
{

}

ScheduleTriggerRuleResource::ScheduleTriggerRuleResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/trigger-groups/{triggergroupid}/members/{triggerid}", mgr, SCH_ROTID_TRIGGERRULE )
{
    appendRelationship( "trigger-group-list", "triggergroupid" );
    appendRelationship( "trigger-rule-list", "triggerid" );
}

ScheduleTriggerRuleResource::~ScheduleTriggerRuleResource()
{

}

ScheduleRuleListResource::ScheduleRuleListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/rules", mgr, SCH_ROTID_EVENTRULE, "event-rule-list", "er")
{

}

ScheduleRuleListResource::~ScheduleRuleListResource()
{

}

ScheduleRuleResource::ScheduleRuleResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/rules/{ruleid}", mgr, SCH_ROTID_EVENTRULE )
{
    appendRelationship( "event-rule-list", "ruleid" );
}

ScheduleRuleResource::~ScheduleRuleResource()
{

}

ScheduleStatusResource::ScheduleStatusResource( ScheduleManager &mgr )
:RESTResourceRESTStatusProvider( "/schedule/status", mgr, SCHRSRC_STATID_STATUS )
{

}

ScheduleStatusResource::~ScheduleStatusResource()
{

}

ScheduleEventLogResource::ScheduleEventLogResource( ScheduleManager &mgr )
:RESTResourceRESTStatusProvider( "/schedule/event-log", mgr, SCHRSRC_STATID_EVENTLOG )
{

}

ScheduleEventLogResource::~ScheduleEventLogResource()
{

}

ScheduleCalendarResource::ScheduleCalendarResource( ScheduleManager &mgr )
:RESTResourceRESTStatusProvider( "/schedule/calendar", mgr, SCHRSRC_STATID_CALENDAR )
{

}

ScheduleCalendarResource::~ScheduleCalendarResource()
{

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

    printf( "ScheduleCalendarResource: schManager - 0x%lx\n", (long unsigned int) schManager );

    ScheduleDateTime startTime;
    startTime.getCurrentTime();
    startTime.subDays( 15 );

    ScheduleDateTime endTime;
    endTime.getCurrentTime();
    endTime.addDays( 15 );

    if( schManager )
    {
        ScheduleEventList *eventList;

        eventList = schManager->getPotentialEventsForPeriod( startTime, endTime );

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

