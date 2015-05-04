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
    return "notset";
}

std::string 
ScheduleZoneRule::getStaticTypeStr()
{
    return "notset";
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

    rtnNode->defineRef( "zoneid", true );

    return rtnNode;
}

void
ScheduleZoneRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Optional Fields
    objCN->getField( "name", tmpStr );

    if( tmpStr.empty() == false )
    {
        name = tmpStr;
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
}

bool 
ScheduleZoneRule::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;

    objCN = new RESTContentNode;

    // Create the object representation
    ScheduleZoneRule::setContentNodeFromFields( objCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

bool 
ScheduleZoneRule::buildRCTemplateTree( RESTContentTemplate *rootCN )
{
    RESTContentTemplate *objCN;

    objCN = generateContentTemplate();

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

void 
ScheduleZoneRule::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleZoneRule::getID()
{
    return id;
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
ScheduleZoneRule::start( ScheduleDateTime &curTime )
{
    printf( "ScheduleZoneRule::start\n" );

    //zone->setStateOn( "Schedule Event" );
}

void 
ScheduleZoneRule::poll( ScheduleDateTime &curTime )
{
}

void 
ScheduleZoneRule::complete( ScheduleDateTime &curTime )
{
    printf( "ScheduleZoneRule::complete\n" );

    //zone->setStateOff( "Schedule Event" );
}




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




ScheduleZoneGroup::ScheduleZoneGroup( RESTContentManager &objMgr )
    : objManager( objMgr )
{

}

ScheduleZoneGroup::~ScheduleZoneGroup()
{

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

bool 
ScheduleZoneGroup::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;
    RESTContentNode *listCN;

    objCN = new RESTContentNode;

    // Create the object representation
    objCN->setAsObject( "schedule-zone-group" );
    objCN->setID( getID() );
    objCN->setField( "policy", getZoneEventPolicyStr() );

    // Include the rule list
    listCN = new RESTContentNode;

    listCN->setAsArray( "zg-rule-list" );

    listCN->setParent( objCN );
    objCN->addChild( listCN );
    
    // Add each rule object
    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "zonerule", ruleIDList );  
    
    for( std::vector< std::string >::iterator it = ruleIDList.begin(); it != ruleIDList.end(); ++it )
    {
        ScheduleZoneRule *child = (ScheduleZoneRule *)objManager.getObjectByID( *it );
        child->buildRCNodeTree( listCN );
    }

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

bool 
ScheduleZoneGroup::buildRCTemplateTree( RESTContentTemplate *rootCN )
{
    RESTContentTemplate *objCN;
    RESTContentTemplate *listCN;

    // Generate this objects template.
    objCN = generateContentTemplate();

    // Create the rule list element
    listCN = new RESTContentTemplate;
    listCN->setAsArray( "zg-rule-list" );
    listCN->setParent( objCN );
    objCN->addChild( listCN );

    // Add a template for each rule
    ScheduleZoneRule::buildRCTemplateTree( listCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
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
ScheduleZoneGroup::setZoneEventPolicyFromStr( std::string value )
{
    
}

std::string 
ScheduleZoneGroup::getZoneEventPolicyStr()
{
    return "sequential";
}

void 
ScheduleZoneGroup::clearZoneList()
{
//    zoneList.clear();
}

void 
ScheduleZoneGroup::addZone( Zone *zone )
{
#if 0
    SZRFixedDuration *rule = new SZRFixedDuration;

    rule->setID( "addZone" );
    rule->setZone( zone );
    zoneList.push_back( rule );
#endif
}

void 
ScheduleZoneGroup::setZoneDuration( std::string zoneID, ScheduleTimeDuration &td )
{
#if 0
    for( std::vector<ScheduleZoneRule *>::iterator it = zoneList.begin(); it != zoneList.end(); ++it )
    {
        if( (*it)->getID() == zoneID )
        {
            ((SZRFixedDuration *)(*it))->setDuration( td );
        }
    }
#endif
}

void
ScheduleZoneGroup::addZoneRule( ScheduleZoneRule *ruleObj )
{
#if 0
    zoneList.push_back( ruleObj );
#endif
}

void 
ScheduleZoneGroup::updateZoneRule( std::string ruleID, RESTContentNode *objCN )
{
#if 0
    for( std::vector<ScheduleZoneRule *>::iterator it = zoneList.begin(); it != zoneList.end(); ++it )
    {
        if( (*it)->getID() == ruleID )
        {
            (*it)->setFieldsFromContentNode( objCN ); 
            return;
        }
    }

    // Not found
    throw SMException( 110, "Zone Rule with id " + ruleID +", does not exist"); 
#endif
}

void
ScheduleZoneGroup::deleteZoneRule( std::string ruleID )
{
#if 0
    ScheduleZoneRule *ruleObj = NULL;

    for( std::vector<ScheduleZoneRule *>::iterator it = zoneList.begin(); it != zoneList.end(); ++it )
    {
        if( (*it)->getID() == ruleID )
        {
            // Save away the object pointer
            ruleObj = *it;

            // Erase the element from the list
            zoneList.erase( it );

            // Get rid of the memory
            delete ruleObj;

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Zone Rule with id " + ruleID +", does not exist"); 
#endif
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

    objManager.getIDListForRelationship( getID(), "zonerule", ruleIDList );  

    printf( "ScheduleZoneGroup -- zone rule count: %d\n", (int)ruleIDList.size() );
    
    // Go through the zone list and schedule an event for each one
    for( std::vector< std::string >::iterator it = ruleIDList.begin(); it != ruleIDList.end(); ++it )
    {
        ScheduleZoneRule *zrObj = (ScheduleZoneRule *) objManager.getObjectByID( *it );

        ScheduleEvent *event = new ScheduleEvent;

        std::string eventName = zrObj->getID() + "-" + eventTime.getSimpleString();

        // Copy over identifying data
        event->setId( id );
        event->setTitle( eventName );

        // Set the start time
        event->setStartTime( eventTime );
    
        // FIX-ME calulate zone watering duration.
        printf("eventDuration: %d\n", (int)((SZRFixedDuration*)zrObj)->getDuration().asTotalSeconds() );
        eventTime.addSeconds( ((SZRFixedDuration*)zrObj)->getDuration().asTotalSeconds() );

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

//    std::string      id;
//    SER_TT_SCOPE     scope;
//    ScheduleDateTime refTime;
}

ScheduleTriggerRule::~ScheduleTriggerRule()
{

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
}

bool 
ScheduleTriggerRule::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;

    objCN = new RESTContentNode;

    // Create the object representation
    ScheduleTriggerRule::setContentNodeFromFields( objCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

bool 
ScheduleTriggerRule::buildRCTemplateTree( RESTContentTemplate *rootCN )
{
    RESTContentTemplate *objCN;

    objCN = generateContentTemplate();

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

STR_TYPE_T
ScheduleTriggerRule::getType()
{
    return STR_TYPE_NOTSET;
}

std::string 
ScheduleTriggerRule::getTypeStr()
{
    return "notset";
}

std::string 
ScheduleTriggerRule::getStaticTypeStr()
{
    return "notset";
}

void 
ScheduleTriggerRule::setID( std::string idValue )
{
    id = idValue;
}

std::string 
ScheduleTriggerRule::getID()
{
    return id;
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

bool 
ScheduleTriggerRule::checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime )
{
    // Base class 
    return false;
}

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

ScheduleTriggerGroup::ScheduleTriggerGroup( RESTContentManager &objMgr )
    : objManager( objMgr )
{
// std::string id;
// std::vector< ScheduleTimeTrigger > timeList;
}

ScheduleTriggerGroup::~ScheduleTriggerGroup()
{

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

bool 
ScheduleTriggerGroup::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;
    RESTContentNode *listCN;

    objCN = new RESTContentNode;

    // Create the object representation
    ScheduleTriggerGroup::setContentNodeFromFields( objCN );

    // Include the rule list
    listCN = new RESTContentNode;

    listCN->setAsArray( "tg-rule-list" );

    listCN->setParent( objCN );
    objCN->addChild( listCN );
    
    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "triggerrule", ruleIDList );  

    printf( "ScheduleTriggerGroup -- trigger rule count: %d\n", (int)ruleIDList.size() );
    
    // Go through the zone list and schedule an event for each one
    for( std::vector< std::string >::iterator it = ruleIDList.begin(); it != ruleIDList.end(); ++it )
    {
        ScheduleTriggerRule *child = (ScheduleTriggerRule *)objManager.getObjectByID( *it );
        child->buildRCNodeTree( listCN );
    }

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

bool 
ScheduleTriggerGroup::buildRCTemplateTree( RESTContentTemplate *rootCN )
{
    RESTContentTemplate *objCN;
    RESTContentTemplate *listCN;

    objCN = generateContentTemplate();

    // Create the rule list element
    listCN = new RESTContentTemplate;
    listCN->setAsArray( "tg-rule-list" );
    listCN->setParent( objCN );
    objCN->addChild( listCN );

    // Generate the template for the list element.
    ScheduleTriggerRule::buildRCTemplateTree( listCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
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
ScheduleTriggerGroup::setName( std::string value )
{
    name = value;
}

std::string 
ScheduleTriggerGroup::getName()
{
    return name;
}

void 
ScheduleTriggerGroup::clearTriggerRuleList()
{
//    ruleList.clear();
}

void 
ScheduleTriggerGroup::addTriggerRule( ScheduleTriggerRule *newTrigger )
{
//    ScheduleTrigger ttObj;
 
//    ttObj.setID( id );
//    ttObj.setRefTime( refTime );
//    ttObj.setScope( scope );

#if 0
    ruleList.push_back( newTrigger );
#endif
}

void 
ScheduleTriggerGroup::updateTriggerRule( std::string ruleID, RESTContentNode *objCN )
{
#if 0
    for( std::vector<ScheduleTriggerRule *>::iterator it = ruleList.begin(); it != ruleList.end(); ++it )
    {
        if( (*it)->getID() == ruleID )
        {
            (*it)->setFieldsFromContentNode( objCN ); 
            return;
        }
    }

    // Not found
    throw SMException( 110, "Trigger Rule with id " + ruleID +", does not exist"); 
#endif
}

void 
ScheduleTriggerGroup::deleteTriggerRule( std::string ruleID )
{
    ScheduleTriggerRule *ruleObj = NULL;

#if 0
    for( std::vector<ScheduleTriggerRule *>::iterator it = ruleList.begin(); it != ruleList.end(); ++it )
    {
        if( (*it)->getID() == ruleID )
        {
            // Save away the object pointer
            ruleObj = *it;

            // Erase the element from the list
            ruleList.erase( it );

            // Get rid of the memory
            delete ruleObj;

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Trigger Rule with id " + ruleID +", does not exist"); 
#endif
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
#if 0
    // Search through possible triggers, if any of them go off then we are done.
    for( std::vector<ScheduleTriggerRule *>::iterator it = ruleList.begin() ; it != ruleList.end(); ++it )
    {
        if( (*it)->checkForTrigger( curTime, eventTime ) == true )
        {
            // At least on triggered, return that.
            // Short circuit, no need to check the rest.
            return true;
        }
    }

    // Nothing triggered, return nothing to do.
    return false;
#endif
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
    rtnNode->defineRef( "zone-group-ref", true );
    rtnNode->defineRef( "trigger-group-ref", true );

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
ScheduleEventRule::setZoneGroup( ScheduleZoneGroup *zgObj )
{
//    zoneGroup = zgObj;
}

std::string 
ScheduleEventRule::getZoneGroupID()
{
    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "zonegroup", ruleIDList );  

    return ruleIDList.size() ? ruleIDList[0] : "";
}

void 
ScheduleEventRule::setTriggerGroup( ScheduleTriggerGroup *tgObj )
{
//    triggerGroup = tgObj;
}

std::string 
ScheduleEventRule::getTriggerGroupID()
{
    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "triggergroup", ruleIDList );  

    return ruleIDList.size() ? ruleIDList[0] : "";

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

    //objCN->setRef( "zone-group-ref", zoneRef.getID() );
    //objCN->setRef( "trigger-group-ref", triggerRef.getID() );
}

bool 
ScheduleEventRule::buildRCNodeTree( RESTContentNode *rootCN )
{
    RESTContentNode *objCN;
    RESTContentNode *listCN;

    objCN = new RESTContentNode;

    // Create the object representation
    ScheduleEventRule::setContentNodeFromFields( objCN );

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
}

bool 
ScheduleEventRule::buildRCTemplateTree( RESTContentTemplate *rootCN )
{
    RESTContentTemplate *objCN;

    objCN = generateContentTemplate();

    // Hook us into the main tree
    objCN->setParent( rootCN );
    rootCN->addChild( objCN );
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

    std::vector< std::string > idList;

    objManager.getIDListForRelationship( getID(), "zone-group", idList );  
    ScheduleZoneGroup *zoneGroup = (ScheduleZoneGroup *) objManager.getObjectByID( idList[0] );

    idList.clear();

    objManager.getIDListForRelationship( getID(), "trigger-group", idList );  
    ScheduleTriggerGroup *triggerGroup = (ScheduleTriggerGroup *) objManager.getObjectByID( idList[0] );

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
    return false;
}

unsigned int 
ScheduleManager::getZoneGroupCount()
{
    return getObjectCountByType( SCH_ROTID_ZONEGROUP );
    //return zoneGroupList.size();
}

ScheduleZoneGroup *
ScheduleManager::getZoneGroupByIndex( unsigned int index )
{
    std::vector< RESTContentNode* > zoneGroupList;

    getObjectVectorByType( SCH_ROTID_ZONEGROUP, zoneGroupList );

    if( index > zoneGroupList.size() )
        return NULL;

    return (ScheduleZoneGroup *) zoneGroupList[ index ];
}

ScheduleZoneGroup *
ScheduleManager::getZoneGroupByID( std::string zgID )
{
    ScheduleZoneGroup *zgObj = NULL;

    zgObj = ( ScheduleZoneGroup* ) getObjectByID( zgID );

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        if( zgID == (*it)->getID() )
        {
            zgObj = *it;
        }
    }
#endif
    return zgObj;
}
#if 0
void 
ScheduleManager::addNewZoneGroup( RESTContentNode *rootCN, ScheduleZoneGroup **newGroup )
{
    std::string tmpValue;

    printf( "ScheduleManager - addNewZoneGroup\n" );

    // Add a rule record based on the content
    ScheduleZoneGroup *grpObj = createNewZoneGroup(); 

    // Have the object fill in it's internal members
    grpObj->setFieldsFromContentNode( rootCN );

    // Add the expander object to the list
//    zoneGroupList.push_back( grpObj );

    // Save the new configuration
    saveConfiguration();

    // Fill out the return pointer.
    *newGroup = grpObj;
}

void 
ScheduleManager::updateZoneGroup( std::string zgID, RESTContentNode *rootCN )
{
    ScheduleZoneGroup *zgObj;
    std::string        tmpValue;

    printf( "ScheduleManager - updateZoneGroup\n" );

    // Make sure the request object already exists.
    zgObj = getZoneGroupByID( zgID );    

    if( zgObj == NULL )
    {
        throw SMException( 103, "Specified zone group does not exist");
    }

    // Check for a zone group update.
    zgObj->setFieldsFromContentNode( rootCN );

    // Save the new configuration
    saveConfiguration();
}

void
ScheduleManager::deleteZoneGroup( std::string zgID )
{
    //ScheduleZoneGroup *zgObj = NULL;

    // Delete an object by id
    deleteObjByID( zgID );

#if 0
    // Search through the list for the right ID
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        if( zgID == (*it)->getID() )
        {
            // Save away the object pointer
            zgObj = *it;

            // Erase the element from the list
            zoneGroupList.erase( it );

            // Get rid of the memory
            delete zgObj;

            // Save the modified configuration
            saveConfiguration();

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Zone Group with id " + zgID +", does not exist"); 
#endif
}

void 
ScheduleManager::addNewZoneRule( std::string zoneGroupID, RESTContentNode *rootCN, ScheduleZoneRule **newRule )
{
    std::string tmpValue;

    printf( "ScheduleManager - addNewZoneRule\n" );

    ScheduleZoneGroup *zgObj = getZoneGroupByID( zoneGroupID );

    if( zgObj == NULL )
    {
        throw SMException( 100, "Specified zone group does not exist");
    }

#if 0
    // Get the zone reference
    rootCN->getRef( "zoneid", tmpValue );

    Zone *zone = zoneMgr->getZoneByID( tmpValue );

    if( zone == NULL )
    {
        throw SMException( 101, "Specified zone does not exist");
    }
#endif

    // Add a rule record based on the content
    ScheduleZoneRule *ruleObj = createNewZoneRule( rootCN ); 

    // Have the object fill in it's internal members
    ruleObj->setFieldsFromContentNode( rootCN );

    // Add the zone object
//    ((SZRFixedDuration*)ruleObj)->setZone( zone );

    // Add the expander object to the list
    zgObj->addZoneRule( ruleObj );

    // Save the new configuration
    saveConfiguration();

    // Fill out the return pointer.
    *newRule = ruleObj;
}

void 
ScheduleManager::updateZoneRule( std::string zgID, std::string ruleID, RESTContentNode *rootCN )
{
    ScheduleZoneGroup *zgObj;
    std::string        tmpValue;

    printf( "ScheduleManager - updateZoneGroup\n" );

    // Make sure the request object already exists.
    zgObj = getZoneGroupByID( zgID );    

    if( zgObj == NULL )
    {
        throw SMException( 103, "Specified zone group does not exist");
    }

    // Check for a zone group update.
    zgObj->updateZoneRule( ruleID, rootCN );

    // Save the new configuration
    saveConfiguration();
}

void
ScheduleManager::deleteZoneRule( std::string zgID, std::string zoneRuleID )
{
//    ScheduleZoneGroup *zgObj = NULL;

    // Delete an object by id
    deleteObjByID( zgID );

    // Search through the list for the right ID
#if 0
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        if( zgID == (*it)->getID() )
        {
            // Ask the zone group to get rid of a zone rule
            (*it)->deleteZoneRule( zoneRuleID );

            // Save the modified configuration
            saveConfiguration();

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Zone Group with id " + zgID +", does not exist"); 
#endif
}
#endif

void
ScheduleManager::generateZoneGroupListContent( RESTContentNode *rootNode )
{
    RESTContentNode   *curNode;

    // Create the root object
    rootNode->setAsArray( "zone-group-list" );

    std::cout << "ScheduleManager::generateZoneGroupListContent - 1" << std::endl;

    // Enumerate the zone-group list
    std::vector< std::string > idList;

    getIDVectorByType( SCH_ROTID_ZONEGROUP, idList );

    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it)
    {
        std::cout << "ScheduleManager::generateZoneGroupListContent - " << *it << std::endl;

        curNode = new RESTContentNode();
      
        curNode->setAsID( *it );

        rootNode->addChild( curNode );
    }

}

void
ScheduleManager::generateZoneGroupContent( std::string objID, RESTContentNode *rootNode )
{
    ScheduleZoneGroup *zgObj = NULL;
    RESTContentNode   *curNode;

    std::cout << "ScheduleManager::generateZoneGroupContent - 1" << std::endl;

    zgObj = getZoneGroupByID( objID );

    if( zgObj )
    {
        zgObj->setContentNodeFromFields( rootNode );
    }
}

void 
ScheduleManager::generateZoneGroupRuleListContent( std::string zoneGroupID, RESTContentNode *rootCN )
{
    ScheduleZoneGroup *zgObj;
    RESTContentNode   *curNode;

    zgObj = getZoneGroupByID( zoneGroupID );

    if( zgObj == NULL )
    {
        throw SMException( 110, "Specified zone group does not exist");
    }

    // Create the root object
    rootCN->setAsArray( "schedule-zone-rule-list" );

    std::cout << "ScheduleManager::generateZoneGroupRuleListContent - 1" << std::endl;

    for( unsigned int index = 0; index < zgObj->getZoneRuleCount(); index++ )
    {
        ScheduleZoneRule *zrObj = zgObj->getZoneRuleByIndex( index );

        std::cout << "ScheduleManager::generateZoneGroupRuleListContent - " <<zrObj->getID() << std::endl;

        curNode = new RESTContentNode();
      
        curNode->setAsID( zrObj->getID() );

        rootCN->addChild( curNode );
    }

}

void 
ScheduleManager::generateZoneGroupRuleContent( std::string zoneGroupID, std::string ruleID, RESTContentNode *rootCN )
{
    ScheduleZoneGroup *zgObj;
    ScheduleZoneRule  *zoneRule;

    zgObj = getZoneGroupByID( zoneGroupID );

    if( zgObj == NULL )
    {
        throw SMException( 110, "Specified zone group does not exist");
    }

    zoneRule = zgObj->getZoneRuleByID( ruleID );

    if( zoneRule == NULL )
    {
        throw SMException( 120, "Specified zone rule does not exist");
    }

    // Create the root object
    zoneRule->setContentNodeFromFields( rootCN );
}

unsigned int 
ScheduleManager::getTriggerGroupCount()
{
    return getObjectCountByType( SCH_ROTID_TRIGGERGROUP );
    //return triggerGroupList.size();
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

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        if( tgID == (*it)->getID() )
        {
            tgObj = *it;
        }
    }
#endif

    return tgObj;
}

#if 0
void 
ScheduleManager::addNewTriggerGroup( RESTContentNode *rootCN, ScheduleTriggerGroup **newGroup )
{
    std::string tmpValue;

    printf( "ScheduleManager - addNewTriggerGroup\n" );

    // Add a rule record based on the content
    ScheduleTriggerGroup *grpObj = createNewTriggerGroup(); 

    // Have the object fill in it's internal members
    grpObj->setFieldsFromContentNode( rootCN );

    // Add the expander object to the list
//    triggerGroupList.push_back( grpObj );

    // Save the new configuration
    saveConfiguration();

    // Fill out the return pointer.
    *newGroup = grpObj;
}

void 
ScheduleManager::updateTriggerGroup( std::string tgID, RESTContentNode *rootCN )
{
    ScheduleTriggerGroup *tgObj;
    std::string        tmpValue;

    printf( "ScheduleManager - updateTriggerGroup\n" );

    // Make sure the request object already exists.
    tgObj = getTriggerGroupByID( tgID );    

    if( tgObj == NULL )
    {
        throw SMException( 103, "Specified zone group does not exist");
    }

    // Check for a zone group update.
    tgObj->setFieldsFromContentNode( rootCN );

    // Save the new configuration
    saveConfiguration();
}

void
ScheduleManager::deleteTriggerGroup( std::string tgID )
{
//    ScheduleTriggerGroup *tgObj = NULL;

    // Delete an object by id
    deleteObjByID( tgID );

#if 0
    // Search through the list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        if( tgID == (*it)->getID() )
        {
            // Save away the object pointer
            tgObj = *it;

            // Erase the element from the list
            triggerGroupList.erase( it );

            // Get rid of the memory
            delete tgObj;

            // Save the modified configuration
            saveConfiguration();

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Trigger Group with id " + tgID +", does not exist"); 
#endif
}

void 
ScheduleManager::addNewTriggerRule( std::string tgID, RESTContentNode *rootCN, ScheduleTriggerRule **newRule )
{
    std::string tmpValue;

    printf( "ScheduleManager - addNewTriggerRule\n" );

    ScheduleTriggerGroup *tgObj = getTriggerGroupByID( tgID );

    if( tgObj == NULL )
    {
        throw SMException( 100, "Specified trigger group does not exist");
    }

    // Add a rule record based on the content
    ScheduleTriggerRule *ruleObj = createNewTriggerRule( rootCN ); 

    // Have the object fill in it's internal members
    ruleObj->setFieldsFromContentNode( rootCN );

    // Add the expander object to the list
    tgObj->addTriggerRule( ruleObj );

    // Save the new configuration
    saveConfiguration();

    // Fill out the return pointer.
    *newRule = ruleObj;
}

void 
ScheduleManager::updateTriggerRule( std::string tgID, std::string ruleID, RESTContentNode *rootCN )
{
    ScheduleTriggerGroup *tgObj;
    std::string        tmpValue;

    printf( "ScheduleManager - updateTriggerRule\n" );

    // Make sure the request object already exists.
    tgObj = getTriggerGroupByID( tgID );    

    if( tgObj == NULL )
    {
        throw SMException( 103, "Specified trigger group does not exist");
    }

    // Check for a zone group update.
    tgObj->updateTriggerRule( ruleID, rootCN );

    // Save the new configuration
    saveConfiguration();
}

void
ScheduleManager::deleteTriggerRule( std::string tgID, std::string ruleID )
{
//    ScheduleTriggerGroup *tgObj = NULL;

    // Delete an object by id
    deleteObjByID( ruleID );

#if 0
    // Search through the list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        if( tgID == (*it)->getID() )
        {
            // Ask the zone group to get rid of a zone rule
            (*it)->deleteTriggerRule( ruleID );

            // Save the modified configuration
            saveConfiguration();

            // Success
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Trigger Group with id " + tgID +", does not exist"); 
#endif
}

#endif

void
ScheduleManager::generateTriggerGroupListContent( RESTContentNode *rootNode )
{
    RESTContentNode   *curNode;

    // Create the root object
    rootNode->setAsArray( "trigger-group-list" );

    std::cout << "ScheduleManager::generateTriggerGroupListContent - 1" << std::endl;

    // Enumerate the zone-group list
    std::vector< std::string > idList;

    getIDVectorByType( SCH_ROTID_TRIGGERGROUP, idList );

    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it)
    {
        std::cout << "ScheduleManager::generateTriggerGroupListContent - " << *it << std::endl;

        curNode = new RESTContentNode();
      
        curNode->setAsID( *it );

        rootNode->addChild( curNode );
    }

}

void
ScheduleManager::generateTriggerGroupContent( std::string objID, RESTContentNode *rootNode )
{
    ScheduleTriggerGroup *tgObj = NULL;
    RESTContentNode   *curNode;

    std::cout << "ScheduleManager::generateTriggerGroupContent - 1" << std::endl;

    tgObj = getTriggerGroupByID( objID );

    if( tgObj )
    {
        tgObj->setContentNodeFromFields( rootNode );
    }
}

void 
ScheduleManager::generateTriggerGroupRuleListContent( std::string tgID, RESTContentNode *rootCN )
{
    ScheduleTriggerGroup *tgObj;
    RESTContentNode   *curNode;

    tgObj = getTriggerGroupByID( tgID );

    if( tgObj == NULL )
    {
        throw SMException( 110, "Specified trigger group does not exist");
    }

    // Create the root object
    rootCN->setAsArray( "schedule-trigger-rule-list" );

    std::cout << "ScheduleManager::generateTriggerGroupRuleListContent - 1" << std::endl;

    for( unsigned int index = 0; index < tgObj->getTriggerRuleCount(); index++ )
    {
        ScheduleTriggerRule *ruleObj = tgObj->getTriggerRuleByIndex( index );

        std::cout << "ScheduleManager::generateTriggerGroupRuleListContent - " << ruleObj->getID() << std::endl;

        curNode = new RESTContentNode();
      
        curNode->setAsID( ruleObj->getID() );

        rootCN->addChild( curNode );
    }

}

void 
ScheduleManager::generateTriggerGroupRuleContent( std::string tgID, std::string ruleID, RESTContentNode *rootCN )
{
    ScheduleTriggerGroup *tgObj;
    ScheduleTriggerRule  *ruleObj;

    tgObj = getTriggerGroupByID( tgID );

    if( tgObj == NULL )
    {
        throw SMException( 110, "Specified trigger group does not exist");
    }

    ruleObj = tgObj->getTriggerRuleByID( ruleID );

    if( ruleObj == NULL )
    {
        throw SMException( 120, "Specified rule does not exist");
    }

    // Create the root object
    ruleObj->setContentNodeFromFields( rootCN );
}

unsigned int 
ScheduleManager::getEventRuleCount()
{
    return getObjectCountByType( SCH_ROTID_EVENTRULE );
    //return eventRuleList.size();
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

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        if( erID == (*it)->getID() )
        {
            erObj = *it;
            break;
        }
    }
#endif

    return erObj;
}

#if 0
ScheduleEventRule *
ScheduleManager::createNewEventRule()
{
    ScheduleEventRule *ruleObj;
    char idStr[128];

    // Allocate a new rule object
    ruleObj = new ScheduleEventRule( *this );

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "er%d", (int) nextID );
    ruleObj->setID( idStr );

    printf( "New idStr: %s\n", idStr );

    // Return the object
    return ruleObj;
}

ScheduleZoneGroup *
ScheduleManager::createNewZoneGroup()
{
    ScheduleZoneGroup *grpObj;
    char idStr[128];

    // Allocate a new rule object
    grpObj = new ScheduleZoneGroup( *this );

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "zg%d", (int) nextID );
    grpObj->setID( idStr );

    printf( "New idStr: %s\n", idStr );

    // Return the object
    return grpObj;
}

ScheduleZoneRule *
ScheduleManager::createNewZoneRule( RESTContentNode *rootCN )
{
    ScheduleZoneRule *ruleObj;
    char idStr[128];

    // Allocate a new rule object
    ruleObj = new ScheduleZoneRule( *this );

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "zr%d", (int) nextID );
    ruleObj->setID( idStr );

    printf( "New idStr: %s\n", idStr );

    // Return the object
    return ruleObj;
}

ScheduleTriggerGroup *
ScheduleManager::createNewTriggerGroup()
{
    ScheduleTriggerGroup *grpObj;
    char idStr[128];

    // Allocate a new rule object
    grpObj = new ScheduleTriggerGroup( *this );

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "tg%d", (int) nextID );
    grpObj->setID( idStr );

    printf( "New idStr: %s\n", idStr );

    // Return the object
    return grpObj;
}

ScheduleTriggerRule *
ScheduleManager::createNewTriggerRule( RESTContentNode *rootCN )
{
    ScheduleTriggerRule *ruleObj;
    char idStr[128];

    // Allocate a new rule object
    ruleObj = new ScheduleTimeTrigger( *this );

    // Create a unique id
    nextID += 1;
    sprintf( idStr, "tr%d", (int) nextID );
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

void 
ScheduleManager::addNewEventRule( ScheduleEventRule *ruleObj )
{
    printf( "ScheduleManager - addNewEventRule\n" );

    // Add the expander object to the list
    //eventRuleList.push_back( ruleObj );

    // Save the new configuration
    saveConfiguration();
}

void 
ScheduleManager::addNewEventRule( RESTContentNode *rootCN, ScheduleEventRule **newEvent )
{
    std::string tmpValue;

    printf( "ScheduleManager - addNewEventRule2\n" );
#if 0
    rootCN->getRef( "zone-group-ref", tmpValue );
    ScheduleZoneGroup *zgObj = getZoneGroupByID( tmpValue );

    if( zgObj == NULL )
    {
        throw SMException( 100, "Specified zone group does not exist");
    }

    rootCN->getRef( "trigger-group-ref", tmpValue );
    ScheduleTriggerGroup *tgObj = getTriggerGroupByID( tmpValue );

    if( tgObj == NULL )
    {
        throw SMException( 101, "Specified trigger group does not exist");
    }
#endif
    // Add a rule record based on the content
    ScheduleEventRule *ruleObj = createNewEventRule();

    // Have the object fill in it's internal members
    ruleObj->setFieldsFromContentNode( rootCN );

    // Set the zone and trigger group pointers.
//    ruleObj->setZoneGroup( zgObj );
//    ruleObj->setTriggerGroup( tgObj );

    // Add the expander object to the list
//    eventRuleList.push_back( ruleObj );

    // Save the new configuration
    saveConfiguration();

    // Fill out the return pointer.
    *newEvent = ruleObj;

}

void 
ScheduleManager::updateEventRule( std::string erID, RESTContentNode *rootCN )
{
    ScheduleEventRule *ruleObj;
    std::string        tmpValue;

    printf( "ScheduleManager - updateEventRule\n" );

    // Make sure the request event rule already exists.
    ruleObj = getEventRuleByID( erID );    

    if( ruleObj == NULL )
    {
        throw SMException( 103, "Specified schdule event rule does not exist");
    }

    // Check for a zone group update.
    if( rootCN->getRef( "zone-group-ref", tmpValue ) )
    {
        ScheduleZoneGroup *zgObj = getZoneGroupByID( tmpValue );

        if( zgObj == NULL )
        {
            throw SMException( 100, "Specified zone group does not exist");
        } 

        // Set the update zone group on the object
        ruleObj->setZoneGroup( zgObj );
    }

    // Check for a trigger group update.
    if( rootCN->getRef( "trigger-group-ref", tmpValue ) )
    {
        ScheduleTriggerGroup *tgObj = getTriggerGroupByID( tmpValue );

        if( tgObj == NULL )
        {
            throw SMException( 101, "Specified trigger group does not exist");
        } 

        // Set the update zone group on the object
        ruleObj->setTriggerGroup( tgObj );
    }


    // Have the object fill in it's internal members
    ruleObj->setFieldsFromContentNode( rootCN );

    // Save the new configuration
    saveConfiguration();
}
#endif

void
ScheduleManager::generateScheduleRuleListContent( RESTContentNode *rootNode )
{
    ScheduleEventRule *erObj = NULL;
    RESTContentNode   *curNode;

    // Create the root object
    rootNode->setAsArray( "event-rule-list" );

    std::cout << "ScheduleManager::generateScheduleRuleListContent - 1" << std::endl;

    // Enumerate the zone-group list
    std::vector< std::string > idList;

    getIDVectorByType( SCH_ROTID_EVENTRULE, idList );

    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it)
    {
        std::cout << "ScheduleManager::generateScheduleRuleListContent - " << *it << std::endl;

        curNode = new RESTContentNode();
      
        curNode->setAsID( *it );

        rootNode->addChild( curNode );
    }

}

void
ScheduleManager::generateEventRuleContent( std::string ruleID, RESTContentNode *rootNode )
{
    ScheduleEventRule *erObj = NULL;
    RESTContentNode   *curNode;

    // Create the root object
    //rootNode->setAsObject( "schedule-event-rule" );

    std::cout << "ScheduleManager::generateEventRuleContent - 1" << std::endl;

    erObj = getEventRuleByID( ruleID );

    if( erObj )
    {
        erObj->setContentNodeFromFields( rootNode );
/*
        rootNode->setID( ruleID );

        rootNode->setField( "enabled", erObj->getEnabled() ? "true" : "false" );
        rootNode->setField( "name", erObj->getName() );
        rootNode->setField( "desc", erObj->getDescription() );
        rootNode->setField( "url", erObj->getURL() );
        rootNode->setField( "zone-group-id", erObj->getZoneGroupID() );
        rootNode->setField( "trigger-group-id", erObj->getTriggerGroupID() );
*/
    }
}

void
ScheduleManager::deleteEventRuleByID( std::string erID )
{
    ScheduleEventRule *erObj = NULL;

    deleteObjByID( erID );

#if 0
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
            return;   
        }
    }

    // Not found
    throw SMException( 110, "Event Rule with id " + erID +", does not exist"); 
#endif
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
    ruleObj = new ScheduleEventRule( *this );

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
    //eventRuleList.push_back( ruleObj );

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
    zgObj = new ScheduleZoneGroup( *this );

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

    // Add the expander object to the list
//    zoneGroupList.push_back( zgObj );
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
    tgObj = new ScheduleTriggerGroup( *this );

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
                printf( "tgObj: 0x%lx\n", (long unsigned int) tgObj );
                printf( "ScheduleManager - trigger group id: %s\n", tgObj->getID().c_str() );

                ScheduleTimeTrigger *newTrigger = new ScheduleTimeTrigger( *this );
                newTrigger->setID( idStr );
                newTrigger->setScope( SER_TT_SCOPE_DAY );
                newTrigger->setRefTime( time );

                tgObj->addTriggerRule( newTrigger );
            }

        }
    }

    printf( "tgObj: 0x%lx\n", (long unsigned int) tgObj );
    printf( "ScheduleManager - add trigger group: %s\n", tgObj->getID().c_str() );

    // Add the expander object to the list
//    triggerGroupList.push_back( tgObj );

}

bool
ScheduleManager::loadConfigNew()
{
    ScheduleConfig cfgReader;
    std::string filePath;
    RESTContentTemplate *templateCN;
    RESTContentTemplate *listCN;

    filePath = cfgPath + "/irrigation/test_config2.xml";

    // Generate the root level configuration object
    templateCN = new RESTContentTemplate;
    
    templateCN->setAsObject( "hnode-irrigation-schedule-cfg" );
    templateCN->setStatic();
    templateCN->setTemplateID( SCH_ROTID_ROOT );

    templateCN->setObjectCallback( this );
    templateCN->setListCallback( this );
//    templateCN->setFactoryCallback( this );
//    templateCN->setReferenceCallback( this );

    // Take care of the zone groups
    listCN = new RESTContentTemplate;

    listCN->setAsArray( "zone-group-list" );
    listCN->setParent( templateCN );

    templateCN->addChild( listCN );

    ScheduleZoneGroup::buildRCTemplateTree( listCN );

    // Take care of the trigger groups
    listCN = new RESTContentTemplate;

    listCN->setAsArray( "trigger-group-list" );
    listCN->setParent( templateCN );

    templateCN->addChild( listCN );

    ScheduleTriggerGroup::buildRCTemplateTree( listCN );

    // Take care of the event groups
    listCN = new RESTContentTemplate;

    listCN->setAsArray( "rule-list" );
    listCN->setParent( templateCN );

    templateCN->addChild( listCN );

    ScheduleEventRule::buildRCTemplateTree( listCN );

    // Read everything from a file
    cfgReader.readConfig( filePath, templateCN );
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

    loadConfigNew();

    return false;
}

#define MY_ENCODING "ISO-8859-1"

#if 0
bool
ScheduleManager::writeContentTree( RESTContentNode *rootCN )
{




    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateTriggerGroupRuleListContent( objID, objNode );        

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
    
}
#endif

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
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "zoneid", "%s", ((SZRFixedDuration *)zoneRule)->getZoneID().c_str() );
        if( rc < 0 ) 
            return true;
        
        // Start an element named duration. 
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "duration", "%s", ((SZRFixedDuration *)zoneRule)->getDuration().getISOString().c_str() );
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

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        saveZoneGroup(*it);
    }
#endif

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
    for( int indx = 0; indx < tgObj->getTriggerRuleCount(); indx++ )
    {
        ScheduleTriggerRule *trigger = tgObj->getTriggerRuleByIndex( indx );

        // Start an element named zone-rule-list. 
        rc = xmlTextWriterStartElement( writer, BAD_CAST "day-time-trigger" );
        if( rc < 0 ) 
            return true;

        // Add the id attribute
        rc = xmlTextWriterWriteAttribute( writer, BAD_CAST "id", BAD_CAST trigger->getID().c_str() );
        if( rc < 0 ) 
            return true;
    
        // Start an element named zoneid. 
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "reftime", "%s", ((ScheduleTimeTrigger *)trigger)->getRefTime().getExtendedISOString().c_str() );
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

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        saveTriggerGroup(*it);
    }
#endif

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

#if 0
    // Search through the switch list for the right ID
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        saveEventRule(*it);
    }
#endif

    // Close the element named zone-group-list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool
ScheduleManager::saveConfigNew()
{
    ScheduleConfig cfgWriter;
    std::string filePath;
    RESTContentNode *rootCN;
    RESTContentNode *listCN;

    filePath = cfgPath + "/irrigation/test_config2.xml";

    // Generate the root level configuration object
    rootCN = new RESTContentNode;
    
    rootCN->setAsObject( "hnode-irrigation-schedule-cfg" );

    // Take care of the zone groups
    listCN = new RESTContentNode;

    listCN->setAsArray( "zone-group-list" );
    listCN->setParent( rootCN );

    rootCN->addChild( listCN );

#if 0
    for( std::vector<ScheduleZoneGroup *>::iterator it = zoneGroupList.begin() ; it != zoneGroupList.end(); ++it)
    {
        (*it)->buildRCNodeTree( listCN );
    }
#endif

    // Take care of the trigger groups
    listCN = new RESTContentNode;

    listCN->setAsArray( "trigger-group-list" );
    listCN->setParent( rootCN );

    rootCN->addChild( listCN );

#if 0
    for( std::vector<ScheduleTriggerGroup *>::iterator it = triggerGroupList.begin() ; it != triggerGroupList.end(); ++it)
    {
        (*it)->buildRCNodeTree( listCN );
    }
#endif

    // Take care of the event groups
    listCN = new RESTContentNode;

    listCN->setAsArray( "rule-list" );
    listCN->setParent( rootCN );

    rootCN->addChild( listCN );

#if 0
    for( std::vector<ScheduleEventRule *>::iterator it = eventRuleList.begin() ; it != eventRuleList.end(); ++it)
    {
        (*it)->buildRCNodeTree( listCN );
    }
#endif

    // Write everything out to a file
    cfgWriter.writeConfig( filePath, rootCN );
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

    // Tmp Call for debug
    saveConfigNew();

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

    std::vector< RESTContentNode* > rtnVector;

    getObjectVectorByType( SCH_ROTID_ROOT, rtnVector );

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

        getObjectVectorByType( SCH_ROTID_ROOT, rtnVector );

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


#if 0
bool 
ScheduleManager::lookupObj( RESTContentIDStack &idStack, RESTContentTemplate *ctObj, std::string objType )
{
    std::cout << "ScheduleManager::lookupObj: " << idStack.getLast() << "|" << objType << std::endl;
    std::cout << "ScheduleManager::lookupObj: " << ctObj->getTemplateID() << std::endl;

    if( ctObj->isStatic() == true )
    {
        // FIX ME -- temporary value till correct solution is understood
        idStack.pushID( "static" );
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_ZONEGROUP )
    {
        ScheduleZoneGroup *zgObj = getZoneGroupByID( idStack.getLast() );

        if( zgObj == NULL )
        {
            return false;
        }
      
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_ZONERULE )
    {
        ScheduleZoneGroup *zgObj = getZoneGroupByID( idStack.getParent( 0 ) );

        if( zgObj == NULL )
        {
            return false;
        }

        ScheduleZoneRule *zrObj = zgObj->getZoneRuleByID( idStack.getLast() );

        if( zrObj == NULL )
        {
            return false;
        }
       
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_TRIGGERGROUP )
    {
        ScheduleTriggerGroup *tgObj = getTriggerGroupByID( idStack.getLast() );

        if( tgObj == NULL )
        {
            return false;
        }
      
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_TRIGGERRULE )
    {
        ScheduleTriggerGroup *tgObj = getTriggerGroupByID( idStack.getParent( 0 ) );

        if( tgObj == NULL )
        {
            return false;
        }

        ScheduleTriggerRule *trObj = tgObj->getTriggerRuleByID( idStack.getLast() );

        if( trObj == NULL )
        {
            return false;
        }
       
        return true;
    }

#if 0
    if( templateType == "zonegroup" )
    {

        ScheduleZoneGroup *getZoneGroupByID( std::string zgID );

        ScheduleZoneRule *getZoneRuleByID( std::string trID );

        ScheduleTriggerGroup *getTriggerGroupByID( std::string tgID );

        ScheduleTriggerRule *getTriggerRuleByID( std::string trID );

        ScheduleEventRule *getEventRuleByID( std::string erID );
 

    }
#endif
    return false;  
}
#endif

#if 0
bool
ScheduleManager::createObj( RESTContentIDStack &idStack, RESTContentTemplate *ctObj, std::string objType, std::string &objID )  
{
    std::cout << "ScheduleManager::createObj: " << idStack.getLast() << "|" << objType << std::endl;

    if( objID.empty() == false )
    {
        idStack.pushID( objID );        
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_ROOT )
    {
        idStack.pushID( "root" );
        return true;
    }

    if( ctObj->getTemplateID() == SCH_ROTID_ZONEGROUP )
    {
        ScheduleZoneGroup *zgObj;
        char idStr[128];

        // Allocate a new rule object
        zgObj = new ScheduleZoneGroup( *this );

        // Create a unique id
        nextID += 1;
        sprintf( idStr, "zg%d", (int) nextID );
        zgObj->setID( idStr );

        printf( "New idStr: %s\n", idStr );

        // Add the expander object to the list
//        zoneGroupList.push_back( zgObj );

        // Return the object
        objID = idStr;
        return false;
    }

#if 0
    ScheduleZoneGroup *createNewZoneGroup();

    ScheduleZoneRule *createNewZoneRule( RESTContentNode *rootCN );

    ScheduleTriggerGroup *createNewTriggerGroup();

    ScheduleTriggerRule *createNewTriggerRule( RESTContentNode *rootCN );

    ScheduleEventRule *createNewEventRule();
#endif

    // Otherwise we are done.
    idStack.pushID( "tmpID" );
    return true;
}
#endif

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
ScheduleEventRule::getObjType()
{
    return SCH_ROTID_EVENTRULE;
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


void 
ScheduleManager::startObject( std::string objID ) 
{
    std::cout << "ScheduleManager::startObject: " << objID << std::endl;
}

void 
ScheduleManager::fieldsValid( std::string objID ) 
{
    std::cout << "ScheduleManager::fieldsValid: " << objID << std::endl;
}

void 
ScheduleManager::startChild( std::string objID )  
{
    std::cout << "ScheduleManager::startChild: " << objID << std::endl;
}

void 
ScheduleManager::endChild( std::string objID )    
{
    std::cout << "ScheduleManager::endChild: " << objID << std::endl;
}

void 
ScheduleManager::endObject( std::string objID )   
{
    std::cout << "ScheduleManager::endObject: " << objID << std::endl;
}

void 
ScheduleManager::updateField( std::string objID, std::string name, std::string value )
{
    std::cout << "ScheduleManager::updateField: " << objID << "|" << name << "|" << value << std::endl;
}

void 
ScheduleManager::updateRef( std::string objID, std::string name, std::string value )
{
    std::cout << "ScheduleManager::updateRef: " << objID << "|" << name << "|" << value << std::endl;
}

void 
ScheduleManager::updateTag( std::string objID, std::string name, std::string value )
{
    std::cout << "ScheduleManager::updateTag: " << objID << "|" << name << "|" << value << std::endl;
}

void 
ScheduleManager::addListMember( std::string objID, std::string listID, std::string childID )
{
    std::cout << "ScheduleManager::addListMember: " << objID << "|" << listID << "|" << childID << std::endl;

}

#if 0
void *
ScheduleManager::resolveRef( std::string refType, std::string objID )
{
    std::cout << "ScheduleManager::resolveRef: " << refType << "|" << objID  << std::endl;

}
#endif

