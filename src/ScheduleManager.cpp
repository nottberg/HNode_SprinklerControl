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

#include "ScheduleConfig.hpp"
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

#if 0
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
#endif

ScheduleRecurrenceRule::ScheduleRecurrenceRule()
{

}

ScheduleRecurrenceRule::~ScheduleRecurrenceRule()
{

}

ScheduleZoneRule::ScheduleZoneRule( RESTContentManager &objMgr )
    : objManager( objMgr )
{
    //ruleType = SZR_TYPE_NOTSET;
    ruleType = SZR_TYPE_FIXED_DURATION;
}

ScheduleZoneRule::~ScheduleZoneRule()
{

}

SZR_TYPE_T
ScheduleZoneRule::getType()
{
    return SZR_TYPE_NOTSET;
}

std::string 
ScheduleZoneRule::getTypeStr()
{
    switch( ruleType )
    {
        case SZR_TYPE_FIXED_DURATION:
            return "fixedduration";
        break;

        default:
        break;
    }

    return "notset";
}

std::string 
ScheduleZoneRule::getStaticTypeStr()
{
    return "notset";
}

std::string 
ScheduleZoneRule::getElementName()
{
    return "schedule-zone-rule";
}

RESTContentTemplate *
ScheduleZoneRule::generateContentTemplate()
{
    RESTContentTemplate *rtnNode;

    rtnNode = RESTContentHelperFactory::newContentTemplate();

    // Give the root element a tag name
    rtnNode->setAsObject( "schedule-zone-rule" );
    rtnNode->setTemplateID( SCH_ROTID_ZONERULE );

    // Required fields
    rtnNode->defineField( "type", true );
    rtnNode->defineField( "name", true );
    rtnNode->defineField( "zoneid", true );

    return rtnNode;
}

void
ScheduleZoneRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;
    ScheduleTimeDuration td;

    objCN->getField( "name", tmpStr );

    if( tmpStr.empty() == false )
    {
        name = tmpStr;
    }

    objCN->getField( "zoneid", tmpStr );

    if( tmpStr.empty() == false )
    {
        zoneID = tmpStr;
    }

    // Optional Fields
    objCN->getField( "type", tmpStr );

    if( tmpStr == "fixedduration" )
    {
        ruleType = SZR_TYPE_FIXED_DURATION;
    }

    // Optional Fields
    objCN->getField( "duration", tmpStr );

    if( tmpStr.empty() == false )
    {
        td.setFromString( tmpStr );
        setDuration( td );
    }
}

void
ScheduleZoneRule::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleZoneRule::setContentNodeFromFields - 1" << std::endl;

    // Create the root object
    objCN->setAsObject( "schedule-zone-rule" );
    objCN->setID( getID() );
    objCN->setField( "type", getTypeStr() );
    objCN->setField( "name", getName() );
    objCN->setField( "duration", getDuration().getISOString() );
    objCN->setField( "zoneid", zoneID );
}

void 
ScheduleZoneRule::setName( std::string nameValue )
{
    name = nameValue;
}

std::string 
ScheduleZoneRule::getName()
{
    return name;
}


void 
ScheduleZoneRule::setZoneID( std::string newZoneID )
{
    zoneID = newZoneID;
}

std::string 
ScheduleZoneRule::getZoneID()
{
    return zoneID;
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

    ( (ScheduleManager &) objManager ).setZoneStateOn( zoneID );
}

void 
ScheduleZoneRule::poll( ScheduleDateTime &curTime )
{
}

void 
ScheduleZoneRule::complete( ScheduleDateTime &curTime )
{
    printf( "ScheduleZoneRule::complete\n" );

    ( (ScheduleManager &) objManager ).setZoneStateOff( zoneID );
}



#if 0
SZRFixedDuration::SZRFixedDuration( RESTContentManager &objMgr )
    : ScheduleZoneRule( objMgr )
{

}

SZRFixedDuration::~SZRFixedDuration()
{

}

SZR_TYPE_T
SZRFixedDuration::getType()
{
    return SZR_TYPE_FIXED_DURATION;
}

std::string 
SZRFixedDuration::getTypeStr()
{
    return "fixedduration";
}

std::string 
SZRFixedDuration::getStaticTypeStr()
{
    return "fixedduration";
}

void
SZRFixedDuration::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;
    ScheduleTimeDuration td;

    // Call the super class version of this first.
    ScheduleZoneRule::setFieldsFromContentNode( objCN );

    // Optional Fields
    objCN->getField( "duration", tmpStr );

    if( tmpStr.empty() == false )
    {
        td.setFromString( tmpStr );
        setDuration( td );
    }

}

void
SZRFixedDuration::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "SZRFixedDuration::setContentNodeFromFields - 1" << std::endl;

    // Call the super class version of this first.
    ScheduleZoneRule::setContentNodeFromFields( objCN );

    // Fill in fields
    objCN->setField( "duration", getDuration().getISOString() );
}

bool 
SZRFixedDuration::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;

    objCN = new RESTContentNode;

    // Create the object representation
    SZRFixedDuration::setContentNodeFromFields( objCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

void 
SZRFixedDuration::setZone( Zone *zonePtr )
{
    zone = zonePtr;
}

Zone * 
SZRFixedDuration::getZone()
{
    return zone;
}

std::string 
SZRFixedDuration::getZoneID()
{
    return zone->getID();
}

void 
SZRFixedDuration::setDuration( ScheduleTimeDuration &td )
{
    duration = td;
}

ScheduleTimeDuration 
SZRFixedDuration::getDuration()
{
    return duration;
}

void 
SZRFixedDuration::start( ScheduleDateTime &curTime )
{
    printf( "SZRFixedDuration::start\n" );

    zone->setStateOn( "Schedule Event" );
}

void 
SZRFixedDuration::poll( ScheduleDateTime &curTime )
{
}

void 
SZRFixedDuration::complete( ScheduleDateTime &curTime )
{
    printf( "SZRFixedDuration::complete\n" );

    zone->setStateOff( "Schedule Event" );
}
#endif



ScheduleZoneGroup::ScheduleZoneGroup( RESTContentManager &objMgr )
    : objManager( objMgr )
{

}

ScheduleZoneGroup::~ScheduleZoneGroup()
{

}

std::string 
ScheduleZoneGroup::getElementName()
{
    return "schedule-zone-group";
}

RESTContentTemplate *
ScheduleZoneGroup::generateContentTemplate()
{
    RESTContentTemplate *rtnNode;

    rtnNode = RESTContentHelperFactory::newContentTemplate();

    // Give the root element a tag name
    rtnNode->setAsObject( "schedule-zone-group" );
    rtnNode->setTemplateID( SCH_ROTID_ZONEGROUP );

    // Required fields
    rtnNode->defineField( "name", true );
    rtnNode->defineField( "policy", true );

    return rtnNode;
}

void
ScheduleZoneGroup::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Required Fields
    if( objCN->getField( "name", tmpStr ) )
    {
        name = tmpStr;
    }
}

void
ScheduleZoneGroup::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleZoneGroup::setContentNodeFromFields - 1" << std::endl;

    // Create the root object
    objCN->setAsObject( "schedule-zone-group" );
    objCN->setID( getID() );
    objCN->setField( "policy", getZoneEventPolicyStr() );
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
ScheduleZoneGroup::setZoneEventPolicyFromStr( std::string value )
{
    
}

std::string 
ScheduleZoneGroup::getZoneEventPolicyStr()
{
    return "sequential";
}

unsigned int 
ScheduleZoneGroup::getZoneRuleCount()
{
#if 0
    return zoneList.size();
#endif
}

ScheduleZoneRule *
ScheduleZoneGroup::getZoneRuleByIndex( unsigned int index )
{
#if 0
    if( index > zoneList.size() )
        return NULL;

    return zoneList[ index ];
#endif
}

ScheduleZoneRule *
ScheduleZoneGroup::getZoneRuleByID( std::string zgID )
{
    return (ScheduleZoneRule *) objManager.getObjectByID( zgID );
}

void 
ScheduleZoneGroup::createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime, ScheduleDateTime &rearmTime )
{
    ScheduleDateTime eventTime;

    printf( "ScheduleZoneGroup -- start createZoneEvents\n");

    // Initial Time
    eventTime.setTime( curTime );
    rearmTime.setTime( curTime );

    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "zone-rule-list", ruleIDList );  

    printf( "ScheduleZoneGroup -- zone rule count: %d\n", (int)ruleIDList.size() );
    
    // Go through the zone list and schedule an event for each one
    for( std::vector< std::string >::iterator it = ruleIDList.begin(); it != ruleIDList.end(); ++it )
    {
        ScheduleZoneRule *zrObj = (ScheduleZoneRule *) objManager.getObjectByID( *it );

        ScheduleEvent *event = new ScheduleEvent;

        std::string eventName = zrObj->getID() + "-" + eventTime.getSimpleString();

        // Copy over identifying data
        event->setId( getID() );
        event->setTitle( eventName );

        // Set the start time
        event->setStartTime( eventTime );
    
        // FIX-ME calulate zone watering duration.
        printf("eventDuration: %d\n", (int)zrObj->getDuration().asTotalSeconds() );
        eventTime.addSeconds( zrObj->getDuration().asTotalSeconds() );

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
        event->setAction( zrObj );

        // Mark ready and queue for execution.
        event->setReady();
        activeEvents.addEvent( event ); 
    }
}

ScheduleTriggerRule::ScheduleTriggerRule( RESTContentManager &objMgr )
    : objManager( objMgr )
{
    ruleType = STR_TYPE_NOTSET;
//    std::string      id;
//    SER_TT_SCOPE     scope;
//    ScheduleDateTime refTime;
}

ScheduleTriggerRule::~ScheduleTriggerRule()
{

}

std::string 
ScheduleTriggerRule::getElementName()
{
    return "schedule-trigger-rule";
}

RESTContentTemplate *
ScheduleTriggerRule::generateContentTemplate()
{
    RESTContentTemplate *rtnNode;

    rtnNode = RESTContentHelperFactory::newContentTemplate();

    // Give the root element a tag name
    rtnNode->setAsObject( "schedule-trigger-rule" );

    // Required fields
    rtnNode->defineField( "type", true );

    // Optional fields
    rtnNode->defineField( "name", false );
    rtnNode->defineField( "scope", false );
    rtnNode->defineField( "reftime", false );

    return rtnNode;
}

void
ScheduleTriggerRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Required Fields
    if( objCN->getField( "name", tmpStr ) )
    {
        name = tmpStr;
    }

    if( objCN->getField( "type", tmpStr ) )
    {
        if( tmpStr == "time" )
            ruleType = STR_TYPE_TIME;
    }

    // Required Fields
    if( objCN->getField( "scope", tmpStr ) )
    {
        setScopeFromStr( tmpStr );
    }

    // Required Fields
    if( objCN->getField( "reftime", tmpStr ) )
    {
        ScheduleDateTime time;
        time.setTimeFromISOString( tmpStr );

        setRefTime( time );
    }

}

void
ScheduleTriggerRule::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleTriggerRule::setContentNodeFromFields - 1" << std::endl;

    // Fill in fields
    objCN->setAsObject( "schedule-trigger-rule" );
    objCN->setField( "id", getID() );
    objCN->setField( "type", getTypeStr() );
    objCN->setField( "name", getName() );
    objCN->setField( "scope", getScopeStr() );
    objCN->setField( "reftime", getRefTime().getISOString() );
}

STR_TYPE_T
ScheduleTriggerRule::getType()
{
    return ruleType; 
}

std::string 
ScheduleTriggerRule::getTypeStr()
{
    switch( ruleType )
    {
        case STR_TYPE_TIME:
            return "time";
        break;

        default:
        break;
    }

    return "notset";
}

std::string 
ScheduleTriggerRule::getStaticTypeStr()
{
    return "notset";
}

void 
ScheduleTriggerRule::setName( std::string value )
{
    name = value;
}

std::string 
ScheduleTriggerRule::getName()
{
    return name;
}

void 
ScheduleTriggerRule::setScope( SER_TT_SCOPE scopeValue )
{
    scope = scopeValue;
}

SER_TT_SCOPE 
ScheduleTriggerRule::getScope()
{
    return scope;
}

bool
ScheduleTriggerRule::setScopeFromStr( std::string scopeStr )
{
    if( scopeStr == SERScopeString[0] )
    {
        scope = SER_TT_SCOPE_NOTSET;
        return false;
    }
    else if( scopeStr == SERScopeString[1] )
    {
        scope = SER_TT_SCOPE_DAY;
        return false;
    }

    // String wasn't recognized
    return true;
}

std::string 
ScheduleTriggerRule::getScopeStr()
{
    return SERScopeString[scope];
}

void 
ScheduleTriggerRule::setRefTime( ScheduleDateTime &refTimeValue )
{
    refTime.setTime( refTimeValue );
}

ScheduleDateTime 
ScheduleTriggerRule::getRefTime()
{
    return refTime;
}

bool 
ScheduleTriggerRule::checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime )
{
    switch( ruleType )
    {
        case STR_TYPE_TIME:
        {
            ScheduleDateTime startTime;
            ScheduleDateTime prestartTime;

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
        }
        break;

        default:
        break;
    }

    // Nothing 
    return false;
}

#if 0
ScheduleTimeTrigger::ScheduleTimeTrigger( RESTContentManager &objMgr )
    : ScheduleTriggerRule( objMgr )
{
//    std::string      id;
//    SER_TT_SCOPE     scope;
//    ScheduleDateTime refTime;
}

ScheduleTimeTrigger::~ScheduleTimeTrigger()
{

}

STR_TYPE_T
ScheduleTimeTrigger::getType()
{
    return STR_TYPE_TIME;
}

std::string 
ScheduleTimeTrigger::getTypeStr()
{
    return "time";
}

std::string 
ScheduleTimeTrigger::getStaticTypeStr()
{
    return "time";
}

void
ScheduleTimeTrigger::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Call the super class
    ScheduleTriggerRule::setFieldsFromContentNode( objCN );

    // Required Fields
    if( objCN->getField( "scope", tmpStr ) )
    {
        setScopeFromStr( tmpStr );
    }

    // Required Fields
    if( objCN->getField( "reftime", tmpStr ) )
    {
        ScheduleDateTime time;
        time.setTimeFromISOString( tmpStr );

        setRefTime( time );
    }

}

void
ScheduleTimeTrigger::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleTimeTrigger::setContentNodeFromFields - 1" << std::endl;

    // Call the super class version of this first.
    ScheduleTriggerRule::setContentNodeFromFields( objCN );

    // Fill in fields
    objCN->setField( "scope", getScopeStr() );
    objCN->setField( "reftime", getRefTime().getISOString() );
}

bool 
ScheduleTimeTrigger::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;

    objCN = new RESTContentNode;

    // Create the object representation
    ScheduleTimeTrigger::setContentNodeFromFields( objCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
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

bool
ScheduleTimeTrigger::setScopeFromStr( std::string scopeStr )
{
    if( scopeStr == SERScopeString[0] )
    {
        scope = SER_TT_SCOPE_NOTSET;
        return false;
    }
    else if( scopeStr == SERScopeString[1] )
    {
        scope = SER_TT_SCOPE_DAY;
        return false;
    }

    // String wasn't recognized
    return true;
}

std::string 
ScheduleTimeTrigger::getScopeStr()
{
    return SERScopeString[scope];
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

bool 
ScheduleTimeTrigger::checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime )
{
    ScheduleDateTime startTime;
    ScheduleDateTime prestartTime;

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
#endif

ScheduleTriggerGroup::ScheduleTriggerGroup( RESTContentManager &objMgr )
    : objManager( objMgr )
{

}

ScheduleTriggerGroup::~ScheduleTriggerGroup()
{

}

std::string 
ScheduleTriggerGroup::getElementName()
{
    return "schedule-trigger-group";
}

RESTContentTemplate *
ScheduleTriggerGroup::generateContentTemplate()
{
    RESTContentTemplate *rtnNode;

    rtnNode = RESTContentHelperFactory::newContentTemplate();

    // Give the root element a tag name
    rtnNode->setAsObject( "schedule-trigger-group" );

    // Required fields
    rtnNode->defineField( "name", true );
    rtnNode->defineField( "policy", true );

    return rtnNode;
}

void
ScheduleTriggerGroup::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Required Fields
    if( objCN->getField( "name", tmpStr ) )
    {
        name = tmpStr;
    }
}

void
ScheduleTriggerGroup::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleTriggerGroup::setContentNodeFromFields - 1" << std::endl;

    // Create the root object
    objCN->setAsObject( "schedule-trigger-group" );
    objCN->setID( getID() );
    objCN->setField( "name", getName() );
}

void 
ScheduleTriggerGroup::setName( std::string value )
{
    name = value;
}

std::string 
ScheduleTriggerGroup::getName()
{
    return name;
}

unsigned int 
ScheduleTriggerGroup::getTriggerRuleCount()
{
//    return ruleList.size();
}

ScheduleTriggerRule *
ScheduleTriggerGroup::getTriggerRuleByIndex( unsigned int index )
{
#if 0
    if( index > ruleList.size() )
        return NULL;

    return ruleList[ index ];
#endif
}

ScheduleTriggerRule *
ScheduleTriggerGroup::getTriggerRuleByID( std::string ttID )
{
    ScheduleTriggerRule *ttObj = NULL;
#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleTriggerRule *>::iterator it = ruleList.begin() ; it != ruleList.end(); ++it )
    {
        if( ttID == (*it)->getID() )
        {
            ttObj = (*it);
        }
    }

    return ttObj;
#endif
}

bool 
ScheduleTriggerGroup::checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime )
{
    std::vector< std::string > idList;

    objManager.getIDListForRelationship( getID(), "trigger-rule-list", idList );  

    // Search through possible triggers, if any of them go off then we are done.
    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it )
    {
        ScheduleTriggerRule *tgObj = (ScheduleTriggerRule *) objManager.getObjectByID( *it );

        if( tgObj->checkForTrigger( curTime, eventTime ) == true )
        {
            // At least one triggered, return that.
            // Short circuit, no need to check the rest.
            return true;
        }
    }

    // Nothing triggered, return nothing to do.
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

ScheduleEventRule::ScheduleEventRule( RESTContentManager &objMgr )
    : objManager( objMgr )
{
    enabled       = false;
    fireManually  = false;
    eventsPending = false;
}

ScheduleEventRule::~ScheduleEventRule()
{

}

unsigned int
ScheduleEventRule::getObjType()
{
    return SCH_ROTID_EVENTRULE;
}

std::string 
ScheduleEventRule::getElementName()
{
    return "schedule-event-rule";
}

RESTContentTemplate *
ScheduleEventRule::generateContentTemplate()
{
    RESTContentTemplate *rtnNode;

    rtnNode = RESTContentHelperFactory::newContentTemplate();

    // Give the root element a tag name
    rtnNode->setAsObject( "schedule-event-rule" );

    // Required fields
    rtnNode->defineField( "name", true );
    rtnNode->defineField( "desc", true );
    rtnNode->defineField( "zone-group-id", true );
    rtnNode->defineField( "trigger-group-id", true );

    // Optional fields
    rtnNode->defineField( "enabled", false );

    return rtnNode;
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
ScheduleEventRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Required Fields
    if( objCN->getField( "name", tmpStr ) )
    {
        name = tmpStr;
    }

    if( objCN->getField( "desc", tmpStr ) )
    {
        desc = tmpStr;
    }

    if( objCN->getField( "zone-group-id", tmpStr ) )
    {
        zoneGroupID = tmpStr;
    }

    if( objCN->getField( "trigger-group-id", tmpStr ) )
    {
        triggerGroupID = tmpStr;
    }

    // Optional Fields
    objCN->getField( "enabled", tmpStr );

    if( tmpStr.empty() )
    {
        // The attribute was not found
        enabled = true;
    }
    else
    {
        // Set state according to 
        // configuration file.
        if( tmpStr == "true" )
            enabled = true;
        else
            enabled = false;
    }

}

void
ScheduleEventRule::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleEventRule::setContentNodeFromFields - 1" << std::endl;

    // Create the root object
    objCN->setAsObject( "schedule-event-rule" );
    objCN->setID( getID() );
    objCN->setField( "name", getName() );
    objCN->setField( "desc", getDescription() );
    objCN->setField( "enabled", enabled ? "true" : "false" );
    objCN->setField( "url", getURL() );
    objCN->setField( "zone-group-id", zoneGroupID );
    objCN->setField( "trigger-group-id", triggerGroupID );
}

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

    // Lookup and validate the zone-group and trigger-group references
    ScheduleZoneGroup *zoneGroup = (ScheduleZoneGroup *) objManager.getObjectByID( zoneGroupID );
    ScheduleTriggerGroup *triggerGroup = (ScheduleTriggerGroup *) objManager.getObjectByID( triggerGroupID );

    if( ( zoneGroup == NULL ) || ( triggerGroup == NULL ) )
    {
        // Skip the rule, zone and/or trigger groups were not valid.
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
ScheduleManager::setZoneStateOn( std::string zoneID )
{
    Zone *zone = NULL;

    if( zoneMgr == NULL )
    {
        return;
    }

    zone = zoneMgr->getZoneByID( zoneID );

    if( zone == NULL )
    {
        return;
    }

    zone->setStateOn( "ScheduleManager" );
}

void 
ScheduleManager::setZoneStateOff( std::string zoneID )
{
    Zone *zone = NULL;

    if( zoneMgr == NULL )
    {
        return;
    }

    zone = zoneMgr->getZoneByID( zoneID );

    if( zone == NULL )
    {
        return;
    }

    zone->setStateOff( "ScheduleManager" );
}

void
ScheduleManager::setConfigurationPath( std::string cfgPathStr )
{
    cfgPath = cfgPathStr;
}

unsigned int 
ScheduleManager::getTriggerGroupCount()
{
    return getObjectCountByType( SCH_ROTID_TRIGGERGROUP );
}

ScheduleTriggerGroup *
ScheduleManager::getTriggerGroupByIndex( unsigned int index )
{
    std::vector< RESTContentNode* > objList;

    getObjectVectorByType( SCH_ROTID_TRIGGERGROUP, objList );

    if( index > objList.size() )
        return NULL;

    return (ScheduleTriggerGroup *) objList[ index ];
}

ScheduleTriggerGroup *
ScheduleManager::getTriggerGroupByID( std::string tgID )
{
    ScheduleTriggerGroup *tgObj = NULL;

    tgObj = ( ScheduleTriggerGroup* ) getObjectByID( tgID );

    return tgObj;
}

unsigned int 
ScheduleManager::getEventRuleCount()
{
    return getObjectCountByType( SCH_ROTID_EVENTRULE );
}

ScheduleEventRule *
ScheduleManager::getEventRuleByIndex( unsigned int index )
{
    std::vector< RESTContentNode* > objList;

    getObjectVectorByType( SCH_ROTID_EVENTRULE, objList );

    if( index > objList.size() )
        return NULL;

    return (ScheduleEventRule *) objList[ index ];
}

ScheduleEventRule *
ScheduleManager::getEventRuleByID( std::string erID )
{
    ScheduleEventRule *erObj = NULL;

    erObj = ( ScheduleEventRule* ) getObjectByID( erID );

    return erObj;
}

bool
ScheduleManager::loadConfiguration()
{
    std::string filePath;
    ScheduleConfig cfgReader;

    filePath = cfgPath + "/irrigation/test_config2.xml";

    // Clear out any existing items.
    clear();

    // Read everything from a file
    cfgReader.readConfig( filePath, this );

    return false;
}

bool
ScheduleManager::saveConfiguration()
{
    ScheduleConfig cfgWriter;
    std::string filePath;

    filePath = cfgPath + "/irrigation/test_config2.xml";

    // Write everything out to a file
    cfgWriter.writeConfig( filePath, this );

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

    std::vector< RESTContentNode* > rtnVector;

    getObjectVectorByType( SCH_ROTID_EVENTRULE, rtnVector );

    // Check if the rules are going to generate any new events.
    for( std::vector< RESTContentNode* >::iterator it = rtnVector.begin(); it != rtnVector.end(); ++it )
    {
        ((ScheduleEventRule *)*it)->updateActiveEvents( activeEvents, curTime );
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

        std::vector< RESTContentNode* > rtnVector;

        getObjectVectorByType( SCH_ROTID_EVENTRULE, rtnVector );

        // Check if the rules are going to generate any new events.
        for( std::vector< RESTContentNode* >::iterator it = rtnVector.begin(); it != rtnVector.end(); ++it )
        {
            ((ScheduleEventRule *)*it)->updateActiveEvents( *eventList, curTime );
        }
#if 0
        // Check if the rules are going to generate any new events.
        for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin(); it != eventRuleList.end(); ++it )
        {
            (*it)->updateActiveEvents( *eventList, curTime );
        }
#endif
    }

    return eventList;
}

void 
ScheduleManager::freeScheduleEventList( ScheduleEventList *listPtr )
{

}

RESTContentNode * 
ScheduleManager::newObject( unsigned int type )
{
    switch( type )
    {
        case SCH_ROTID_EVENTRULE:
            return new ScheduleEventRule( *this );
        break;

        case SCH_ROTID_ZONEGROUP:
            return new ScheduleZoneGroup( *this );
        break;

        case SCH_ROTID_ZONERULE:
            return new ScheduleZoneRule( *this );
        break;

        case SCH_ROTID_TRIGGERGROUP:
            return new ScheduleTriggerGroup( *this );
        break;

        case SCH_ROTID_TRIGGERRULE:
            return new ScheduleTriggerRule( *this );
        break;
    }

    return NULL;
}

void 
ScheduleManager::freeObject( RESTContentNode *objPtr )
{
    std::cout << "freeObject: " << objPtr->getID() << std::endl;
    delete objPtr;
}

RESTContentTemplate *
ScheduleManager::getContentTemplateForType( unsigned int type )
{
    switch( type )
    {
        case SCH_ROTID_EVENTRULE:
            return ScheduleEventRule::generateContentTemplate();
        break;

        case SCH_ROTID_ZONEGROUP:
            return ScheduleZoneGroup::generateContentTemplate();
        break;

        case SCH_ROTID_ZONERULE:
            return ScheduleZoneRule::generateContentTemplate();
        break;

        case SCH_ROTID_TRIGGERGROUP:
            return ScheduleTriggerGroup::generateContentTemplate();
        break;

        case SCH_ROTID_TRIGGERRULE:
            return ScheduleTriggerRule::generateContentTemplate();
        break;
    }

    return NULL;
}

unsigned int 
ScheduleManager::getTypeFromObjectElementName( std::string name )
{
    if( name == ScheduleEventRule::getElementName() )
    {
        return SCH_ROTID_EVENTRULE;
    }
    else if( name == ScheduleZoneGroup::getElementName() )
    {
        return SCH_ROTID_ZONEGROUP;
    }
    else if( name == ScheduleZoneRule::getElementName() )
    {
        return SCH_ROTID_ZONERULE;
    }
    else if( name == ScheduleTriggerGroup::getElementName() )
    {
        return SCH_ROTID_TRIGGERGROUP;
    }
    else if( name == ScheduleTriggerRule::getElementName() )
    {
        return SCH_ROTID_TRIGGERRULE;
    }
    else
    {
        std::cerr << "ERROR: getTypeFromObjectElementName - Unrecognized element name" << std::endl;
        return 0;
    }
}

void 
ScheduleManager::notifyCfgChange()
{
    saveConfiguration();
    loadConfiguration();
}

unsigned int
ScheduleZoneGroup::getObjType()
{
    return SCH_ROTID_ZONEGROUP;
}

unsigned int
ScheduleZoneRule::getObjType()
{
    return SCH_ROTID_ZONERULE;
}

unsigned int
ScheduleTriggerGroup::getObjType()
{
    return SCH_ROTID_TRIGGERGROUP;
}

unsigned int
ScheduleTriggerRule::getObjType()
{
    return SCH_ROTID_TRIGGERRULE;
}

