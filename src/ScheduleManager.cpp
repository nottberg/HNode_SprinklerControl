#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <string.h>

#include "ScheduleManager.hpp"

ScheduleDateTime::ScheduleDateTime()
{

}

ScheduleDateTime::~ScheduleDateTime()
{

}

void 
ScheduleDateTime::getCurrentTime()
{
    //dt = boost::gregorian::day_clock::universal_day();
    time = second_clock::universal_time();
}

void 
ScheduleDateTime::setTime( ScheduleDateTime &stime )
{
    time = stime.time;
}

void 
ScheduleDateTime::addDuration( time_duration &td )
{
    time += td;
}

long 
ScheduleDateTime::getHour()
{
    return time.time_of_day().hours();
}

long 
ScheduleDateTime::getMinute()
{
    return time.time_of_day().minutes();
}

long
ScheduleDateTime::getSecond()
{
    return time.time_of_day().seconds();
}

void 
ScheduleDateTime::addSeconds( long secValue )
{
    time = time + seconds( secValue );
}

void 
ScheduleDateTime::addMinutes( long minValue )
{
    printf("addMinutes: %s\n", to_iso_string( minutes( minValue ) ).c_str());
    time = time + minutes( minValue );
}

void 
ScheduleDateTime::addHours( long hourValue )
{
    time = time + hours( hourValue );
}

void 
ScheduleDateTime::addDays( long dayValue )
{
    time = time + days( dayValue );
}

void 
ScheduleDateTime::subSeconds( long secValue )
{
    time = time - seconds( secValue );
}

void 
ScheduleDateTime::subMinutes( long minValue )
{
    //printf("subMinutes: %s\n", to_iso_string( minutes( minValue ) ).c_str());
    time = time - minutes( minValue );
}

void 
ScheduleDateTime::subHours( long hourValue )
{
    time = time - hours( hourValue );
}

void 
ScheduleDateTime::subDays( long dayValue )
{
    time = time - days( dayValue );
}

void 
ScheduleDateTime::replaceTimeOfDay( ScheduleDateTime &replaceTime )
{
    ptime newTime( time.date(), replaceTime.time.time_of_day() );

    time = newTime; 
}

bool 
ScheduleDateTime::isBefore( ScheduleDateTime &cmpTime )
{
    return (time <= cmpTime.time);
}

bool 
ScheduleDateTime::isAfter( ScheduleDateTime &cmpTime )
{
    return (time >= cmpTime.time);
}

std::string
ScheduleDateTime::getSimpleString()
{
    return to_simple_string( time );
}

long
ScheduleDateTime::getDayOfWeek()
{
    return time.date().day_of_week();
}

std::string
ScheduleDateTime::getISOString()
{
    return to_iso_string( time );
} 

std::string
ScheduleDateTime::getExtendedISOString()
{
    return to_iso_extended_string( time );
} 
void 
ScheduleDateTime::setTimeFromISOString( std::string isoTime )
{
    time = from_iso_string( isoTime );
}

ScheduleTimeDuration::ScheduleTimeDuration()
{
    //td 
}

ScheduleTimeDuration::~ScheduleTimeDuration()
{

}

void
ScheduleTimeDuration::setFromString( std::string timeStr )
{
    td = duration_from_string( timeStr );
}

std::string
ScheduleTimeDuration::getISOString()
{
    return to_iso_string( td );
}

long 
ScheduleTimeDuration::asTotalSeconds()
{
    return td.total_seconds();
}

ScheduleTimePeriod::ScheduleTimePeriod()
{
    //td 
}

ScheduleTimePeriod::~ScheduleTimePeriod()
{

}

void
ScheduleTimePeriod::setFromStartAndEnd( ScheduleDateTime &start, ScheduleDateTime &end )
{
    //td 
}

ScheduleTimeIterator::ScheduleTimeIterator()
{
    ti = NULL; 
}

ScheduleTimeIterator::~ScheduleTimeIterator()
{
    if( ti )
        delete ti;
}

void
ScheduleTimeIterator::setFromStartAndInterval( ScheduleDateTime &start, ScheduleTimeDuration &interval )
{
    ti = new time_iterator( start.time, interval.td );
}

ScheduleEvent::ScheduleEvent()
{
    state     = SESTATE_IDLE;
    actionObj = NULL;
}

ScheduleEvent::~ScheduleEvent()
{

}

void 
ScheduleEvent::setId( std::string idStr )
{
    id = idStr;
}

std::string 
ScheduleEvent::getId()
{
    return id;
}

void 
ScheduleEvent::setTitle( std::string titleStr )
{
    title = titleStr;
}

std::string 
ScheduleEvent::getTitle()
{
    return title;
}

void 
ScheduleEvent::setStartTime( ScheduleDateTime &time )
{
    start = time;
}

void 
ScheduleEvent::getStartTime( ScheduleDateTime &time )
{
    time = start;
}

void 
ScheduleEvent::setEndTime( ScheduleDateTime &time )
{
    end = time;

    recycle.setTime( time );
    recycle.addMinutes( 2 );
}

void 
ScheduleEvent::getEndTime( ScheduleDateTime &time )
{
    time = end;
}

void 
ScheduleEvent::setAction( ScheduleAction *action )
{
    actionObj = action;
}

void 
ScheduleEvent::setReady()
{
    state = SESTATE_READY;
}

SESTATE 
ScheduleEvent::getState()
{
    return state;
}

bool 
ScheduleEvent::processCurrent( ScheduleDateTime &curTime )
{
    //printf( "processCurrent - %s:%s - %s - %s - %s -%s\n", getId().c_str(), getTitle().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );

    if( recycle.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_COMPLETE )
        {
            state = SESTATE_RECYCLE;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getTitle().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Recycle\n", getId().c_str(), getTitle().c_str() );               
        }   
    }

    if( end.isBefore( curTime ) )
    {
        // This event is complete
        if( state == SESTATE_RUNNING )
        {
            state = SESTATE_COMPLETE;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getTitle().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Completing\n", getId().c_str(), getTitle().c_str() );               

            if( actionObj != NULL )
            {
                actionObj->complete( curTime );
            }
        }
    }

    if( start.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_READY )
        {
            state = SESTATE_RUNNING;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getTitle().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Starting\n", getId().c_str(), getTitle().c_str() );   

            if( actionObj != NULL )
            {
                actionObj->start( curTime );
            }
            
        }
    } 

    // This event should be started
    if( state == SESTATE_RUNNING )
    {
        state = SESTATE_RUNNING;
        //printf( "processCurrent - %s:%s - Running\n", getId().c_str(), getTitle().c_str() ); 

        // Let the action perform polling steps.
        if( actionObj != NULL )
        {
            actionObj->poll( curTime );
        }      
    }

    // This event should be started
    //if( state == SESTATE_COMPLETE )
    //{
    //    printf( "processCurrent - %s:%s - Completed\n", getId().c_str(), getTitle().c_str() );               
    //}


    // No state change.
    return false;
}

void 
ScheduleEvent::processFinal()
{
    printf( "processCurrent - %s:%s - %s\n", getId().c_str(), getTitle().c_str(), recycle.getISOString().c_str() );

}

bool 
ScheduleEvent::isComplete()
{
    if( state == SESTATE_COMPLETE )
    {
        return true;
    }

    return false;
}

void 
ScheduleEvent::setComplete()
{
    state = SESTATE_COMPLETE;
}

bool 
ScheduleEvent::isRecyclable()
{
    if( state == SESTATE_RECYCLE )
    {
        return true;
    }

    return false;
}

ScheduleAction::ScheduleAction()
{

}

ScheduleAction::~ScheduleAction()
{

}

void 
ScheduleAction::start( ScheduleDateTime &curTime )
{

}

void 
ScheduleAction::poll( ScheduleDateTime &curTime )
{

}

void 
ScheduleAction::complete( ScheduleDateTime &curTime )
{

}

ZoneAction::ZoneAction()
{
    //std::vector<std::string> zoneIDList;
}

ZoneAction::~ZoneAction()
{
}

void 
ZoneAction::clearZoneList()
{
    zoneList.clear();
}

void 
ZoneAction::addZone( Zone *zone )
{
    zoneList.push_back( zone );
}

void 
ZoneAction::start( ScheduleDateTime &curTime )
{
    printf( "ZoneAction::start\n" );
    // Search through the switch list for the right ID
    for( std::vector<Zone *>::iterator it = zoneList.begin() ; it != zoneList.end(); ++it)
    {
        (*it)->setStateOn( "Schedule Event" );
    }    
}

void 
ZoneAction::poll( ScheduleDateTime &curTime )
{
}

void 
ZoneAction::complete( ScheduleDateTime &curTime )
{
    printf( "ZoneAction::complete\n" );
    // Search through the switch list for the right ID
    for( std::vector<Zone *>::iterator it = zoneList.begin() ; it != zoneList.end(); ++it)
    {
        (*it)->setStateOff( "Schedule Event" );
    }    
}

ScheduleRecurrenceRule::ScheduleRecurrenceRule()
{

}

ScheduleRecurrenceRule::~ScheduleRecurrenceRule()
{

}

ScheduleZoneRule::ScheduleZoneRule()
{

}

ScheduleZoneRule::~ScheduleZoneRule()
{

}

void 
ScheduleZoneRule::setZone( Zone *zonePtr )
{
    zone = zonePtr;
}

Zone * 
ScheduleZoneRule::getZone()
{
    return zone;
}

std::string 
ScheduleZoneRule::getZoneID()
{
    return zone->getID();
}

void 
ScheduleZoneRule::setDuration( ScheduleTimeDuration &td )
{
    duration = td;
}

ScheduleTimeDuration 
ScheduleZoneRule::getDuration()
{
    return duration;
}

void 
ScheduleZoneRule::start( ScheduleDateTime &curTime )
{
    printf( "ScheduleZoneRule::start\n" );

    zone->setStateOn( "Schedule Event" );
}

void 
ScheduleZoneRule::poll( ScheduleDateTime &curTime )
{
}

void 
ScheduleZoneRule::complete( ScheduleDateTime &curTime )
{
    printf( "ScheduleZoneRule::complete\n" );

    zone->setStateOff( "Schedule Event" );
}


ScheduleZoneGroup::ScheduleZoneGroup()
{

}

ScheduleZoneGroup::~ScheduleZoneGroup()
{

}

void 
ScheduleZoneGroup::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleZoneGroup::getID()
{
    return id;
}

void 
ScheduleZoneGroup::setZoneEventPolicy( SER_ZONE_EVENT_POLICY policy )
{
    zonePolicy = policy;
}

SER_ZONE_EVENT_POLICY 
ScheduleZoneGroup::getZoneEventPolicy()
{
    return zonePolicy;
}

void 
ScheduleZoneGroup::clearZoneList()
{
    zoneList.clear();
}

void 
ScheduleZoneGroup::addZone( Zone *zone )
{
    ScheduleZoneRule rule;

    rule.setZone( zone );
    zoneList.push_back( rule );
}

void 
ScheduleZoneGroup::setZoneDuration( std::string zoneID, ScheduleTimeDuration &td )
{
    for( std::vector<ScheduleZoneRule>::iterator it = zoneList.begin(); it != zoneList.end(); ++it )
    {
        if( it->getZone()->getID() == zoneID )
        {
            it->setDuration( td );
        }
    }
}

unsigned int 
ScheduleZoneGroup::getZoneRuleCount()
{
    return zoneList.size();
}

ScheduleZoneRule *
ScheduleZoneGroup::getZoneRuleByIndex( unsigned int index )
{
    if( index > zoneList.size() )
        return NULL;

    return &zoneList[ index ];
}

ScheduleZoneRule *
ScheduleZoneGroup::getZoneRuleByID( std::string zgID )
{
    ScheduleZoneRule *zgObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleZoneRule>::iterator it = zoneList.begin() ; it != zoneList.end(); ++it )
    {
        if( zgID == (it)->getZoneID() )
        {
            zgObj = &(*it);
        }
    }

    return zgObj;
}

void 
ScheduleZoneGroup::createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime, ScheduleDateTime &rearmTime )
{
    ScheduleDateTime eventTime;

    printf( "ScheduleZoneGroup -- start createZoneEvents\n");

    // Initial Time
    eventTime.setTime( curTime );
    rearmTime.setTime( curTime );

    printf( "ScheduleZoneGroup -- zone rule count: %d\n", (int)zoneList.size() );

    // Go through the zone list and schedule an event for each one
    for( std::vector<ScheduleZoneRule>::iterator it = zoneList.begin(); it != zoneList.end(); ++it )
    {
        ScheduleEvent *event = new ScheduleEvent;

        std::string eventName = it->getZoneID() + "-" + eventTime.getSimpleString();

        // Copy over identifying data
        event->setId( id );
        event->setTitle( eventName );

        // Set the start time
        event->setStartTime( eventTime );
    
        // FIX-ME calulate zone watering duration.
        printf("eventDuration: %d\n", (int)it->getDuration().asTotalSeconds() );
        eventTime.addSeconds( it->getDuration().asTotalSeconds() );

        // Set the end time.
        event->setEndTime( eventTime );

        // Add a second of padding between events
        eventTime.addSeconds( 1 );

        // Track the latest end time so that
        // the rule can be rearmed at that point.
        if( eventTime.isAfter( rearmTime ) )
        {
            rearmTime.setTime( eventTime );
        }

        // Create the zone action object
        event->setAction( &(*it) );

        // Mark ready and queue for execution.
        event->setReady();
        activeEvents.addEvent( event ); 
    }
}

ScheduleTimeTrigger::ScheduleTimeTrigger()
{
//    std::string      id;
//    SER_TT_SCOPE     scope;
//    ScheduleDateTime refTime;
}

ScheduleTimeTrigger::~ScheduleTimeTrigger()
{

}

void 
ScheduleTimeTrigger::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleTimeTrigger::getID()
{
    return id;
}

void 
ScheduleTimeTrigger::setScope( SER_TT_SCOPE scopeValue )
{
    scope = scopeValue;
}

SER_TT_SCOPE 
ScheduleTimeTrigger::getScope()
{
    return scope;
}

std::string 
ScheduleTimeTrigger::getScopeStr()
{

}

void 
ScheduleTimeTrigger::setRefTime( ScheduleDateTime &refTimeValue )
{
    refTime.setTime( refTimeValue );
}

ScheduleDateTime 
ScheduleTimeTrigger::getRefTime()
{
    return refTime;
}

ScheduleTriggerGroup::ScheduleTriggerGroup()
{
// std::string id;
// std::vector< ScheduleTimeTrigger > timeList;
}

ScheduleTriggerGroup::~ScheduleTriggerGroup()
{

}

void 
ScheduleTriggerGroup::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleTriggerGroup::getID()
{
    return id;
}

void 
ScheduleTriggerGroup::clearTimeTriggerList()
{
    timeList.clear();
}

void 
ScheduleTriggerGroup::addTimeTrigger( std::string id, SER_TT_SCOPE scope, ScheduleDateTime& refTime )
{
    ScheduleTimeTrigger ttObj;
 
    ttObj.setID( id );
    ttObj.setRefTime( refTime );
    ttObj.setScope( scope );

    timeList.push_back( ttObj );
}

void 
ScheduleTriggerGroup::removeTimeTrigger( std::string id )
{

}

unsigned int 
ScheduleTriggerGroup::getTimeTriggerCount()
{
    return timeList.size();
}

ScheduleTimeTrigger *
ScheduleTriggerGroup::getTimeTriggerByIndex( unsigned int index )
{
    if( index > timeList.size() )
        return NULL;

    return &timeList[ index ];
}

ScheduleTimeTrigger *
ScheduleTriggerGroup::getTimeTriggerByID( std::string ttID )
{
    ScheduleTimeTrigger *ttObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleTimeTrigger>::iterator it = timeList.begin() ; it != timeList.end(); ++it )
    {
        if( ttID == (it)->getID() )
        {
            ttObj = &(*it);
        }
    }

    return ttObj;
}

bool 
ScheduleTriggerGroup::checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime )
{
    ScheduleDateTime startTime;
    ScheduleDateTime prestartTime;
    ScheduleDateTime refTime;

    // Tmp get the refTime.
    refTime = timeList[0].getRefTime();

    // If we are not in the right day, then exit
    if( refTime.getDayOfWeek() != curTime.getDayOfWeek() )
        return false;

    // We are in the ballpark so calculate the local startTime
    // based on the curTime
    startTime.setTime( curTime );
    startTime.replaceTimeOfDay( refTime );

    // Schedule the event a couple of minutes before start time.
    prestartTime.setTime( startTime );
    prestartTime.subMinutes( 2 ); 

    //printf( "ScheduleEventRule -- dow - ref: %s\n", refTime.getISOString().c_str() );
    //printf( "ScheduleEventRule -- dow - prestart: %s\n", prestartTime.getISOString().c_str() );
    //printf( "ScheduleEventRule -- dow - start: %s\n", startTime.getISOString().c_str() );
    //printf( "ScheduleEventRule -- dow - cur: %s\n", curTime.getISOString().c_str() );

    // Schedule events a bit before their actual start times.
    if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
    {
        eventTime.setTime( startTime );
        return true;
    }

    return false;
}



ScheduleEventList::ScheduleEventList()
{

}

ScheduleEventList::~ScheduleEventList()
{

}

void 
ScheduleEventList::addEvent( ScheduleEvent *event )
{
    eventList.push_back( event );
}

unsigned int 
ScheduleEventList::getEventCount()
{
    return eventList.size();
}

ScheduleEvent *
ScheduleEventList::getEvent( unsigned int index )
{
    return eventList[ index ];
}

void
ScheduleEventList::reapEvents()
{
    // Find the elements that we want to remove
    std::vector<ScheduleEvent *>::iterator it = eventList.begin();
    while( it != eventList.end() )
    {
        // Check if this event is ready to go
        if( (*it)->isRecyclable() )
        {
            // Get a pointer to the element
            ScheduleEvent *finiEvent = *it;

            // Remove it from the list
            it = eventList.erase( it );

            // Let it perform any final processing
            finiEvent->processFinal();

            // Clean up the memory
            delete finiEvent;
        }
        else
        {
            // Next element
            it++;
        }

    }



}

ScheduleEventRule::ScheduleEventRule()
{
    enabled       = false;
    fireManually  = false;
    eventsPending = false;

    triggerGroup = NULL;
}

ScheduleEventRule::~ScheduleEventRule()
{

}

void 
ScheduleEventRule::setEnabled()
{
    enabled = true;
}

void 
ScheduleEventRule::clearEnabled()
{
    enabled = false;
}

bool 
ScheduleEventRule::getEnabled()
{
    return enabled;
}

void 
ScheduleEventRule::startManually()
{
    fireManually = true;
}

void 
ScheduleEventRule::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleEventRule::getID()
{
    return id;
}

void 
ScheduleEventRule::setName( std::string nameValue )
{
    name = nameValue;
}

std::string 
ScheduleEventRule::getName()
{
    return name;
}
 
void 
ScheduleEventRule::setDescription( std::string descValue )
{
    desc = descValue;
}

std::string 
ScheduleEventRule::getDescription()
{
    return desc;
}
       
void 
ScheduleEventRule::setURL( std::string urlValue )
{
    url = urlValue;
}

std::string 
ScheduleEventRule::getURL()
{
    return url;
}

/*
void 
ScheduleEventRule::setOcurrenceType( SER_OCURRENCE_TYPE typeValue )
{
    type = typeValue;
}

SER_OCURRENCE_TYPE 
ScheduleEventRule::getOcurrenceType()
{
    return type;
}
*/

#if 0
void 
ScheduleEventRule::setReferenceTime( ScheduleDateTime &time )
{
    ScheduleTriggerGroup* ttObj = new ScheduleTriggerGroup;

    std::string idStr = getID();
    idStr += "tmpTG";

    std::cout << "setRefTime idStr: " << idStr << std::endl;

    ttObj->setID( idStr );
    ttObj->addTimeTrigger( SER_TT_SCOPE_DAY, time );

    triggerGroup = ttObj;
}

void 
ScheduleEventRule::getReferenceTime( ScheduleDateTime &time )
{
    ScheduleTimeTrigger *ttObj;

    if( triggerGroup == NULL )
        return;

    ttObj = triggerGroup->getTimeTriggerByIndex( 0 );

    time = ttObj->getRefTime();
}
#endif

void 
ScheduleEventRule::setZoneGroup( ScheduleZoneGroup *zgObj )
{
    zoneGroup = zgObj;
}

std::string 
ScheduleEventRule::getZoneGroupID()
{
    if( zoneGroup == NULL )
        return "";

    return zoneGroup->getID();
}

void 
ScheduleEventRule::setTriggerGroup( ScheduleTriggerGroup *tgObj )
{
    triggerGroup = tgObj;
}

std::string 
ScheduleEventRule::getTriggerGroupID()
{
    if( triggerGroup == NULL )
        return "";

    return triggerGroup->getID();
}

#if 0
void 
ScheduleEventRule::setZoneEventPolicy( SER_ZONE_EVENT_POLICY policy )
{
    zonePolicy = policy;
}

SER_ZONE_EVENT_POLICY 
ScheduleEventRule::getZoneEventPolicy()
{
    return zonePolicy;
}

void 
ScheduleEventRule::clearZoneList()
{
    zoneRuleList.clear();
}

void 
ScheduleEventRule::addZoneRule( Zone *zone, ScheduleTimeDuration &td )
{
    ScheduleZoneRule rule;

    rule.setZone( zone );
    rule.setDuration( td );

    zoneRuleList.push_back( rule );
}
#endif
#if 0
void 
ScheduleEventRule::createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime )
{
    ScheduleDateTime eventTime;

    printf( "ScheduleEventRule -- start createZoneEvents\n");

    // Don't add new events if this rule already has events
    // scheduled.


    // Initial Time
    eventTime.setTime( curTime );

    printf( "ScheduleEventRule -- zone rule count: %d\n", zoneRuleList.size() );

    // Go through the zone list and schedule an event for each one
    for( std::vector<ScheduleZoneRule>::iterator it = zoneRuleList.begin(); it != zoneRuleList.end(); ++it )
    {
        ScheduleEvent *event = new ScheduleEvent;

        std::string eventName = name + "-" + it->getZoneID() + "-" + eventTime.getSimpleString();

        // Copy over identifying data
        event->setId( id );
        event->setTitle( eventName );

        // Set the start time
        event->setStartTime( eventTime );
    
        // FIX-ME calulate zone watering duration.
        printf("eventDuration: %d\n", it->getDuration().asTotalSeconds() );
        eventTime.addSeconds( it->getDuration().asTotalSeconds() );

        // Set the end time.
        event->setEndTime( eventTime );

        // Add a second of padding between events
        eventTime.addSeconds( 1 );

        // Create the zone action object
        ZoneAction *action = new ZoneAction();
        action->addZone( it->getZone() );
        event->setAction( action );

        // Mark ready and queue for execution.
        event->setReady();
        activeEvents.addEvent( event ); 
    }
}
#endif

void
ScheduleEventRule::updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime )
{
    char tmpStr[64];
    ScheduleDateTime eventTime;

    //printf( "ScheduleEventRule -- start updateActiveEvents\n");
    //printf( "ScheduleEventRule -- name: %s\n", name.c_str() );
    //printf( "ScheduleEventRule -- type: %d\n", type);

    // Only check the rule if it is enabled.
    if( enabled == false )
    {
        // Skip the rule, it wasn't enabled
        return;
    }

    // If the event was fired manually,
    // then ignore the start time and
    // just schedule the events
    if( fireManually )
    {
        printf( "ScheduleEventRule -- manual start: %s\n", name.c_str() );

        // Create the events
        zoneGroup->createZoneEvents( activeEvents, eventTime, rearmTime );

        // Clear the manual fire flag, since we only wanted to 
        // trigger it once.
        fireManually = false;

        // Done
        return;
    }

    // Make sure we have what we need for scheduling
    if( ( triggerGroup == NULL ) || ( zoneGroup == NULL ) )
    {
        return;
    }

    // If we already have events pending then
    // don't try to schedule them again.
    if( eventsPending == true )
    {
        // Check if we should rearm for next trigger
        if( rearmTime.isBefore( curTime ) )
        {
            // Rearm and check for event trigger
            eventsPending = false;
        }
        else
        {
            // Rule has not been rearmed after previous
            // event generation, so no further processing.
            return;
        }
    }

    // Check if this rule triggered
    if( triggerGroup->checkForTrigger( curTime, eventTime ) )
    {
        zoneGroup->createZoneEvents( activeEvents, eventTime, rearmTime );
        eventsPending = true;
    }

}

ScheduleManager::ScheduleManager()
{
    cfgPath = "/etc/hnode";  
    zoneMgr = NULL;
    nextID = 1;
}

ScheduleManager::~ScheduleManager()
{

}

void
ScheduleManager::setZoneManager( ZoneManager *zMgr )
{
    zoneMgr = zMgr;
}

void
ScheduleManager::setConfigurationPath( std::string cfgPathStr )
{
    cfgPath = cfgPathStr;
}

bool
ScheduleManager::getAttribute( xmlNode *elem, std::string attrName, std::string &result )
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
ScheduleManager::getChildContent( xmlNode *elem, std::string childName, std::string &result )
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
ScheduleManager::parseActionList( xmlDocPtr doc, xmlNode *ruleElem, ScheduleEventRule *ruleObj )
{
#if 0
    xmlNode *curElem;
    xmlNode *listElem;
    std::string tmpStr;
   
    printf( "ScheduleManager - start parseActionList\n" );

    // How to handle zones.
    ruleObj->setZoneEventPolicy( SER_ZEP_SEQUENCIAL );

    // Parse the elements under the rule list
    listElem = NULL;
    for( curElem = ruleElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "zone-rule-list" ) == 0 )
        {
            listElem = curElem;
        }
    }

    // Make sure we found a list element
    if( listElem == NULL )
    {
        return true;
    }

    // Parse the elements under the rule list
    for( curElem = listElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "zone-rule" ) == 0 )
        {
            std::string zoneid;
            Zone *zone;
            ScheduleTimeDuration td;
            
            // Get the zone id
            if( getChildContent( curElem, "zoneid", tmpStr ) == false )
            {
                printf( "ScheduleManager - zoneid: %s\n", tmpStr.c_str() );
                zone = zoneMgr->getZoneByID( tmpStr );
            }

            // Get the zone duration
            if( getChildContent( curElem, "duration", tmpStr ) == false )
            {
                printf( "ScheduleManager - duration: %s\n", tmpStr.c_str() );

                td.setFromString( tmpStr );
            }

            printf( "ScheduleManager - add zone rule\n" );

            ruleObj->addZoneRule( zone, td );
        }
    }
#endif
    return false;
}

unsigned int 
ScheduleManager::getZoneGroupCount()
{
    return zoneGroupList.size();
}

ScheduleZoneGroup *
ScheduleManager::getZoneGroupByIndex( unsigned int index )
{
    if( index > zoneGroupList.size() )
        return NULL;

    return zoneGroupList[ index ];
}

ScheduleZoneGroup *
ScheduleManager::getZoneGroupByID( std::string zgID )
{
    ScheduleZoneGroup *zgObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        if( zgID == (*it)->getID() )
        {
            zgObj = *it;
        }
    }

    return zgObj;
}

unsigned int 
ScheduleManager::getTriggerGroupCount()
{
    return triggerGroupList.size();
}

ScheduleTriggerGroup *
ScheduleManager::getTriggerGroupByIndex( unsigned int index )
{
    if( index > triggerGroupList.size() )
        return NULL;

    return triggerGroupList[ index ];
}

ScheduleTriggerGroup *
ScheduleManager::getTriggerGroupByID( std::string tgID )
{
    ScheduleTriggerGroup *tgObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        if( tgID == (*it)->getID() )
        {
            tgObj = *it;
        }
    }

    return tgObj;
}

unsigned int 
ScheduleManager::getEventRuleCount()
{
    return eventRuleList.size();
}

ScheduleEventRule *
ScheduleManager::getEventRuleByIndex( unsigned int index )
{
    if( index > eventRuleList.size() )
        return NULL;

    return eventRuleList[ index ];
}

ScheduleEventRule *
ScheduleManager::getEventRuleByID( std::string erID )
{
    ScheduleEventRule *erObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        if( erID == (*it)->getID() )
        {
            erObj = *it;
            break;
        }
    }

    return erObj;
}

ScheduleEventRule *
ScheduleManager::createNewEventRule()
{
    ScheduleEventRule *ruleObj;
    char idStr[128];

    // Allocate a new rule object
    ruleObj = new ScheduleEventRule;

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "er%d", nextID );
    ruleObj->setID( idStr );

    printf( "New idStr: %s\n", idStr );

    // Return the object
    return ruleObj;
}

void
ScheduleManager::freeNewEventRule( ScheduleEventRule *ruleObj )
{
    delete ruleObj;
}

bool 
ScheduleManager::addNewEventRule( ScheduleEventRule *ruleObj )
{
    printf( "ScheduleManager - addNewEventRule\n" );

    // Add the expander object to the list
    eventRuleList.push_back( ruleObj );

    // Save the new configuration
    saveConfiguration();
}

bool 
ScheduleManager::deleteEventRuleByID( std::string erID )
{
    ScheduleEventRule *erObj = NULL;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        if( erID == (*it)->getID() )
        {
            // Save away the object pointer
            erObj = *it;

            // Erase the element from the list
            eventRuleList.erase( it );

            // Get rid of the memory
            delete erObj;

            // Save the modified configuration
            saveConfiguration();

            // Success
            return false;   
        }
    }

    // Not found
    return true;
}

bool 
ScheduleManager::addRule( xmlDocPtr doc, xmlNode *ruleElem )
{
    xmlNode *gpioElem;
    xmlNode *curElem;
    xmlChar *attrValue;

    ScheduleEventRule *ruleObj;

    std::string tmpStr;
    std::string idStr;

    printf( "ScheduleManager - starting rule parse\n" );

    // Allocate a 
    ruleObj = new ScheduleEventRule;

    // Grab the required id attribute
    if( getAttribute( ruleElem, "id", idStr ) )
    {
        return true;
    }   

    printf( "ScheduleManager - rule id: %s\n", idStr.c_str() );
    ruleObj->setID( idStr );

    // Grab the required id attribute
    if( getAttribute( ruleElem, "name", tmpStr ) )
    {
        return true;
    }   

    printf( "ScheduleManager - rule name: %s\n", tmpStr.c_str() );
    ruleObj->setName( tmpStr );

    // See if this rule is enabled or not, if enabled attribute
    // is not provided then assume it is enabled for compatibility
    // reasons
    if( getAttribute( ruleElem, "enabled", tmpStr ) )
    {
        // The attribute was not found
        ruleObj->setEnabled();
    }
    else
    {
        // Set state according to 
        // configuration file.
        if( tmpStr == "true" )
            ruleObj->setEnabled();
        else
            ruleObj->clearEnabled();
    }

    // Get the reference time
    if( getChildContent( ruleElem, "zone-group-ref", tmpStr ) == true )
    {
        return true;
    }

    printf( "ScheduleManager - zone-group-ref: %s\n", tmpStr.c_str() );

    // Lookup the zone group by name
    ScheduleZoneGroup *zg = getZoneGroupByID( tmpStr );

    if( zg != NULL )
    {
        printf( "ScheduleManager - ZGObj: %lx\n", (unsigned long) zg );

        // Process zone list
        ruleObj->setZoneGroup( zg );
    }

    // Get the reference time
    if( getChildContent( ruleElem, "trigger-group-ref", tmpStr ) == true )
    {
        return true;
    }

    printf( "ScheduleManager - trigger-group-ref: %s\n", tmpStr.c_str() );

    // Lookup the zone group by name
    ScheduleTriggerGroup *tg = getTriggerGroupByID( tmpStr );

    if( tg != NULL )
    {
        printf( "ScheduleManager - TGObj: %lx\n", (unsigned long) tg );

        // Process zone list
        ruleObj->setTriggerGroup( tg );
    }

    // Get the description string.
    if( getChildContent( ruleElem, "desc", tmpStr ) == false )
    {
        ruleObj->setDescription( tmpStr );
    }

    printf( "ScheduleManager - push rule\n" );

    // Add the expander object to the list
    eventRuleList.push_back( ruleObj );

}

bool 
ScheduleManager::parseZoneRuleList( xmlDocPtr doc, xmlNode *zgElem, ScheduleZoneGroup *zgObj )
{
    xmlNode *curElem;
    xmlNode *listElem;
    std::string tmpStr;
   
    printf( "ScheduleManager - start parseZoneRuleList\n" );

    // Parse the elements under the rule list
    listElem = NULL;
    for( curElem = zgElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "zone-rule-list" ) == 0 )
        {
            listElem = curElem;
        }
    }

    // Make sure we found a list element
    if( listElem == NULL )
    {
        return true;
    }

    // Parse the elements under the rule list
    for( curElem = listElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "zone-rule" ) == 0 )
        {
            std::string zoneid;
            Zone *zone;
            ScheduleTimeDuration td;
            
            // Get the zone id
            if( getChildContent( curElem, "zoneid", tmpStr ) == false )
            {
                printf( "ScheduleManager - zoneid: %s\n", tmpStr.c_str() );
                zone = zoneMgr->getZoneByID( tmpStr );
            }

            // Get the zone duration
            if( getChildContent( curElem, "duration", tmpStr ) == false )
            {
                printf( "ScheduleManager - duration: %s\n", tmpStr.c_str() );

                td.setFromString( tmpStr );
            }

            printf( "ScheduleManager - add zone rule\n" );

            zgObj->addZone( zone );
            zgObj->setZoneDuration( zone->getID(), td );
        }
    }

    return false;
}

bool 
ScheduleManager::addZoneGroup( xmlDocPtr doc, xmlNode *zgElem )
{
    xmlNode *curElem;
    xmlChar *attrValue;

    ScheduleZoneGroup *zgObj;

    std::string tmpStr;
    std::string idStr;

    printf( "ScheduleManager - add Zone Group\n" );

    // Allocate a 
    zgObj = new ScheduleZoneGroup;

    // Grab the required id attribute
    if( getAttribute( zgElem, "id", idStr ) )
    {
        return true;
    }   

    printf( "ScheduleManager - zg id: %s\n", idStr.c_str() );
    zgObj->setID( idStr );

    // How to handle zones.
    zgObj->setZoneEventPolicy( SER_ZEP_SEQUENCIAL );

    parseZoneRuleList( doc, zgElem, zgObj );


/*
    // Grab the required id attribute
    if( getAttribute( ruleElem, "name", tmpStr ) )
    {
        return true;
    }   

    printf( "ScheduleManager - rule name: %s\n", tmpStr.c_str() );
    ruleObj->setName( tmpStr );

    // Get the rule type
    if( getAttribute( ruleElem, "type", tmpStr ) )
    {
        return true;
    }

    printf( "ScheduleManager - rule type: %s\n", tmpStr.c_str() );

    // Check if it is a recurring day type.
    if( tmpStr == "day" )
    {
        printf( "ScheduleManager - found day-of-week type rule\n" );

        // Assign the rule its type
        ruleObj->setOcurrenceType( SER_TYPE_DAY );

        // Get the reference time
        if( getChildContent( ruleElem, "reftime", tmpStr ) == true )
        {
            return true;
        }

        printf( "ScheduleManager - dow-rule - reftime: %s\n", tmpStr.c_str() );

        // Assign the reference time.
        ScheduleDateTime time;
        time.setTimeFromISOString( tmpStr );
        ruleObj->setReferenceTime( time );
        
        printf( "ScheduleManager - dow-rule - reftime: %s\n", time.getISOString().c_str() );

        // Process zone list
        parseActionList( doc, ruleElem, ruleObj );
    }

    // Get the description string.
    if( getChildContent( ruleElem, "desc", tmpStr ) == false )
    {
        ruleObj->setDescription( tmpStr );
    }

    printf( "ScheduleManager - push rule\n" );
*/
    // Add the expander object to the list
    zoneGroupList.push_back( zgObj );

}

bool 
ScheduleManager::addTriggerGroup( xmlDocPtr doc, xmlNode *tgElem )
{
    xmlNode *curElem;
    xmlChar *attrValue;
    xmlNode *listElem;

    ScheduleTriggerGroup *tgObj;

    std::string tmpStr;
    std::string idStr;

    printf( "ScheduleManager - add Trigger Group\n" );

    // Allocate a 
    tgObj = new ScheduleTriggerGroup;

    // Grab the required id attribute
    if( getAttribute( tgElem, "id", idStr ) )
    {
        return true;
    }   

    printf( "ScheduleManager - tg id: %s\n", idStr.c_str() );
    tgObj->setID( idStr );
  
    printf( "ScheduleManager - start parseTriggerList\n" );

    // Parse the elements under the rule list
    listElem = NULL;
    for( curElem = tgElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "day-time-trigger" ) == 0 )
        {
            idStr = "";

            // Grab the required id attribute
            if( getAttribute( curElem, "id", tmpStr ) == false )
            {
                idStr = tmpStr;
            }
 
            // Get the reference time
            if( getChildContent( curElem, "reftime", tmpStr ) == false )
            {
                printf( "ScheduleManager - dow-trigger - reftime: %s\n", tmpStr.c_str() );

                // Assign the reference time.
                ScheduleDateTime time;
                time.setTimeFromISOString( tmpStr );
                    
                printf( "ScheduleManager - dow-trigger - reftime: %s\n", time.getISOString().c_str() );
                printf( "ScheduleManager - add time trigger\n" );
                printf( "tgObj: 0x%lx\n", tgObj );
                printf( "ScheduleManager - trigger group id: %s\n", tgObj->getID().c_str() );

                tgObj->addTimeTrigger( idStr, SER_TT_SCOPE_DAY, time );
            }

        }
    }

    printf( "tgObj: 0x%lx\n", tgObj );
    printf( "ScheduleManager - add trigger group: %s\n", tgObj->getID().c_str() );

    // Add the expander object to the list
    triggerGroupList.push_back( tgObj );

}

bool
ScheduleManager::loadConfiguration()
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *ruleListElem;
    xmlNode    *zgListElem;
    xmlNode    *tgListElem;
    xmlNode    *curElem;

    std::string filePath;

    filePath = cfgPath + "/irrigation/schedule_config.xml";

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
	    return true;
    }

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Make sure it has the expected root tag
    if( strcmp( (char *)rootElem->name, "hnode-irrigation-schedule-cfg" ) != 0 )
    {
        fprintf( stderr, "Root tag didn't match expected: %s\n", rootElem->name );
	    return true;
    } 

    // Find the zone group list element
    zgListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "zone-group-list" ) == 0) ) 
        {
            printf("zone-group-list found\n");
            zgListElem = curElem;
            break;
        }
    }

    if( zgListElem != NULL )
    {
        // Parse the elements under the rule list
        for( curElem = zgListElem->children; curElem; curElem = curElem->next )
        {
            // We are only interested in the elements at this level
            if( curElem->type != XML_ELEMENT_NODE )
                continue;

            // Check for an zone group
            if( strcmp( (char *)curElem->name, "zone-group" ) == 0 )
            {
                addZoneGroup( doc, curElem );
            }
        }
    }

    // Find the trigger group list element
    tgListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "trigger-group-list" ) == 0) ) 
        {
            printf("trigger-group-list found\n");
            tgListElem = curElem;
            break;
        }
    }

    if( tgListElem != NULL )
    {
        // Parse the elements under the rule list
        for( curElem = tgListElem->children; curElem; curElem = curElem->next )
        {
            // We are only interested in the elements at this level
            if( curElem->type != XML_ELEMENT_NODE )
                continue;

            // Check for a trigger group
            if( strcmp( (char *)curElem->name, "trigger-group" ) == 0 )
            {
                addTriggerGroup( doc, curElem );
            }
        }
    }

    // Find the schedule rule list element
    ruleListElem = NULL;
    for( curElem = rootElem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, "rule-list" ) == 0) ) 
        {
            printf("rule-list found\n");
            ruleListElem = curElem;
            break;
        }
    }

    if( ruleListElem == NULL )
    {
        fprintf( stderr, "Could not find rule-list element\n" );
	    return true;
    } 

    // Parse the elements under the rule list
    for( curElem = ruleListElem->children; curElem; curElem = curElem->next )
    {
        // We are only interested in the elements at this level
        if( curElem->type != XML_ELEMENT_NODE )
            continue;

        // Check for an i2c expander
        if( strcmp( (char *)curElem->name, "rule" ) == 0 )
        {
            addRule( doc, curElem );
        }
    }


    // Free the config document
    xmlFreeDoc(doc);

    return false;
}

#define MY_ENCODING "ISO-8859-1"


bool
ScheduleManager::saveZoneGroup( ScheduleZoneGroup *zgObj )
{
    int rc;

    // Start an element named zone-group. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "zone-group" );
    if( rc < 0 ) 
        return true;

    // Add the id attribute
    rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "id", BAD_CAST zgObj->getID().c_str() );
    if( rc < 0 ) 
        return true;

    // Start an element named zone-rule-list. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "zone-rule-list" );
    if( rc < 0 ) 
        return true;

    // Parse the elements under the rule list
    for( int indx = 0; indx < zgObj->getZoneRuleCount(); indx++ )
    {
        ScheduleZoneRule *zoneRule = zgObj->getZoneRuleByIndex( indx );

        // Start an element named zone-rule. 
        rc = xmlTextWriterStartElement( writer, BAD_CAST "zone-rule" );
        if( rc < 0 ) 
           return true;
    
        // Start an element named zoneid. 
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "zoneid", "%s", zoneRule->getZoneID().c_str() );
        if( rc < 0 ) 
            return true;
        
        // Start an element named duration. 
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "duration", "%s", zoneRule->getDuration().getISOString().c_str() );
        if( rc < 0 ) 
            return true;

        // Close the element named zone-rule. 
        rc = xmlTextWriterEndElement( writer );
        if( rc < 0 ) 
            return true;
    }

    // Close the element named zone-rule-list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    // Close the element named zone-group. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;
}

bool
ScheduleManager::saveZoneGroupList()
{
    int rc;

    // Start an element named zone-group-list as child of hnode-irrigation-schedule-cfg. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "zone-group-list" );
    if( rc < 0 ) 
        return true;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        saveZoneGroup(*it);
    }

    // Close the element named zone-group-list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool 
ScheduleManager::saveTriggerGroup( ScheduleTriggerGroup *tgObj )
{
    int rc;

    // Start an element named zone-group. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "trigger-group" );
    if( rc < 0 ) 
        return true;

    // Add the id attribute
    rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "id", BAD_CAST tgObj->getID().c_str() );
    if( rc < 0 ) 
        return true;


    // Parse the elements under the rule list
    for( int indx = 0; indx < tgObj->getTimeTriggerCount(); indx++ )
    {
        ScheduleTimeTrigger *trigger = tgObj->getTimeTriggerByIndex( indx );

        // Start an element named zone-rule-list. 
        rc = xmlTextWriterStartElement( writer, BAD_CAST "day-time-trigger" );
        if( rc < 0 ) 
            return true;

        // Add the id attribute
        rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "id", BAD_CAST trigger->getID().c_str() );
        if( rc < 0 ) 
            return true;
    
        // Start an element named zoneid. 
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "reftime", "%s", trigger->getRefTime().getExtendedISOString().c_str() );
        if( rc < 0 ) 
            return true;

        // Close the element named zone-rule. 
        rc = xmlTextWriterEndElement( writer );
        if( rc < 0 ) 
            return true;
    }

    // Close the element named zone-group. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;
}

bool 
ScheduleManager::saveTriggerGroupList()
{
    int rc;

    // Start an element named zone-group-list as child of hnode-irrigation-schedule-cfg. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "trigger-group-list" );
    if( rc < 0 ) 
        return true;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        saveTriggerGroup(*it);
    }

    // Close the element named zone-group-list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool 
ScheduleManager::saveEventRule( ScheduleEventRule *erObj )
{
    int rc;

    // Start an element named zone-group. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rule" );
    if( rc < 0 ) 
        return true;

    // Add the id attribute
    rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "id", BAD_CAST erObj->getID().c_str() );
    if( rc < 0 ) 
        return true;

    // Add the id attribute
    rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "enabled", BAD_CAST erObj->getEnabled()? BAD_CAST "true": BAD_CAST "false" );
    if( rc < 0 ) 
        return true;

    // Add the id attribute
    rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "name", BAD_CAST erObj->getName().c_str() );
    if( rc < 0 ) 
        return true;

    // Start an element named zoneid. 
    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "desc", "%s", erObj->getDescription().c_str() );
    if( rc < 0 ) 
        return true;

    // Start an element named zoneid. 
    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "zone-group-ref", "%s", erObj->getZoneGroupID().c_str() );
    if( rc < 0 ) 
        return true;

    // Start an element named zoneid. 
    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "trigger-group-ref", "%s", erObj->getTriggerGroupID().c_str() );
    if( rc < 0 ) 
        return true;

    // Close the element named rule. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;
}

bool 
ScheduleManager::saveEventRuleList()
{
    int rc;

    // Start an element named zone-group-list as child of hnode-irrigation-schedule-cfg. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rule-list" );
    if( rc < 0 ) 
        return true;

    // Search through the switch list for the right ID
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        saveEventRule(*it);
    }

    // Close the element named zone-group-list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool
ScheduleManager::saveConfiguration()
{
    int rc;
    xmlChar *tmp;

    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *ruleListElem;
    xmlNode    *zgListElem;
    xmlNode    *tgListElem;
    xmlNode    *curElem;

    std::string filePath;

    filePath = cfgPath + "/irrigation/test_config.xml";

    // Create a new XmlWriter for filename, with no compression. 
    writer = xmlNewTextWriterFilename( filePath.c_str(), 0 );
    if( writer == NULL ) 
    {
        printf("testXmlwriterFilename: Error creating the xml writer\n");
        return true;
    }

    // Start the document with the xml default for the version,
    // encoding ISO 8859-1 and the default for the standalone
    // declaration. 
    rc = xmlTextWriterStartDocument( writer, NULL, MY_ENCODING, NULL );
    if( rc < 0 ) 
    {
        printf( "testXmlwriterFilename: Error at xmlTextWriterStartDocument\n" );
        return true;
    }

    // Start an element named hnode-irrigation-schedule-cfg. Since thist is the first
    // element, this will be the root element of the document. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "hnode-irrigation-schedule-cfg" );
    if( rc < 0 ) 
    {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return true;
    }

    if( saveZoneGroupList() == true )
        return true;
   
    if( saveTriggerGroupList() == true )
        return true;

    if( saveEventRuleList() == true )
        return true;
 
    // Close the element named hnode-irrigation-schedule-cfg. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
    {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return true;
    }

    // Close out the document
    rc = xmlTextWriterEndDocument( writer );
    if( rc < 0 ) 
    {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
        return true;
    }

    // Write the file
    xmlFreeTextWriter( writer );

    return false;
}

void 
ScheduleManager::processCurrentEvents( ScheduleDateTime &curTime )
{
    //printf( "processCurrentEvents: %s\n", curTime.getSimpleString().c_str() );
    //printf( "processCurrentEvents: %s\n", curTime.getISOString().c_str() );
    //printf( "hour: %d\n", curTime.getHour() );
    //printf( "minutes: %d\n", curTime.getMinute() );
    //printf( "seconds: %d\n", curTime.getSecond() );

    if( activeEvents.getEventCount() > 0 )
    {
        if( curTime.getSecond() % 10 == 0 )
        {
            printf( "processCurrentEvents - active: %d heartbeat(10sec): %s\n", activeEvents.getEventCount(), curTime.getISOString().c_str() );
        }
    }
    else
    {
        if( ( curTime.getSecond() == 0 ) && ( curTime.getMinute() == 30 || curTime.getMinute() == 0 )  )
        {
            printf( "processCurrentEvents - heartbeat(30min): %s\n", curTime.getISOString().c_str() );
        }
    }

    time_duration ztd = seconds(10);    

    // Check if the rules are going to generate any new events.
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin(); it != eventRuleList.end(); ++it )
    {
        (*it)->updateActiveEvents( activeEvents, curTime );
    }

    //printf( "ActiveEventCount: %d\n", activeEvents.getEventCount() );

    // Do processing for active rules
    for( unsigned int index = 0; index < activeEvents.getEventCount(); ++index )
    {
        ScheduleEvent *event = activeEvents.getEvent( index );

        // Perform processing for this event
        if( event->processCurrent( curTime ) )
        {
            // State changed so see if action is necessary
            switch( event->getState() )
            {
                default:
                break;
            }
        }
    }

    // Cleanup Events that finished processing
    activeEvents.reapEvents();

}

ScheduleEventList *
ScheduleManager::getActiveEvents()
{

}

ScheduleEventList *
ScheduleManager::getEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime )
{
    ScheduleEventList *eventList = new ScheduleEventList();
    ScheduleDateTime curTime; 

    // Walk through the times at minute intervals
    for( curTime.setTime( startTime ); curTime.isBefore( endTime ); curTime.addMinutes(1) )
    {
        // Check if the rules are going to generate any new events.
        for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin(); it != eventRuleList.end(); ++it )
        {
            (*it)->updateActiveEvents( *eventList, curTime );
        }
    }

    return eventList;
}

void 
ScheduleManager::freeScheduleEventList( ScheduleEventList *listPtr )
{

}



