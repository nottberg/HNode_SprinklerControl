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
    //printf("subMinutes: %s\n", to_iso_string( minutes( minValue ) ).c_str());
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

void
ScheduleEventRule::updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime )
{
    char tmpStr[64];
    ScheduleDateTime eventTime;

    //printf( "ScheduleEventRule -- start updateActiveEvents\n");
    //printf( "ScheduleEventRule -- name: %s\n", name.c_str() );
    //printf( "ScheduleEventRule -- type: %d\n", type);

    // If the event was fired manually,
    // then ignore the start time and
    // just schedule the events
    if( fireManually )
    {
        printf( "ScheduleEventRule -- manual start: %s\n", name.c_str() );

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
            //printf( "ScheduleEventRule -- dow - %d - %d\n", refTime.getDayOfWeek(), curTime.getDayOfWeek() );

            // If we are in the right day, then proceed
            if( refTime.getDayOfWeek() == curTime.getDayOfWeek() )
            {
                ScheduleDateTime startTime;
                ScheduleDateTime prestartTime;

                // We are in the ballpark so calculate the local startTime
                // based on the curTime
                startTime.setTime( curTime );
                startTime.replaceTimeOfDay( refTime );

                // Use start time in this case
                eventTime.setTime( startTime );

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
                    if( eventsPending == false )
                    {
                        printf( "ScheduleEventRule -- time start: %s\n", name.c_str() );

                        // Create the events
                        createZoneEvents( activeEvents, eventTime );
                 
                        // Remember that we created the events already
                        eventsPending = true;
                    }
                }

                // After the start time has passed then reset us so that
                // future events can be processed
                if( startTime.isBefore( curTime ) )
                {
                    //printf( "ScheduleEventRule -- clear eventsPending\n" );

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
            // Back up one interval.

            // Find where the reference time falls 
            // between the previous interval and
            // current time.  

            // Using the localized reference time,
            // check if the current time falls
            // within the interval.

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
    zoneMgr = NULL;
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

    return false;
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

    // Add the expander object to the list
    eventRuleList.push_back( ruleObj );

}

bool
ScheduleManager::loadConfiguration()
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *ruleListElem;
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
            printf( "processCurrentEvents - active: %d periodic: %s\n", activeEvents.getEventCount(), curTime.getISOString().c_str() );
        }
    }
    else
    {
        if( curTime.getMinute() == 30 )
        {
            printf( "processCurrentEvents - periodic: %s\n", curTime.getISOString().c_str() );
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

}

void 
ScheduleManager::freeScheduleEventList( ScheduleEventList *listPtr )
{

}



