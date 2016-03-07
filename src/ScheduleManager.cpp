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
#include <arpa/inet.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

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
ScheduleDateTime::addWeeks( long weekValue )
{
    time = time + weeks( weekValue );
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
ScheduleDateTime::replaceSecondScope( ScheduleDateTime &replaceTime )
{
    time_duration td( time.time_of_day().hours(), time.time_of_day().minutes(), replaceTime.time.time_of_day().seconds(), 0 );

    ptime newTime( time.date(), td );

    time = newTime; 
}

void 
ScheduleDateTime::replaceMinuteScope( ScheduleDateTime &replaceTime )
{
    time_duration td( time.time_of_day().hours(), replaceTime.time.time_of_day().minutes(), replaceTime.time.time_of_day().seconds(), 0 );

    ptime newTime( time.date(), td );

    time = newTime; 
}

void 
ScheduleDateTime::replaceHourScope( ScheduleDateTime &replaceTime )
{
    ptime newTime( time.date(), replaceTime.time.time_of_day() );

    time = newTime; 
}

void 
ScheduleDateTime::advanceToMatchingWeekDay( ScheduleDateTime &targetTime )
{
    unsigned long curDayOfWeek = time.date().day_of_week();
    unsigned long tgtDayOfWeek = targetTime.time.date().day_of_week();

    std::cout << "advance: " << curDayOfWeek << " : " << tgtDayOfWeek << std::endl;

    std::cout << "t1: " << time << std::endl;

    if( curDayOfWeek > tgtDayOfWeek )
    {
        time += days( ((7 - curDayOfWeek) + tgtDayOfWeek) );
    }
    else if( curDayOfWeek < tgtDayOfWeek )
    {
        time += days( (tgtDayOfWeek - curDayOfWeek) );
    }

    std::cout << "t2: " << time << std::endl;

    ptime newTime( time.date(), targetTime.time.time_of_day() );

    time = newTime; 

    std::cout << "t3: " << time << std::endl;

}

void 
ScheduleDateTime::retreatToStartOfDay()
{
    time_duration td( 0, 0, 0, 0 );

    ptime newTime( time.date(), td );

    time = newTime; 
}

void 
ScheduleDateTime::advanceToEndOfDay()
{
    time_duration td( 23, 59, 59, 0 );

    ptime newTime( time.date(), td );

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

bool 
ScheduleDateTime::isSameDay( ScheduleDateTime &cmpTime )
{
    return ( time.date() == cmpTime.time.date() );
}

std::string
ScheduleDateTime::getSimpleString()
{
    return to_simple_string( time );
}

long 
ScheduleDateTime::getSecondOfMinute()
{
    return time.time_of_day().seconds();
}

long 
ScheduleDateTime::getMinuteOfHour()
{
    return time.time_of_day().minutes();
}

long 
ScheduleDateTime::getHourOfDay()
{
    return time.time_of_day().hours();
}

long
ScheduleDateTime::getDayOfWeek()
{
    return time.date().day_of_week();
}

long
ScheduleDateTime::getWeekNumber()
{
    return time.date().week_number();
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
ScheduleTimeDuration::setFromSeconds( unsigned long secCnt )
{
    td += seconds( secCnt );
}

void
ScheduleTimeDuration::setFromString( std::string timeStr )
{
    td = duration_from_string( timeStr );
}

void 
ScheduleTimeDuration::setFromTimeDifference( ScheduleDateTime &start, ScheduleDateTime &end )
{
    td = end.time - start.time;
}

std::string
ScheduleTimeDuration::getISOString()
{
    return to_iso_string( td );
}

std::string 
ScheduleTimeDuration::getSecondsString()
{
    char result[64];

    sprintf( result, "%ld", asTotalSeconds() );

    return result;
}

std::string 
ScheduleTimeDuration::getAsHMSString()
{
    std::string rtnStr;
    bool spaces = false;
    char result[64];

    if( (long) td.hours() )
    {
        sprintf( result, "%ld hrs", (long) td.hours() );
        rtnStr += result;
        spaces = true;
    }    

    if( (long) td.minutes() )
    {
        if( spaces == true )
            rtnStr += " ";

        sprintf( result, "%ld min", (long) td.minutes() );
        rtnStr += result;
        spaces = true;
    }

    if( (long) td.seconds() )
    {
        if( spaces == true )
            rtnStr += " ";

        sprintf( result, "%ld sec", (long) td.seconds() );
        rtnStr += result;
    }

    return rtnStr;
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



typedef struct TZInfoHeader
{
    char magic4[4];
    char version[1];
    char reserved[15];
    char ttisgmtcnt[4]; // UTC/local indicator count
    char ttisstdcnt[4]; // standard/wall indicator count
    char leapcnt[4];    // leap seconds count
    char timecnt[4];    // transistion times count
    char typecnt[4];    // local time types count
    char charcnt[4];    // timezone abbreviation strings    
}TZINFO_HDR_T;

ScheduleTimezone::ScheduleTimezone()
{
}

ScheduleTimezone::~ScheduleTimezone()
{

}

void
ScheduleTimezone::initFromSystemFiles()
{
    struct tm newtime;
    time_t ltime;
    char olsenzone[512];
    char buf[512];
    size_t readCnt;
    TZINFO_HDR_T tzhdr;

    unsigned long ttisgmtcnt; // UTC/local indicator count
    unsigned long ttisstdcnt; // standard/wall indicator count
    unsigned long leapcnt;    // leap seconds count
    unsigned long timecnt;    // transistion times count
    unsigned long typecnt;    // local time types count
    unsigned long charcnt;    // timezone abbreviation strings

    FILE *filePtr = fopen( "/etc/timezone", "r" );
    readCnt = fread( olsenzone, 1, sizeof(olsenzone), filePtr ); 
    fclose( filePtr );

    olsenZone = olsenzone;
    boost::trim( olsenZone );

    printf("otz: %s\n", olsenZone.c_str() );

    std::string filePath("/usr/share/zoneinfo/");
    filePath += olsenZone;

    filePtr = fopen( filePath.c_str(), "r" );
    if( filePtr == NULL )
    {
        perror("zoneinfo");
        return;
    }

    // Magic 4
    readCnt = fread( (void *)&tzhdr, sizeof(tzhdr), 1, filePtr ); 

    printf("magic4: %4.4s\n", tzhdr.magic4 );

    if( strncmp( (const char*) tzhdr.magic4, "TZif", 4 ) != 0 )
    {
        printf("magic4 mismatch\n");
        return;
    }

    // 6 item counts
    ttisgmtcnt = ntohl( *((unsigned long *)&tzhdr.ttisgmtcnt) );
    ttisstdcnt = ntohl( *((unsigned long *)&tzhdr.ttisstdcnt) );
    leapcnt = ntohl( *((unsigned long *)&tzhdr.leapcnt) );
    timecnt = ntohl( *((unsigned long *)&tzhdr.timecnt) );
    typecnt = ntohl( *((unsigned long *)&tzhdr.typecnt) );
    charcnt = ntohl( *((unsigned long *)&tzhdr.charcnt) );

    //printf( "Size: %ld\n", sizeof(tzhdr) );
    //printf( "Version: %c\n", tzhdr.version[0] );
    //printf( "Counts: %ld, %ld, %ld, %ld, %ld, %ld\n", ttisgmtcnt, ttisstdcnt, leapcnt, timecnt, typecnt, charcnt );
   
    // Skip
    unsigned long skip = 0;
    skip += (timecnt * 4); // Transistion times (longs)
    skip += (timecnt * 1); // Local Time Types for transition times
    skip += (typecnt * (4 + 1 + 1)); // ttinfo structure, gmtoff, isdst, abbrind
    skip += (leapcnt * (4 + 4)); // leapcnt pairs
    skip += (ttisstdcnt * 1); // standard or wall clock indicators
    skip += (ttisgmtcnt * 1); // utc or locat time indicators
    skip += charcnt * 1;

    //printf( "skip1: %ld\n", skip );

    // Skip over first set of data
    fseek( filePtr, skip, SEEK_CUR );

    if( tzhdr.version[0] != '2' )
        return;

    // Magic 4
    readCnt = fread( (void *)&tzhdr, sizeof(tzhdr), 1, filePtr ); 

    // 6 item counts
    ttisgmtcnt = ntohl( *((unsigned long *)&tzhdr.ttisgmtcnt) );
    ttisstdcnt = ntohl( *((unsigned long *)&tzhdr.ttisstdcnt) );
    leapcnt = ntohl( *((unsigned long *)&tzhdr.leapcnt) );
    timecnt = ntohl( *((unsigned long *)&tzhdr.timecnt) );
    typecnt = ntohl( *((unsigned long *)&tzhdr.typecnt) );
    charcnt = ntohl( *((unsigned long *)&tzhdr.charcnt) );

    //printf( "Counts2: %ld, %ld, %ld, %ld, %ld, %ld\n", ttisgmtcnt, ttisstdcnt, leapcnt, timecnt, typecnt, charcnt );
   
    // Skip
    skip = 0;
    skip += (timecnt * 8); // Transistion times (longs)
    skip += (timecnt * 1); // Local Time Types for transition times
    skip += (typecnt * (4 + 1 + 1)); // ttinfo structure, gmtoff, isdst, abbrind
    skip += (leapcnt * (4 + 8)); // leapcnt pairs
    skip += (ttisstdcnt * 1); // standard or wall clock indicators
    skip += (ttisgmtcnt * 1); // utc or locat time indicators
    skip += charcnt * 1;

    //printf( "skip2: %ld\n", skip );

    // Skip over second set of data
    fseek( filePtr, skip, SEEK_CUR );

    readCnt = fread( (void *)buf, 1, sizeof(buf), filePtr ); 

    printf( "buf read: %d\n", readCnt );
    printf( "buf: %*.*s\n", readCnt, readCnt, buf );

    posixStr.append( buf, readCnt );
    boost::trim( posixStr );

    fclose( filePtr );

    //printf("otz: %s\n", olsenzone );

    //ltime=time(&ltime);
    //localtime_r(&ltime, &newtime);
    //printf("The date and time is %s", asctime_r(&newtime, buf));
    //printf("tz: %s\n", newtime.tm_zone );

    // Swap the polarity to match boost's implementation
    boost::regex expression( "^([A-Z]+)([-]?)([0-9:]+)(.*)" );
    boost::smatch what;
    std::string boostStr;

    if( boost::regex_match( posixStr, what, expression ) )
    {
        // what[0] contains whole string
        // what[1] contains normal timezone abbreviation 
        // what[2] contains optional '-' character
        // what[3] contains the time offset from UTC 
        // what[4] contains the savings time abbreviation, and the date for change over.
        boostStr = what[1].str();

        if( what[2].str().empty() )
        {
            boostStr += "-";
        }

        boostStr += what[3].str();
        boostStr += what[4].str();
    }
    else
    {
        // Defualt to UTC time
        boostStr = "UTC0";
    }

    std::cout << "boostStr" << boostStr << std::endl;

    time_zone_ptr tzPtr( new posix_time_zone( boostStr ) );
//    time_zone_ptr tzPtr( new posix_time_zone( "MST-7MDT,M3.2.0,M11.1.0" ) );

    std::cout << tzPtr->to_posix_string() << std::endl;

    ptz = tzPtr;
}

std::string 
ScheduleTimezone::getOlsenStr() 
{ 
    return olsenZone; 
}

std::string 
ScheduleTimezone::getPosixStr() 
{ 
    return ptz->to_posix_string(); 
}

ScheduleLocalDateTime::ScheduleLocalDateTime()
{

}

ScheduleLocalDateTime::ScheduleLocalDateTime( ScheduleDateTime &timestamp )
{
    // Copy over the timestamp
    utcObj = timestamp;

    // Use the system timezone.
    tzObj.initFromSystemFiles();
}

ScheduleLocalDateTime::ScheduleLocalDateTime( ScheduleDateTime &timestamp, ScheduleTimezone &timezone )
{
    // Copy over the timestamp
    utcObj = timestamp;

    // Use the provided timezone.
    tzObj = timezone;
}

ScheduleLocalDateTime::~ScheduleLocalDateTime()
{

}

void 
ScheduleLocalDateTime::setTimezone( ScheduleTimezone &timezone )
{
    tzObj = timezone;
}

void 
ScheduleLocalDateTime::setTime( ScheduleDateTime &time )
{
    utcObj = time;
}

void 
ScheduleLocalDateTime::setFromCurrentSystemTime()
{
    // Get the timezone for the local system time.
    tzObj.initFromSystemFiles();

    // Get the current time from the system clock.
    utcObj.getCurrentTime();
}

ScheduleDateTime 
ScheduleLocalDateTime::getUTCTime()
{
    return utcObj;
}

std::string 
ScheduleLocalDateTime::getSimpleString()
{
    local_date_time ltime( utcObj.time, tzObj.ptz );

    std::cout << "local getSimpleStr: " << ltime << std::endl;

    return to_simple_string( ltime.local_time() );
}

std::string 
ScheduleLocalDateTime::getISOString()
{
    local_date_time ltime( utcObj.time, tzObj.ptz );

    return to_iso_string( ltime.local_time() );
}

std::string 
ScheduleLocalDateTime::getExtendedISOString()
{
    local_date_time ltime( utcObj.time, tzObj.ptz );

    return to_iso_extended_string( ltime.local_time() );
}

void 
ScheduleLocalDateTime::retreatToStartOfDay()
{
    local_date_time ltime( utcObj.time, tzObj.ptz );

    time_duration td( 0, 0, 0, 0 );

    local_date_time newTime( ltime.date(), td, tzObj.ptz, local_date_time::NOT_DATE_TIME_ON_ERROR );

    utcObj.time = newTime.utc_time(); 
}

void 
ScheduleLocalDateTime::advanceToEndOfDay()
{
    local_date_time ltime( utcObj.time, tzObj.ptz );

    time_duration td( 23, 59, 59, 0 );

    local_date_time newTime( ltime.date(), td, tzObj.ptz, local_date_time::NOT_DATE_TIME_ON_ERROR );

    utcObj.time = newTime.utc_time(); 
}

ScheduleEvent::ScheduleEvent()
{
    state     = SESTATE_IDLE;
    actionObj = NULL;
}

ScheduleEvent::~ScheduleEvent()
{

}

std::string 
ScheduleEvent::getId()
{
    std::string id = tRecord.getERID() + tRecord.getGroupID() + tRecord.getRuleID() + zRecord.getGroupID() + zRecord.getRuleID() + "-" + tRecord.getEventTime().getISOString();

    return id;
}

std::string 
ScheduleEvent::getDescription()
{
    // "Zone:  NorthPlanter  Duration:  5 minutes  Trigger: Twice A Day    
    std::string desc = "Zone: " + zRecord.getZoneName();

    ScheduleTimeDuration td;
    td.setFromTimeDifference( start, end );
    desc += "  Duration: " + td.getAsHMSString();

    std::cout << "ScheduleEvent::getDescription(): " << tRecord.getTriggerName() << std::endl;

    desc += "  Trigger: " + tRecord.getTriggerName();

    return desc;
}

std::string 
ScheduleEvent::getDurationStr()
{
    ScheduleTimeDuration td;
    td.setFromTimeDifference( start, end );
    return td.getAsHMSString();
}

void 
ScheduleEvent::setTriggerRecord( ScheduleTriggerRecord &tRec )
{
    tRecord = tRec;
}

ScheduleTriggerRecord &
ScheduleEvent::getTriggerRecord()
{
    return tRecord;
}

void 
ScheduleEvent::setZoneRecord( ScheduleZoneRecord &zRec )
{
    zRecord = zRec;
}

ScheduleZoneRecord &
ScheduleEvent::getZoneRecord()
{
    return zRecord;
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
ScheduleEvent::processCurrent( ScheduleDateTime &curTime, ScheduleEventLog &log )
{
    //printf( "processCurrent - %s:%s - %s - %s - %s -%s\n", getId().c_str(), getDescription().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );

    if( recycle.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_COMPLETE )
        {
            state = SESTATE_RECYCLE;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getDescription().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Recycle\n", getId().c_str(), getDescription().c_str() );               
        }   
    }

    if( end.isBefore( curTime ) )
    {
        // This event is complete
        if( state == SESTATE_RUNNING )
        {
            state = SESTATE_COMPLETE;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getDescription().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Completing\n", getId().c_str(), getDescription().c_str() );               

            if( actionObj != NULL )
            {
                actionObj->complete( curTime );
                std::string logStr = "Zone " + zRecord.getZoneName() + " for " + getDurationStr() + " has finished. ( id: " + getId() + " ) ";
                log.addLogEntry( "sch-action-complete", logStr );
            }
        }
    }

    if( start.isBefore( curTime ) )
    {
        // This event should be started
        if( state == SESTATE_READY )
        {
            state = SESTATE_RUNNING;
            printf( "processCurrent - %s:%s - start: %s - cur: %s - end: %s - recycle: %s\n", getId().c_str(), getDescription().c_str(), start.getISOString().c_str(), curTime.getISOString().c_str(), end.getISOString().c_str(), recycle.getISOString().c_str() );
            printf( "processCurrent - %s:%s - Starting\n", getId().c_str(), getDescription().c_str() );   

            if( actionObj != NULL )
            {
                actionObj->start( curTime );
                std::string logStr = "Zone " + zRecord.getZoneName() + " for " + getDurationStr() + " is starting. ( id: " + getId() + " ) ";
                log.addLogEntry( "sch-action-start", logStr );
            }
            
        }
    } 

    // This event should be started
    if( state == SESTATE_RUNNING )
    {
        state = SESTATE_RUNNING;
        //printf( "processCurrent - %s:%s - Running\n", getId().c_str(), getDescription().c_str() ); 

        // Let the action perform polling steps.
        if( actionObj != NULL )
        {
            actionObj->poll( curTime );
        }      
    }

    // This event should be started
    //if( state == SESTATE_COMPLETE )
    //{
    //    printf( "processCurrent - %s:%s - Completed\n", getId().c_str(), getDescription().c_str() );               
    //}


    // No state change.
    return false;
}

void 
ScheduleEvent::processFinal()
{
    printf( "processCurrent - %s:%s - %s\n", getId().c_str(), getDescription().c_str(), recycle.getISOString().c_str() );

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
    rtnNode->defineField( "zoneid", true );
    rtnNode->defineField( "duration", true );

    return rtnNode;
}

void
ScheduleZoneRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;
    ScheduleTimeDuration td;

    std::cout << "ScheduleZoneRule::setFieldsFromContentNode" << std::endl;

    objCN->getField( "zoneid", tmpStr );

    if( tmpStr.empty() == false )
    {
        zoneID = tmpStr;
    }

    std::cout << "ScheduleZoneRule::setFieldsFromContentNode - 1" << std::endl;

    // Optional Fields
    objCN->getField( "type", tmpStr );

    if( tmpStr == "fixedduration" )
    {
        ruleType = SZR_TYPE_FIXED_DURATION;
    }

    std::cout << "ScheduleZoneRule::setFieldsFromContentNode - 2" << std::endl;

    // Optional Fields
    objCN->getField( "duration", tmpStr );

    if( tmpStr.empty() == false )
    {
        td.setFromSeconds( strtol( tmpStr.c_str(), NULL, 0 ) );
        std::cout << "DStr: " << tmpStr << ", " << td.getISOString() << std::endl;
        setDuration( td );
    }
    std::cout << "ScheduleZoneRule::setFieldsFromContentNode - 3" << std::endl;

}

void
ScheduleZoneRule::setContentNodeFromFields( RESTContentNode *objCN )
{
    std::cout << "ScheduleZoneRule::setContentNodeFromFields - 1" << std::endl;

    // Create the root object
    objCN->setAsObject( "schedule-zone-rule" );
    objCN->setID( getID() );
    objCN->setField( "type", getTypeStr() );
    objCN->setField( "duration", getDuration().getSecondsString() );
    objCN->setField( "zoneid", zoneID );
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

std::string 
ScheduleZoneRule::getZoneName()
{
    return ( (ScheduleManager &) objManager ).getZoneName( zoneID );
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
    objCN->setField( "name", getName() );
    objCN->setField( "policy", getZoneEventPolicyStr() );
}

void
ScheduleZoneGroup::setName( std::string value )
{
    name = value;
}

std::string
ScheduleZoneGroup::getName()
{
    return name;
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
ScheduleZoneGroup::createZoneEvents( ScheduleEventList &activeEvents, ScheduleTriggerRecord &tRecord, bool serializeEvents, ScheduleDateTime &rearmTime )
{
    ScheduleDateTime eventTime;

    printf( "ScheduleZoneGroup -- start createZoneEvents\n");
    std::cout << "ScheduleZoneGroup::createZoneEvents0: " << tRecord.getTriggerName() << std::endl;

    // Initial Time
    eventTime.setTime( tRecord.getEventTime() );
    rearmTime.setTime( tRecord.getEventTime() );

    // If events are to be serialized then find the latest event
    // in the active list and put the new events after that.
    if( serializeEvents )
    {
        ScheduleDateTime lastTime;

        lastTime.setTime( tRecord.getEventTime() );

        // Do processing for active rules
        for( unsigned int index = 0; index < activeEvents.getEventCount(); ++index )
        {
            ScheduleDateTime endTime;
            ScheduleEvent *event = activeEvents.getEvent( index );

            event->getEndTime( endTime );

            if( endTime.isAfter( lastTime ) )
            {
                endTime.addSeconds( 1 );
                lastTime.setTime( endTime );
            }
        }

        printf( "ScheduleZoneGroup -- lastTime: %s\n", lastTime.getISOString().c_str() );

        eventTime.setTime( lastTime );
        rearmTime.setTime( lastTime );
    }
    else
    {
        printf( "ScheduleZoneGroup -- curTime: %s\n", tRecord.getEventTime().getISOString().c_str() );

        eventTime.setTime( tRecord.getEventTime() );
        rearmTime.setTime( tRecord.getEventTime() );
    }

    std::vector< std::string > ruleIDList;

    objManager.getIDListForRelationship( getID(), "zone-rule-list", ruleIDList );  

    printf( "ScheduleZoneGroup -- zone rule count: %d\n", (int)ruleIDList.size() );
    
    // Go through the zone list and schedule an event for each one
    for( std::vector< std::string >::iterator it = ruleIDList.begin(); it != ruleIDList.end(); ++it )
    {
        ScheduleZoneRule *zrObj = (ScheduleZoneRule *) objManager.getObjectByID( *it );

        ScheduleZoneRecord zRecord;

        zRecord.setERID( tRecord.getERID() );
        zRecord.setGroupID( getID() );
        zRecord.setRuleID( *it );
        zRecord.setZoneName( zrObj->getZoneName() );

        ScheduleEvent *event = new ScheduleEvent;

        // Copy over identifying data
        event->setTriggerRecord( tRecord );
        event->setZoneRecord( zRecord );

        std::cout << "ScheduleZoneGroup::createZoneEvents: " << event->getTriggerRecord().getTriggerName() << std::endl;

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

ScheduleZoneRecord::ScheduleZoneRecord()
{

}

ScheduleZoneRecord::~ScheduleZoneRecord()
{

}

void 
ScheduleZoneRecord::setERID( std::string idStr )
{
    eventRuleID = idStr;
}

void 
ScheduleZoneRecord::setGroupID( std::string idStr )
{
    groupID = idStr;
}

void 
ScheduleZoneRecord::setRuleID( std::string idStr )
{
    ruleID = idStr;
}

void 
ScheduleZoneRecord::setZoneName( std::string nameStr )
{
    zoneName = nameStr;
}

std::string 
ScheduleZoneRecord::getERID()
{
    return eventRuleID;
}

std::string 
ScheduleZoneRecord::getGroupID()
{
    return groupID;
}

std::string 
ScheduleZoneRecord::getRuleID()
{
    return ruleID;
}

std::string 
ScheduleZoneRecord::getZoneName()
{
    return zoneName;
}

ScheduleTriggerRecord::ScheduleTriggerRecord()
{
    manual = false;
}

ScheduleTriggerRecord::~ScheduleTriggerRecord()
{

}

void 
ScheduleTriggerRecord::setManualTrigger()
{
    manual = true;
}

void 
ScheduleTriggerRecord::clearManualTrigger()
{
    manual = false;
}

bool 
ScheduleTriggerRecord::wasManuallyTriggered()
{
    return manual;
}

void 
ScheduleTriggerRecord::setERID( std::string idStr )
{
    eventRuleID = idStr;
}

void 
ScheduleTriggerRecord::setGroupID( std::string idStr )
{
    groupID = idStr;
}

void 
ScheduleTriggerRecord::setRuleID( std::string idStr )
{
    ruleID = idStr;
}

void 
ScheduleTriggerRecord::setTriggerName( std::string nameStr )
{
    triggerName = nameStr;
}

void 
ScheduleTriggerRecord::setEventTime( ScheduleDateTime &evTime )
{
    eventTime.setTime( evTime );
}

std::string 
ScheduleTriggerRecord::getERID()
{
    return eventRuleID;
}

std::string 
ScheduleTriggerRecord::getGroupID()
{
    return groupID;
}

std::string 
ScheduleTriggerRecord::getRuleID()
{
    return ruleID;
}

std::string 
ScheduleTriggerRecord::getTriggerName()
{
    return triggerName;
}

ScheduleDateTime &
ScheduleTriggerRecord::getEventTime()
{
    return eventTime;
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
    rtnNode->defineField( "scope", false );
    rtnNode->defineField( "reftime", false );

    return rtnNode;
}

void
ScheduleTriggerRule::setFieldsFromContentNode( RESTContentNode *objCN )
{
    std::string tmpStr;

    // Required Fields
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
        scope = SER_TT_SCOPE_NEVER;
        return false;
    }
    else if( scopeStr == SERScopeString[2] )
    {
        scope = SER_TT_SCOPE_MINUTE;
        return false;
    }
    else if( scopeStr == SERScopeString[3] )
    {
        scope = SER_TT_SCOPE_HOUR;
        return false;
    }
    else if( scopeStr == SERScopeString[4] )
    {
        scope = SER_TT_SCOPE_DAY;
        return false;
    }
    else if( scopeStr == SERScopeString[5] )
    {
        scope = SER_TT_SCOPE_WEEK;
        return false;
    }
    else if( scopeStr == SERScopeString[6] )
    {
        scope = SER_TT_SCOPE_EVEN_WEEK;
        return false;
    }
    else if( scopeStr == SERScopeString[7] )
    {
        scope = SER_TT_SCOPE_ODD_WEEK;
        return false;
    }
    else if( scopeStr == SERScopeString[8] )
    {
        scope = SER_TT_SCOPE_YEAR;
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
ScheduleTriggerRule::checkForTimeTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord ) // ScheduleDateTime &eventTime )
{
    ScheduleDateTime startTime;
    ScheduleDateTime prestartTime;

    //std::cout << "ScheduleTriggerRule::checkForTimeTrigger - id: " << getID() << ", " << curTime.getISOString() <<std::endl;

    // Record which rule potentially fired.
    tRecord.setRuleID( getID() ); 

    // Determine if the event will fire and at what time
    switch( scope )
    {
        case SER_TT_SCOPE_NOTSET:
        break;

        case SER_TT_SCOPE_NEVER:
            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( refTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- never - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- never - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- never - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- never - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && refTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( refTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_MINUTE:
            // If we are not in the right second, then exit
            if( refTime.getSecondOfMinute() != curTime.getSecondOfMinute() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceSecondScope( refTime );

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subSeconds( 30 ); 

            printf( "ScheduleTriggerRule -- minute - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- minute - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- minute - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- minute - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_HOUR:
            // If we are not in the right minute, then exit
            if( refTime.getMinuteOfHour() != curTime.getMinuteOfHour() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceMinuteScope( refTime );

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- hour - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- hour - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- hour - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- hour - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_DAY:
            // If we are not in the right hour, then exit
            if( refTime.getHourOfDay() != curTime.getHourOfDay() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceHourScope( refTime );

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- day - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- day - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- day - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- day - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_WEEK:
            // If we are not in the right day, then exit
            if( refTime.getDayOfWeek() != curTime.getDayOfWeek() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceHourScope( refTime );

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- week - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- week - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- week - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- week - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_EVEN_WEEK:
            // If we are not in the right day, then exit
            if( refTime.getDayOfWeek() != curTime.getDayOfWeek() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceHourScope( refTime );

            // It the adjusted start time doesn't fall on an even week
            // then this isn't a match
            if( ( startTime.getWeekNumber() & 0x1 ) != 0 )
                return false;

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- even_week - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- even_week - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- even_week - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- even_week - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_ODD_WEEK:
            // If we are not in the right day, then exit
            if( refTime.getDayOfWeek() != curTime.getDayOfWeek() )
                return false;

            // We are in the ballpark so calculate the local startTime
            // based on the curTime
            startTime.setTime( curTime );
            startTime.replaceHourScope( refTime );

            // It the adjusted start time doesn't fall on an odd week
            // then this isn't a match
            if( ( startTime.getWeekNumber() & 0x1 ) == 0 )
                return false;

            // Schedule the event a couple of minutes before start time.
            prestartTime.setTime( startTime );
            prestartTime.subMinutes( 2 ); 

            printf( "ScheduleTriggerRule -- odd_week - ref: %s\n", refTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- odd_week - prestart: %s\n", prestartTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- odd_week - start: %s\n", startTime.getISOString().c_str() );
            printf( "ScheduleTriggerRule -- odd_week - cur: %s\n", curTime.getISOString().c_str() );

            // Schedule events a bit before their actual start times.
            if( prestartTime.isBefore( curTime ) && startTime.isAfter( curTime ) )
            {
                tRecord.setEventTime( startTime );
                return true;
            }
        break;

        case SER_TT_SCOPE_YEAR:
        break;
    }

    //  Did not trigger
    return false;
}

bool 
ScheduleTriggerRule::checkForTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord ) // ScheduleDateTime &eventTime )
{
    switch( ruleType )
    {
        case STR_TYPE_TIME:
        {
            return checkForTimeTrigger( curTime, tRecord ); // eventTime );
        }
        break;

        default:
        break;
    }

    // Nothing 
    return false;
}

void 
ScheduleTriggerRule::getPotentialTimeTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList )
{
    ScheduleDateTime incTime;

    std::cout << "ScheduleTriggerRule::getPotentialTimeTriggersForPeriod - id: " << getID() << ", " << startTime.getISOString() << ", " << endTime.getISOString() << std::endl;
 
    switch( scope )
    {
        case SER_TT_SCOPE_NOTSET:
        break;

        case SER_TT_SCOPE_NEVER:
            // This event only happens once, so a simple check will do.
            if( refTime.isAfter( startTime ) && refTime.isBefore( endTime ) )
            {
                ScheduleTriggerRecord tRec;

                tRec.setRuleID( getID() );
                tRec.setEventTime( refTime );

                tList.push_back( tRec );
            }
        break;

        case SER_TT_SCOPE_MINUTE:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.replaceSecondScope( refTime );

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addMinutes( 1 );
            }
        break;

        case SER_TT_SCOPE_HOUR:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.replaceMinuteScope( refTime );

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                std::cout << "getPotentialTimeTriggersForPeriod - hour: " << incTime.getISOString() << std::endl;

                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addHours( 1 );
            }
        break;

        case SER_TT_SCOPE_DAY:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.replaceHourScope( refTime );

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addDays( 1 );
            }
        break;

        case SER_TT_SCOPE_WEEK:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.advanceToMatchingWeekDay( refTime );

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addWeeks( 1 );
            }
        break;

        case SER_TT_SCOPE_EVEN_WEEK:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.advanceToMatchingWeekDay( refTime );

            // If the first time isn't in an even week, then add a week
            // to move from odd to even.
            if( ( incTime.getWeekNumber() & 0x1 ) != 0 )
            {
                incTime.addWeeks( 1 );
            }

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addWeeks( 2 );
            }
        break;

        case SER_TT_SCOPE_ODD_WEEK:
            // Move the start time to the first possible occurrence.
            incTime.setTime( startTime );
            incTime.advanceToMatchingWeekDay( refTime );

            // If the first time isn't is an even week, then add a week
            // to move from even to odd.
            if( ( incTime.getWeekNumber() & 0x1 ) == 0 )
            {
                incTime.addWeeks( 1 );
            }

            // Now walk the increment time up until we are past the endTime
            while( incTime.isBefore( endTime ) )
            {
                // Check if this time qualifies.
                if( incTime.isAfter( startTime ) && incTime.isBefore( endTime ) )
                {
                    ScheduleTriggerRecord tRec;

                    tRec.setRuleID( getID() );
                    tRec.setEventTime( incTime );

                    tList.push_back( tRec );
                }

                // Try the next time.
                incTime.addWeeks( 2 );
            }
        break;

        case SER_TT_SCOPE_YEAR:
        break;
    }

    //  Did not trigger
    return;
}

void 
ScheduleTriggerRule::getPotentialTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList )
{
    switch( ruleType )
    {
        case STR_TYPE_TIME:
        {
            getPotentialTimeTriggersForPeriod( startTime, endTime, tList );
            return;
        }
        break;

        default:
        break;
    }

    // Nothing 
    return;
}

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
    rtnNode->defineField( "desc", true );

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

    if( objCN->getField( "desc", tmpStr ) )
    {
        desc = tmpStr;
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
    objCN->setField( "desc", getDescription() );
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
ScheduleTriggerGroup::setDescription( std::string value )
{
    desc = value;
}

std::string 
ScheduleTriggerGroup::getDescription()
{
    return desc;
}

bool 
ScheduleTriggerGroup::checkForTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord )
{
    std::vector< std::string > idList;

    tRecord.setGroupID( getID() );
    tRecord.setTriggerName( getName() );

    objManager.getIDListForRelationship( getID(), "trigger-rule-list", idList );  

    // Search through possible triggers, if any of them go off then we are done.
    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it )
    {
        ScheduleTriggerRule *tgObj = (ScheduleTriggerRule *) objManager.getObjectByID( *it );

        if( tgObj->checkForTrigger( curTime, tRecord ) == true )
        {
            // At least one triggered, return that.
            // Short circuit, no need to check the rest.
            return true;
        }
    }
   
    // Nothing triggered, return nothing to do.
    return false;
}

void 
ScheduleTriggerGroup::getPotentialTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList )
{
    std::vector< std::string > idList;

    objManager.getIDListForRelationship( getID(), "trigger-rule-list", idList );  

    // Search through possible triggers, if any of them go off then we are done.
    for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it )
    {
        ScheduleTriggerRule *tgObj = (ScheduleTriggerRule *) objManager.getObjectByID( *it );

        tgObj->getPotentialTriggersForPeriod( startTime, endTime, tList );
    }

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

static bool seCompAscendStartTime( ScheduleEvent *i, ScheduleEvent *j ) 
{ 
    ScheduleDateTime iTime;
    ScheduleDateTime jTime;

    i->getStartTime( iTime );
    j->getStartTime( jTime );

    return( iTime.isBefore( jTime ) );
}

void
ScheduleEventList::sortAscendingByStartTime()
{
    std::sort( eventList.begin(), eventList.end(), seCompAscendStartTime);
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
    if( objCN->getField( "enabled", tmpStr ) )
    {
        // Set state according to 
        // configuration file.
        if( tmpStr == "true" )
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
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
ScheduleEventRule::updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime, bool serializeEvents )
{
    char tmpStr[64];
    ScheduleDateTime eventTime;
    ScheduleTriggerRecord tRecord;

    //printf( "ScheduleEventRule -- start updateActiveEvents\n");
    //printf( "ScheduleEventRule -- name: %s\n", name.c_str() );
    //printf( "ScheduleEventRule -- zgID: %s\n", zoneGroupID.c_str() );
    //printf( "ScheduleEventRule -- tgID: %s\n", triggerGroupID.c_str() );

    // Only check the rule if it is enabled.
    if( enabled == false )
    {
        // Skip the rule, it wasn't enabled
        return;
    }

    // Lookup and validate the zone-group and trigger-group references
    ScheduleZoneGroup *zoneGroup = (ScheduleZoneGroup *) objManager.getObjectByID( zoneGroupID );

    if( zoneGroup == NULL )
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
        
        eventTime.getCurrentTime();

        tRecord.setERID( getID() );
        tRecord.setManualTrigger();
        tRecord.setEventTime( eventTime );
        tRecord.setTriggerName( "Manual" );

        // Create the events
        zoneGroup->createZoneEvents( activeEvents, tRecord, serializeEvents, rearmTime );

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

    // Lookup and validate the zone-group and trigger-group references
    ScheduleTriggerGroup *triggerGroup = (ScheduleTriggerGroup *) objManager.getObjectByID( triggerGroupID );

    if( triggerGroup == NULL )
    {
        // Skip the rule, trigger groups were not valid.
        return;
    }

    std::string triggerRuleID;

    // Check if this rule triggered

    tRecord.setERID( getID() );
    tRecord.setGroupID( triggerGroupID );
    tRecord.setTriggerName( triggerGroup->getName() );

    if( triggerGroup->checkForTrigger( curTime, tRecord ) )
    {
        zoneGroup->createZoneEvents( activeEvents, tRecord, serializeEvents, rearmTime );
        eventsPending = true;
    }

}

void
ScheduleEventRule::getPotentialEventsForPeriod( ScheduleEventList &activeEvents, ScheduleDateTime startTime, ScheduleDateTime endTime )
{
    char tmpStr[64];
    ScheduleDateTime dummyTime;

    //printf( "ScheduleEventRule -- start updateActiveEvents\n");
    //printf( "ScheduleEventRule -- name: %s\n", name.c_str() );
    //printf( "ScheduleEventRule -- zgID: %s\n", zoneGroupID.c_str() );
    //printf( "ScheduleEventRule -- tgID: %s\n", triggerGroupID.c_str() );

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

    // Check if this rule triggered
    std::vector< ScheduleTriggerRecord > tList;
    triggerGroup->getPotentialTriggersForPeriod( startTime, endTime, tList );
    
    for( std::vector< ScheduleTriggerRecord >::iterator it = tList.begin(); it != tList.end(); it++ )
    {
        it->setERID( getID() );
        it->setGroupID( triggerGroupID );
        it->setTriggerName( triggerGroup->getName() );

        zoneGroup->createZoneEvents( activeEvents, *it, false, dummyTime );
    }
}

ScheduleManager::ScheduleManager()
: eventLog( 100 )
{
    cfgPath         = "/etc/hnode";  
    zoneMgr         = NULL;
    nextID          = 1;
    serializeEvents = true;
    masterEnable    = true;
}

ScheduleManager::~ScheduleManager()
{

}

void
ScheduleManager::setZoneManager( ZoneManager *zMgr )
{
    zoneMgr = zMgr;
}

std::string 
ScheduleManager::getZoneName( std::string zoneID )
{
    std::string rtnStr;
    Zone *zone = NULL;

    if( zoneMgr == NULL )
    {
        return rtnStr;
    }

    zone = zoneMgr->getZoneByID( zoneID );

    if( zone == NULL )
    {
        return rtnStr;
    }

    return zone->getName();
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

    filePath = cfgPath + "/irrigation/schedule_config.xml";

    // Clear out any existing items.
    clear();

    // Load the local timezone information
    localtz.initFromSystemFiles();

    // Read everything from a file
    cfgReader.readConfig( filePath, this );

    eventLog.addLogEntry( "sch-load-config", "The configuration file was loaded." );

    return false;
}

bool
ScheduleManager::saveConfiguration()
{
    ScheduleConfig cfgWriter;
    std::string filePath;

    filePath = cfgPath + "/irrigation/schedule_config.xml";

    // Write everything out to a file
    cfgWriter.writeConfig( filePath, this );

    eventLog.addLogEntry( "sch-save-config", "The configuration file was saved." );

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
        ((ScheduleEventRule *)*it)->updateActiveEvents( activeEvents, curTime, serializeEvents );
    }

    //printf( "ActiveEventCount: %d\n", activeEvents.getEventCount() );

    // Do processing for active rules
    for( unsigned int index = 0; index < activeEvents.getEventCount(); ++index )
    {
        ScheduleEvent *event = activeEvents.getEvent( index );

        // Perform processing for this event
        if( event->processCurrent( curTime, eventLog ) )
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
ScheduleManager::getPotentialEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime )
{
    ScheduleEventList *eventList = new ScheduleEventList();
    ScheduleDateTime curTime; 

    std::vector< RESTContentNode* > rtnVector;

    getObjectVectorByType( SCH_ROTID_EVENTRULE, rtnVector );

    // Check if the rules are going to generate any new events.
    for( std::vector< RESTContentNode* >::iterator it = rtnVector.begin(); it != rtnVector.end(); ++it )
    {
        ((ScheduleEventRule *)*it)->getPotentialEventsForPeriod( *eventList, startTime, endTime );
    }

    // Sort the generated list into chronological order
    eventList->sortAscendingByStartTime();

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
}

void 
ScheduleManager::populateContentNodeFromStatusProvider( unsigned int id, RESTContentNode *outNode, std::map< std::string, std::string > paramMap )
{
    switch( id )
    {
        case SCHRSRC_STATID_STATUS:
        {
            ScheduleEventList *eventList;

            // Give the root element a tag name
            outNode->setAsObject( "irrigation-dashboard" );

            // Give the currect time as we see it
            ScheduleDateTime timestamp;
            timestamp.getCurrentTime();
            outNode->setField( "timestamp", timestamp.getISOString() );

            // Give the controllers timezone
            std::string tmpStr = localtz.getOlsenStr();
            outNode->setField( "timezone", tmpStr );

            // Give the controllers timezone
            tmpStr = localtz.getPosixStr();
            outNode->setField( "posix-timezone", tmpStr );

            // Master enable state
            outNode->setField( "master-enable", masterEnable ? "true" : "false" );

            // Add a list for any active zones
            RESTContentNode *azList = RESTContentHelperFactory::newContentNode();

            azList->setAsArray( "zone-states" );
            outNode->addChild( azList );

            for( int indx = 0; indx < zoneMgr->getZoneCount(); indx++ )
            {
                Zone *zonePtr;

                zonePtr = zoneMgr->getZoneByIndex( indx );

                RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "zone" );

                curNode->setField( "id", zonePtr->getID() );
                curNode->setField( "name", zonePtr->getName() );
                curNode->setField( "state", zonePtr->isStateOn() ? "on" : "off" );

                azList->addChild( curNode );
            }

            // Add todays scheduled events
            RESTContentNode *dsList = RESTContentHelperFactory::newContentNode();

            dsList->setAsArray( "todays-schedule" );
            outNode->addChild( dsList );

/*
            ScheduleDateTime startTime( timestamp );
            startTime.retreatToStartOfDay();

            ScheduleDateTime endTime( timestamp );
            endTime.advanceToEndOfDay();
*/
            ScheduleLocalDateTime startTime( timestamp );
            startTime.retreatToStartOfDay();

            ScheduleLocalDateTime endTime( timestamp );
            endTime.advanceToEndOfDay();

            eventList = getPotentialEventsForPeriod( startTime.getUTCTime(), endTime.getUTCTime() );

            // Do processing for active rules
            for( unsigned int index = 0; index < eventList->getEventCount(); ++index )
            {
                ScheduleEvent *event = eventList->getEvent( index );

                ScheduleDateTime evStart;
                ScheduleDateTime evEnd;

                event->getStartTime( evStart );
                event->getEndTime( evEnd );

                std::cout << "Event Entry -- ID: " << event->getId() << " Title: " << event->getDescription() << " Start: " << evStart.getISOString() << " End: " << evEnd.getISOString() << std::endl;

                RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "event" );

                curNode->setField( "id", event->getId() );
                curNode->setField( "start-time", evStart.getISOString() );
                curNode->setField( "end-time", evEnd.getISOString() );
                curNode->setField( "zone-name", event->getZoneRecord().getZoneName() );
                curNode->setField( "trigger-name", event->getTriggerRecord().getTriggerName() );
                curNode->setField( "duration", event->getDurationStr() );  
                curNode->setField( "erID", event->getTriggerRecord().getERID() );
                curNode->setField( "zgID", event->getZoneRecord().getGroupID() );
                curNode->setField( "zrID", event->getZoneRecord().getRuleID() );
                curNode->setField( "tgID", event->getTriggerRecord().getGroupID() );
                curNode->setField( "trID", event->getTriggerRecord().getRuleID() );
          
                dsList->addChild( curNode );
            } 

            freeScheduleEventList( eventList );
    

            // Add todays log events
            RESTContentNode *evList = RESTContentHelperFactory::newContentNode();

            outNode->addChild( evList );

            eventLog.populateTodaysEventsNode( evList, timestamp );

        }
        break;

        case SCHRSRC_STATID_EVENTLOG:
        {
            eventLog.populateContentNode( outNode );
        }
        break;

        case SCHRSRC_STATID_CALENDAR:
        {
            ScheduleEventList *eventList;
            ScheduleDateTime   startTime;
            ScheduleDateTime   endTime;

            std::map< std::string, std::string >::iterator it = paramMap.find( "startTime" );
 
            if( it == paramMap.end() )
            { 
                startTime.getCurrentTime();
            }
            else
            {
                startTime.setTimeFromISOString( it->second );
            }

            it = paramMap.find( "endTime" );
 
            if( it == paramMap.end() )
            { 
                endTime.setTime( startTime );
                endTime.addHours( 2 );
            }
            else
            {
                endTime.setTimeFromISOString( it->second );
            }

            eventList = getPotentialEventsForPeriod( startTime, endTime );

            // Give the root element a tag name
            outNode->setAsObject( "schedule-event-calendar" );

            outNode->setField( "period-start", startTime.getISOString() );
            outNode->setField( "period-end", endTime.getISOString() );

            RESTContentNode *evList = RESTContentHelperFactory::newContentNode();

            evList->setAsArray( "event-list" );
            outNode->addChild( evList );

            // Do processing for active rules
            for( unsigned int index = 0; index < eventList->getEventCount(); ++index )
            {
                ScheduleEvent *event = eventList->getEvent( index );

                ScheduleDateTime evStart;
                ScheduleDateTime evEnd;

                event->getStartTime( evStart );
                event->getEndTime( evEnd );

                std::cout << "Event Entry -- ID: " << event->getId() << " Title: " << event->getDescription() << " Start: " << evStart.getISOString() << " End: " << evEnd.getISOString() << std::endl;

                RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "event" );

                curNode->setField( "id", event->getId() );
                curNode->setField( "start-time", evStart.getISOString() );
                curNode->setField( "end-time", evEnd.getISOString() );
                curNode->setField( "zone-name", event->getZoneRecord().getZoneName() );
                curNode->setField( "trigger-name", event->getTriggerRecord().getTriggerName() );
                curNode->setField( "duration", event->getDurationStr() );  
                curNode->setField( "erID", event->getTriggerRecord().getERID() );
                curNode->setField( "zgID", event->getZoneRecord().getGroupID() );
                curNode->setField( "zrID", event->getZoneRecord().getRuleID() );
                curNode->setField( "tgID", event->getTriggerRecord().getGroupID() );
                curNode->setField( "trID", event->getTriggerRecord().getRuleID() );
          
                evList->addChild( curNode );
            } 

            freeScheduleEventList( eventList );

        }
        break;

        default:
            std::cerr << "ERROR: Undefined status provider" << std::endl;
            return;
        break;
    }

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

ScheduleEventLogEntry::ScheduleEventLogEntry()
{
    seqNum = 0;
//        ScheduleDateTime  timestamp;
//        std::string       id;
//        std::string       msg;
//        RESTContentNode   extra;

}

ScheduleEventLogEntry::~ScheduleEventLogEntry()
{

}

void 
ScheduleEventLogEntry::setEvent( unsigned long seqNumber, std::string eventID, std::string eventMsg )
{
    // Get the timestamp
    timestamp.getCurrentTime();

    // Set the sequence number
    seqNum = seqNumber;

    // Set the other fields
    id  = eventID;
    msg = eventMsg;
}

void 
ScheduleEventLogEntry::setEvent( unsigned long seqNumber, std::string eventID, std::string eventMsg, RESTContentNode &eventData )
{
    setEvent( seqNumber, eventID, eventMsg );
}

bool 
ScheduleEventLogEntry::onThisDay( ScheduleDateTime &targetDay )
{
    return timestamp.isSameDay( targetDay );
}

std::string 
ScheduleEventLogEntry::getTimestampAsStr()
{
    return timestamp.getExtendedISOString();
}

std::string 
ScheduleEventLogEntry::getEventID()
{
    return id;
}

std::string 
ScheduleEventLogEntry::getEventMsg()
{
    return msg;
}

void
ScheduleEventLogEntry::setContentNodeFromFields( RESTContentNode *objCN )
{
    char snStr[64];

    std::cout << "ScheduleEventLogEntry::setContentNodeFromFields - 1" << std::endl;

    sprintf( snStr, "%ld", seqNum );
 
    // Fill in fields
    objCN->setAsObject( "log-entry" );
    objCN->setField( "timestamp", timestamp.getISOString() );
    objCN->setField( "seqnum", snStr );
    objCN->setField( "event-id", id );
    objCN->setField( "event-msg", msg );
}

ScheduleEventLog::ScheduleEventLog( unsigned long maxEntries )
{
    nextSeqNumber = 0;

    maxSize = maxEntries;
//    std::list< ScheduleEventLogEntry > logData;
}

ScheduleEventLog::~ScheduleEventLog()
{

}

void
ScheduleEventLog::populateContentNode( RESTContentNode *rtnNode )
{
    // Give the root element a tag name
    rtnNode->setAsArray( "schedule-event-log" );

    for( std::list< ScheduleEventLogEntry >::reverse_iterator it = logData.rbegin(); it != logData.rend() ; it++ )
    {
        RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

        curNode->setAsObject( "entry" );
        it->setContentNodeFromFields( curNode );

        rtnNode->addChild( curNode );
    }
}

void
ScheduleEventLog::populateTodaysEventsNode( RESTContentNode *rtnNode, ScheduleDateTime &targetDay )
{
    // Give the root element a tag name
    rtnNode->setAsArray( "todays-events" );

    // Return the events that are on the current day.
    for( std::list< ScheduleEventLogEntry >::reverse_iterator it = logData.rbegin(); it != logData.rend() ; it++ )
    {
        RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

        if( it->onThisDay( targetDay ) )
        {
            curNode->setAsObject( "entry" );
            it->setContentNodeFromFields( curNode );
            rtnNode->addChild( curNode );
        }
    }
}

void 
ScheduleEventLog::addLogEntry( std::string eventID, std::string eventMsg )
{
    ScheduleEventLogEntry newEntry;

    if( logData.size() > maxSize )
    {
        logData.pop_front();
    }

    newEntry.setEvent( nextSeqNumber, eventID, eventMsg );
    nextSeqNumber += 1;

    logData.push_back( newEntry );
}

void 
ScheduleEventLog::addLogEntry( std::string eventID, std::string eventMsg, RESTContentNode &eventData )
{
    ScheduleEventLogEntry newEntry;

    if( logData.size() > maxSize )
    {
        logData.pop_front();
    }

    newEntry.setEvent( nextSeqNumber, eventID, eventMsg, eventData );
    nextSeqNumber += 1;

    logData.push_back( newEntry );
}

