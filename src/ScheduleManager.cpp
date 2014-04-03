#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

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
ScheduleDateTime::subSeconds( long secValue )
{
    time = time - seconds( secValue );
}

void 
ScheduleDateTime::subMinutes( long minValue )
{
    printf("subMinutes: %s\n", to_iso_string( minutes( minValue ) ).c_str());
    time = time - minutes( minValue );
}

void 
ScheduleDateTime::subHours( long hourValue )
{
    time = time - hours( hourValue );
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

ScheduleDuration::ScheduleDuration()
{
    //td 
}

ScheduleDuration::~ScheduleDuration()
{

}

ScheduleEvent::ScheduleEvent()
{
    state = SESTATE_IDLE;
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
ScheduleEvent::setStartAction( ScheduleAction &action )
{
    startAction = action;
}

void 
ScheduleEvent::getStartAction( ScheduleAction &action )
{
    action = startAction;
}

void 
ScheduleEvent::setPollAction( ScheduleAction &action )
{
    pollAction = action;
}

void 
ScheduleEvent::getPollAction( ScheduleAction &action )
{
    action = pollAction;
}

void 
ScheduleEvent::setEndAction( ScheduleAction &action )
{
    endAction = action;
}

void 
ScheduleEvent::getEndAction( ScheduleAction &action )
{
    action = endAction;
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
    printf( "processCurrent - %s:%s - %s - %s - %s -%s\n", getId().c_str(), getTitle().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );

    if( recycle.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_COMPLETE )
        {
            state = SESTATE_RECYCLE;
            printf( "processCurrent - %s:%s - Recycle\n", getId().c_str(), getTitle().c_str() );               
        }   
    }

    if( end.isBefore( curTime ) )
    {
        // This event is complete
        if( state == SESTATE_RUNNING )
        {
            state = SESTATE_COMPLETE;
            printf( "processCurrent - %s:%s - Completing\n", getId().c_str(), getTitle().c_str() );               
        }
    }

    if( start.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_READY )
        {
            state = SESTATE_RUNNING;
            printf( "processCurrent - %s:%s - Starting\n", getId().c_str(), getTitle().c_str() );               
        }
    } 

    // This event should be started
    if( state == SESTATE_RUNNING )
    {
        state = SESTATE_RUNNING;
        printf( "processCurrent - %s:%s - Running\n", getId().c_str(), getTitle().c_str() );               
    }

    // This event should be started
    if( state == SESTATE_COMPLETE )
    {
        printf( "processCurrent - %s:%s - Completed\n", getId().c_str(), getTitle().c_str() );               
    }

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
    enabled      = false;
    fireManually = false;
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
ScheduleEventRule::setTitle( std::string titleValue )
{
    title = titleValue;
}

std::string 
ScheduleEventRule::getTitle()
{
    return title;
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

void 
ScheduleEventRule::setReferenceTime( ScheduleDateTime &time )
{
    refTime.setTime( time );
}

void 
ScheduleEventRule::getReferenceTime( ScheduleDateTime &time )
{
    time = refTime;
}

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
    zoneIDList.clear();
}

void 
ScheduleEventRule::addZoneId( std::string zoneId )
{
    zoneIDList.push_back( zoneId );
}

void 
ScheduleEventRule::createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime )
{
    ScheduleDateTime eventTime;

    // Don't add new events if this rule already has events
    // scheduled.


    // Initial Time
    eventTime.setTime( curTime );

    // Go through the zone list and schedule an event for each one
    for( std::vector<std::string>::iterator it = zoneIDList.begin(); it != zoneIDList.end(); ++it )
    {
        ScheduleEvent *event = new ScheduleEvent;

        // Copy over identifying data
        event->setId( id );
        event->setTitle( title );

        // Set the start time
        event->setStartTime( eventTime );
    
        // FIX-ME calulate zone watering duration.
        eventTime.addSeconds(30);

        // Set the end time.
        event->setEndTime( eventTime );

        // Mark ready and queue for execution.
        event->setReady();
        activeEvents.addEvent( event ); 
    }
}

void
ScheduleEventRule::updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime )
{
    char tmpStr[64];
    ScheduleDateTime eventTime;

    // If the event was fired manually,
    // then ignore the start time and
    // just schedule the events
    if( fireManually )
    {
        // Create the events
        createZoneEvents( activeEvents, eventTime );

        // Clear the manual fire flag, since we only wanted to 
        // trigger it once.
        fireManually = false;

        // Done
        return;
    }

    // Copy over the current time in case we need to do some math.
    eventTime.setTime( curTime );

    // Check if our event has fired.
    switch( type )
    {
        // Execute on specific days of the week.
        case SER_TYPE_DAY:
        {
            // If we are in the right day, then proceed
            if( refTime.getDayOfWeek() == curTime.getDayOfWeek() )
            {
                ScheduleDateTime startTime;
                ScheduleDateTime prestartTime;

                // We are in the ballpark so calculate the local startTime
                // based on the curTime
                startTime.setTime( curTime );
                startTime.replaceTimeOfDay( refTime );

                // Schedule the event a couple of minutes before start time.
                prestartTime.setTime( startTime );
                prestartTime.subMinutes( 2 ); 

                // Schedule events a bit before their actual start times.
                if( prestartTime.isBefore( curTime ) && refTime.isAfter( curTime ) )
                {
                    // Create the events
                    createZoneEvents( activeEvents, eventTime );
                 
                    // Remember that we created the events already
                    eventsPending = true;
                }

                // After the start time has passed then reset us so that
                // future events can be processed
                if( refTime.isAfter( curTime ) )
                {
                    // Free us up to schedule future events
                    eventsPending = false;
                }
            }
        }
        break;

        // Execute on an interval away from the 
        // start time.
        case SER_TYPE_INTERVAL:
        {

        }
        break;
    }

#if 0
    // See if we should schedule things
    if( ( ( curTime.getMinute() % 2 ) == 0 ) && ( curTime.getSecond() == 0 ) )
    {


        for( int zoneIndx = 0; zoneIndx < 4; zoneIndx++ )
        {
            ScheduleEvent *event = new ScheduleEvent;

            sprintf( tmpStr, "%s.%d", curTime.getISOString().c_str(), zoneIndx ); 
            event->setId( tmpStr );

            sprintf( tmpStr, "Water Zone %d", zoneIndx ); 
            event->setTitle( tmpStr );

            event->setStartTime( eventTime );
            
            eventTime.addSeconds(30);

            event->setEndTime( eventTime );

            event->setReady();

            activeEvents.addEvent( event ); 
        }
    }
#endif

}

ScheduleManager::ScheduleManager()
{
    cfgPath = "/etc/hnode";  

    ScheduleEventRule *rule = new ScheduleEventRule;
    eventRuleList.push_back( rule );
}

ScheduleManager::~ScheduleManager()
{

}

void
ScheduleManager::setConfigurationPath( std::string cfgPathStr )
{
    cfgPath = cfgPathStr;
}

bool
ScheduleManager::loadConfiguration()
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *zoneListElem;
    xmlNode    *curElem;

    std::string filePath;
#if 0
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
#endif
    return false;
}

void 
ScheduleManager::processCurrentEvents( ScheduleDateTime &curTime )
{
    printf( "processCurrentEvents: %s\n", curTime.getSimpleString().c_str() );
    printf( "processCurrentEvents: %s\n", curTime.getISOString().c_str() );
    printf( "hour: %d\n", curTime.getHour() );
    printf( "minutes: %d\n", curTime.getMinute() );
    printf( "seconds: %d\n", curTime.getSecond() );

    time_duration ztd = seconds(10);

    // Check if the rules are going to generate any new events.
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin(); it != eventRuleList.end(); ++it )
    {
        (*it)->updateActiveEvents( activeEvents, curTime );
    }

    printf( "ActiveEventCount: %d\n", activeEvents.getEventCount() );

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

}

void 
ScheduleManager::freeScheduleEventList( ScheduleEventList *listPtr )
{

}



