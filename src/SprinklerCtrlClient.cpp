#include <stdio.h>
//#include <curl/curl.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/local_time/local_time.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "REST/REST.hpp"

namespace po = boost::program_options;

bool hasEnding( std::string const &fullString, std::string const &ending ) 
{
    if( fullString.length() >= ending.length() ) 
    {
        return( 0 == fullString.compare( fullString.length() - ending.length(), ending.length(), ending ) );
    } 

    return false;
}

bool 
create_zone_rule( std::string zgID, std::string zoneID, int duration, std::string &zrID )
{
    RESTHttpClient client;
    char durStr[32];
    std::string locID;
    std::string url = "http://localhost:8200/schedule/zone-groups/" + zgID + "/members";

    client.setRequest( RHC_REQTYPE_POST, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-zone-rule" );
    objNode->setField( "zoneid", zoneID );
    objNode->setField( "type", "fixedduration" );

    sprintf( durStr, "%d", duration );
    objNode->setField( "duration", durStr );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();

    client.getLocationHeaderTerminal( zrID );

    std::cout << "zrID: " << zrID << std::endl;
}

bool 
add_zone_rules( std::string zgID, std::string zoneSpecStr )
{
    std::string zrID;
    boost::regex re(",+");
    boost::sregex_token_iterator i( zoneSpecStr.begin(), zoneSpecStr.end(), re, -1);
    boost::sregex_token_iterator j;

    while(i != j)
    {
        std::string zoneSpec = *i;

        std::cout << "ZoneSpec: " << zoneSpec << std::endl;

        boost::regex re2(":+");
        boost::sregex_token_iterator k( zoneSpec.begin(), zoneSpec.end(), re2, -1);

        unsigned int pos = 0;
        std::string zoneID;
        std::string durStr;
        unsigned long duration;
        while( k != j )
        {
            if( pos == 0 )
            {
                std::cout << "zoneID: " << *k << std::endl;
                zoneID = *k;
            }
            else if( pos == 1 )
            {
                std::cout << "Duration: " << *k << std::endl;
                durStr = *k;

                if( hasEnding( durStr, "h" ) || hasEnding( durStr, "H" ) )
                {
                    duration = strtol( durStr.c_str(), NULL, 0 );
                    duration *= 60 * 60;
                }
                else if( hasEnding( durStr, "m" ) || hasEnding( durStr, "M" ) )
                {
                    duration = strtol( durStr.c_str(), NULL, 0 );
                    duration *= 60;
                }
                else
                {
                    duration = strtol( durStr.c_str(), NULL, 0 );
                }

            }

            pos++;
            k++;
        }

        create_zone_rule( zgID, zoneID, duration, zrID );

        i++;
    }

}

bool 
create_zone_group( std::string name, std::string desc, std::string zones, std::string &zgID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/zone-groups";

    client.setRequest( RHC_REQTYPE_POST, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-zone-group" );
    objNode->setField( "name", name );
    objNode->setField( "desc", desc );
    objNode->setField( "policy", "sequential" );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();

    client.getLocationHeaderTerminal( zgID );

    std::cout << "zgID: " << zgID << std::endl;

    // If there is also a zone list then continue processing
    if( zones.size() )
    {
        add_zone_rules( zgID, zones );
    }
}

bool 
update_zone_group( std::string zgID, std::string name, std::string desc )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/zone-groups/" + zgID;

    client.setRequest( RHC_REQTYPE_PUT, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-zone-group" );

    if( name.empty() == false )
    {
        objNode->setField( "name", name );
    }

    if( desc.empty() == false )
    {
        objNode->setField( "desc", desc );
    }

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();
}

bool 
delete_zone_group( std::string zgID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/zone-groups/" + zgID;

    client.setRequest( RHC_REQTYPE_DELETE, url );

    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    client.makeRequest();
}

bool
delete_zone_rule( std::string zgID, std::string zrID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/zone-groups/" + zgID + "/members/" + zrID;

    client.setRequest( RHC_REQTYPE_DELETE, url );

    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    client.makeRequest();
}

bool
create_trigger_group( std::string name, std::string desc, std::string &tgID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/trigger-groups";

    client.setRequest( RHC_REQTYPE_POST, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-trigger-group" );
    objNode->setField( "name", name );
    objNode->setField( "desc", desc );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();

    client.getLocationHeaderTerminal( tgID );

    std::cout << "tgID: " << tgID << std::endl;
}

bool
update_trigger_group( std::string tgID, std::string name, std::string desc )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/trigger-groups/" + tgID;

    client.setRequest( RHC_REQTYPE_PUT, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-trigger-group" );

    if( name.empty() == false )
    {
        objNode->setField( "name", name );
    }

    if( desc.empty() == false )
    {
        objNode->setField( "desc", desc );
    }

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();
}

bool
delete_trigger_group( std::string tgID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/trigger-groups/" + tgID;

    client.setRequest( RHC_REQTYPE_DELETE, url );

    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    client.makeRequest();
}

bool
delete_trigger_rule( std::string tgID, std::string trID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/trigger-groups/" + tgID + "/members/" + trID;

    client.setRequest( RHC_REQTYPE_DELETE, url );

    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    client.makeRequest();
}

boost::local_time::time_zone_ptr
get_system_timezone()
{
    struct tm tmdata;
    time_t curtime;

    curtime = time(NULL);
    localtime_r( &curtime, &tmdata );

    std::ostringstream tzStr;
    tzStr << tmdata.tm_zone << ((tmdata.tm_gmtoff)/(60*60));

    boost::local_time::time_zone_ptr zone( new boost::local_time::posix_time_zone( tzStr.str() ) );

    return zone;
}

typedef enum TriggerRuleSpecRepeatHorizon
{
    TRS_REPEAT_NEVER,
    TRS_REPEAT_MINUTE,
    TRS_REPEAT_HOUR,
    TRS_REPEAT_DAY,
    TRS_REPEAT_WEEK,
    TRS_REPEAT_EVEN_WEEK,
    TRS_REPEAT_ODD_WEEK,
    TRS_REPEAT_YEAR
} TRS_REPEAT_T;

const char *gTRSScopeStrings[] =
{
    "none",      // TRS_REPEAT_NEVER,
    "minute",    // TRS_REPEAT_MINUTE,
    "hour",      // TRS_REPEAT_HOUR,
    "day",       // TRS_REPEAT_DAY,
    "week",      // TRS_REPEAT_WEEK,
    "even_week", // TRS_REPEAT_EVEN_WEEK,
    "odd_week",  // TRS_REPEAT_ODD_WEEK,
    "year"       // TRS_REPEAT_YEAR
};

class TriggerRuleSpec
{
    private:
        TRS_REPEAT_T             scope;
        boost::posix_time::ptime refTime;

    public:
        TriggerRuleSpec( TRS_REPEAT_T repeat, boost::posix_time::ptime time );
       ~TriggerRuleSpec();

        std::string getScopeAsStr();
        std::string getReftimeAsISOStr();
 
        void debugPrint();
};

TriggerRuleSpec::TriggerRuleSpec( TRS_REPEAT_T repeat, boost::posix_time::ptime time ) 
{ 
    scope   = repeat; 
    refTime = time; 
}

TriggerRuleSpec::~TriggerRuleSpec()
{

}

std::string 
TriggerRuleSpec::getScopeAsStr()
{
    return gTRSScopeStrings[ scope ];
}

std::string 
TriggerRuleSpec::getReftimeAsISOStr()
{
    return to_iso_string( refTime );
}

void
TriggerRuleSpec::debugPrint()
{
    boost::local_time::time_zone_ptr zone = get_system_timezone();
    boost::local_time::local_date_time ldtList( refTime, zone );

    std::cout << "TRS - scope: " << getScopeAsStr() << std::endl;    
    std::cout << "TRS - TRef: " << getReftimeAsISOStr() << std::endl;
    std::cout << "TRS - Local: " << ldtList << std::endl;
}

void
process_time_of_day( std::string timeOfDayStr, boost::posix_time::time_duration &duration )
{
    boost::regex timeRE("([0-9]+[0-9]*):*([0-9]*[0-9]*):*([0-9]*[0-9]*)(am|pm|AM|PM)?");
    boost::cmatch timeMatch;

    unsigned int hour   = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    bool is24Hour       = true;
    bool isAM           = true;    

    if( boost::regex_match( timeOfDayStr.c_str(), timeMatch, timeRE ) )
    {
        if( timeMatch[1].str().empty() == false )
        {
            hour = strtol( timeMatch[1].str().c_str(), NULL, 0 );
        }

        if( timeMatch[2].str().empty() == false )
        {
            minute = strtol( timeMatch[2].str().c_str(), NULL, 0 );
        }

        if( timeMatch[3].str().empty() == false )
        {
            second = strtol( timeMatch[3].str().c_str(), NULL, 0 );
        }

        if( ( timeMatch[4].str() == "AM" ) || ( timeMatch[4].str() == "am" )  )
        {
            is24Hour = false;
            isAM     = true;
        }
        else if( ( timeMatch[4].str() == "PM" ) || ( timeMatch[4].str() == "pm" )  )
        {
            is24Hour = false;
            isAM     = false;
        }
        
    }
   
    duration += boost::posix_time::hours( hour );
    duration += boost::posix_time::minutes( minute );
    duration += boost::posix_time::seconds( second );
   
    if( ( is24Hour == false ) && ( isAM == false ) )
    {
        duration += boost::posix_time::hours( 12 );
    }
}

void
process_time_of_day_list( std::string timeOfDayListStr, std::vector< boost::posix_time::time_duration > &tdList )
{
    boost::posix_time::time_duration td( 0, 0, 0, 0 );
    boost::regex listSepRE(",+");

    boost::sregex_token_iterator end;
    boost::sregex_token_iterator timeList( timeOfDayListStr.begin(), timeOfDayListStr.end(), listSepRE, -1);

    while( timeList != end )
    {
        std::string dayStr = *timeList;
 
        td = boost::posix_time::hours(0);

        process_time_of_day( dayStr, td );

        tdList.push_back( td );

        timeList++;
    }
}

void
build_minute_list( std::string timeListStr, std::vector< TriggerRuleSpec > &trList )
{
    boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Monday, boost::gregorian::Jan);
    boost::gregorian::date baseDate = fdm.get_date(2000);

    boost::local_time::time_zone_ptr zone = get_system_timezone();
    std::vector< boost::posix_time::time_duration > tdList;

    boost::regex listSepRE(",+");

    boost::sregex_token_iterator end;
    boost::sregex_token_iterator timeList( timeListStr.begin(), timeListStr.end(), listSepRE, -1);

    while( timeList != end )
    {
        boost::regex timeRE("([0-9]+[0-9]*):*([0-9]*[0-9]*)");
        boost::cmatch timeMatch;
        boost::posix_time::time_duration td( 0, 0, 0, 0 );

        unsigned int minute = 0;
        unsigned int second = 0;

        std::string timeStr = *timeList;
  
        if( boost::regex_match( timeStr.c_str(), timeMatch, timeRE ) )
        {
            if( timeMatch[1].str().empty() == false )
            {
                minute = strtol( timeMatch[1].str().c_str(), NULL, 0 );
            }

            if( timeMatch[2].str().empty() == false )
            {
                second = strtol( timeMatch[2].str().c_str(), NULL, 0 );
            }
        
            td += boost::posix_time::minutes( minute );
            td += boost::posix_time::seconds( second );

            boost::local_time::local_date_time ldt( baseDate, td, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR );

            TriggerRuleSpec trSpec( TRS_REPEAT_HOUR, ldt.utc_time() ); 

            trList.push_back( trSpec );
        }
   
        timeList++;
       
    }  
}

void
build_hour_list( std::string timeListStr, std::vector< TriggerRuleSpec > &trList )
{
    boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Monday, boost::gregorian::Jan);
    boost::gregorian::date baseDate = fdm.get_date(2000);

    boost::local_time::time_zone_ptr zone = get_system_timezone();
    std::vector< boost::posix_time::time_duration > tdList;

    process_time_of_day_list( timeListStr, tdList );

    for( std::vector< boost::posix_time::time_duration >::iterator it = tdList.begin(); it != tdList.end(); ++it )
    {
        boost::local_time::local_date_time ldt( baseDate, *it, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR );

        TriggerRuleSpec trSpec( TRS_REPEAT_DAY, ldt.utc_time() ); 

        trList.push_back( trSpec );
    }    
}

void
build_day_of_week_list( std::string timeListStr, TRS_REPEAT_T repeat, boost::gregorian::date baseDate, std::vector< TriggerRuleSpec > &trList )
{
    boost::local_time::time_zone_ptr zone = get_system_timezone();
    std::vector< boost::posix_time::time_duration > tdList;

    process_time_of_day_list( timeListStr, tdList );

    for( std::vector< boost::posix_time::time_duration >::iterator it = tdList.begin(); it != tdList.end(); ++it )
    {
        boost::local_time::local_date_time ldt( baseDate, *it, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR );

        TriggerRuleSpec trSpec( repeat, ldt.utc_time() ); 

        trList.push_back( trSpec );
    }    
}



void
process_time_set_list( std::string timeListStr, std::vector< TriggerRuleSpec > &trList )
{
    boost::regex listSepRE(";+");

    boost::regex ruleRE("(Once|Minute|Hour|Mon|Tue|Wed|Thu|Fri|Sat|Sun|EMon|ETue|EWed|EThu|EFri|ESat|ESun|OMon|OTue|OWed|OThu|OFri|OSat|OSun)@([0-9,:|am|pm|AM|PM]*)");

    boost::sregex_token_iterator dayAndTime( timeListStr.begin(), timeListStr.end(), listSepRE, -1);
    boost::sregex_token_iterator end;

    while( dayAndTime != end )
    {
        std::string tmpStr = *dayAndTime;

        boost::cmatch ruleMatch;
        if( boost::regex_match( tmpStr.c_str(), ruleMatch, ruleRE ) )
        {
            if( "Once" == ruleMatch[1] )
            {

            }
            else if( "Minute" == ruleMatch[1] )
            {
                build_minute_list( ruleMatch[2], trList );
            }
            else if( "Hour" == ruleMatch[1] )
            {
                build_hour_list( ruleMatch[2],  trList );
            }
            else if( "Mon" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Monday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Tue" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Tuesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Wed" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Wednesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Thu" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Thursday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Fri" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Friday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Sat" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Saturday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "Sun" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Sunday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_WEEK, d,  trList );
            }
            else if( "EMon" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Monday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "ETue" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Tuesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "EWed" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Wednesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "EThu" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Thursday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "EFri" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Friday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "ESat" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Saturday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "ESun" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Sunday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_EVEN_WEEK, d,  trList );
            }
            else if( "OMon" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Monday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);

                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OTue" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Tuesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OWed" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Wednesday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OThu" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Thursday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OFri" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Friday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OSat" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Saturday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else if( "OSun" == ruleMatch[1] )
            {
                boost::gregorian::first_day_of_the_week_in_month fdm( boost::gregorian::Sunday, boost::gregorian::Jan);
                boost::gregorian::date d = fdm.get_date(2000);
                d += boost::gregorian::days(7);
 
                build_day_of_week_list( ruleMatch[2], TRS_REPEAT_ODD_WEEK, d,  trList );
            }
            else
            {
                std::cerr << "ERROR: Unrecognized time scope - skipping: " << tmpStr << std::endl; 
                dayAndTime++;
                continue;
            }

        }

        dayAndTime++;
    }        
}

bool 
create_trigger_rule( std::string parentID, TriggerRuleSpec &trSpec, std::string &trID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/trigger-groups/" + parentID + "/members";

    client.setRequest( RHC_REQTYPE_POST, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-trigger-rule" );
    objNode->setField( "type", "time" );
    objNode->setField( "scope", trSpec.getScopeAsStr() );
    objNode->setField( "reftime", trSpec.getReftimeAsISOStr() );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();

    client.getLocationHeaderTerminal( trID );

    std::cout << "trID: " << trID << std::endl;
}

void
add_trigger_rules( std::string tgID, std::string timeSpec )
{
    std::vector< TriggerRuleSpec > trList;
    std::string trID;

    process_time_set_list( timeSpec, trList );

    for( std::vector< TriggerRuleSpec >::iterator it = trList.begin(); it != trList.end(); ++it )
    {
        it->debugPrint();
        create_trigger_rule( tgID, *it, trID );
    }    
}

bool
create_schedule_rule( std::string name, std::string desc, std::string zgID, std::string tgID, std::string &erID )
{
    RESTHttpClient client;
    std::string locID;

    client.setRequest( RHC_REQTYPE_POST, "http://localhost:8200/schedule/rules" );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-event-rule" );
    objNode->setField( "name", name );
    objNode->setField( "desc", desc );
    objNode->setField( "zone-group-id", zgID );
    objNode->setField( "trigger-group-id", tgID );
    objNode->setField( "enabled", "true" );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();

    client.getLocationHeaderTerminal( locID );

    std::cout << "New ObjID: " << locID << std::endl;
}

bool
update_schedule_rule( std::string seID, std::string name, std::string desc, std::string zgID, std::string tgID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/rules/" + seID;

    client.setRequest( RHC_REQTYPE_PUT, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-event-rule" );

    if( name.empty() == false )
    {
        objNode->setField( "name", name );
    }

    if( desc.empty() == false )
    {
        objNode->setField( "desc", desc );
    }

    if( zgID.empty() == false )
    {
        objNode->setField( "zone-group-id", zgID );
    }

    if( tgID.empty() == false )
    {
        objNode->setField( "trigger-group-id", tgID );
    }

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();
}

bool
delete_schedule_rule( std::string seID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/rules/" + seID;

    client.setRequest( RHC_REQTYPE_DELETE, url );

    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    client.makeRequest();
}

bool
enable_schedule_rule( std::string seID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/rules/" + seID;

    client.setRequest( RHC_REQTYPE_PUT, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-event-rule" );
    objNode->setField( "enabled", "true" );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();
}

bool
disable_schedule_rule( std::string seID )
{
    RESTHttpClient client;
    std::string locID;
    std::string url = "http://localhost:8200/schedule/rules/" + seID;

    client.setRequest( RHC_REQTYPE_PUT, url );

    client.getOutboundRepresentation().setSimpleContent( "application/xml" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Allocate the appropriate type of helper to parse the content
    RESTContentHelper *helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getOutboundRepresentation() );
    RESTContentNode *objNode  = helper->getRootNode();

    objNode->setAsObject( "schedule-event-rule" );
    objNode->setField( "enabled", "false" );

    helper->generateContentRepresentation( &client.getOutboundRepresentation() );

    client.makeRequest();
}

void
get_event_log()
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Acquire the log data
    client.setRequest( RHC_REQTYPE_GET, "http://localhost:8200/schedule/event-log/" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return;
    }

    // Parse and display the response data
    printf("Return successfully parsed:\n");

    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( strcmp( (const char *)rootNode->name, "schedule-event-log" ) == 0 )
    {
        xmlNode *entryNode = NULL;

        for( entryNode = rootNode->children; entryNode; entryNode = entryNode->next )
        {
            xmlNode *curNode = NULL;

            unsigned long seqNum;
            std::string timestamp;
            boost::posix_time::ptime pstamp;
            std::string eventID;
            std::string eventMsg;

            for( curNode = entryNode->children; curNode; curNode = curNode->next )
            {
                if( strcmp( (const char *)curNode->name, "seqnum" ) == 0 )
                {
                    xmlChar *content = xmlNodeGetContent( curNode );
                    seqNum = strtol( (const char *)content, NULL, 0);
                    xmlFree( content );
                }
                else if( strcmp( (const char *)curNode->name, "timestamp" ) == 0 )
                {
                    xmlChar *content = xmlNodeGetContent( curNode );
                    timestamp = (const char *)content;

                    pstamp = boost::posix_time::from_iso_string( timestamp );

                    boost::local_time::time_zone_ptr zone = get_system_timezone();
                    boost::local_time::local_date_time ltstamp( pstamp, zone );

                    timestamp = boost::posix_time::to_simple_string( ltstamp.local_time() );

                    xmlFree( content );
                }
                else if( strcmp( (const char *)curNode->name, "event-id" ) == 0 )
                {
                    xmlChar *content = xmlNodeGetContent( curNode );
                    eventID = (const char *)content;
                    xmlFree( content );
                }
                else if( strcmp( (const char *)curNode->name, "event-msg" ) == 0 )
                {
                    xmlChar *content = xmlNodeGetContent( curNode );
                    eventMsg = (const char *)content;
                    xmlFree( content );
                }
            }

            printf( "%-5ld %-25s %-25s %s\n", seqNum, timestamp.c_str(), eventID.c_str(), eventMsg.c_str() );
        }
    }

    xmlFreeDoc( doc );
}

void
get_status()
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Acquire the log data
    client.setRequest( RHC_REQTYPE_GET, "http://localhost:8200/schedule/status/" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return;
    }

    // Parse and display the response data
    printf("Return successfully parsed:\n");

    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( strcmp( (const char *)rootNode->name, "schedule-status" ) == 0 )
    {
        xmlNode *curNode = NULL;

        unsigned long seqNum;
        std::string timestamp;
        boost::posix_time::ptime pstamp;
      
        for( curNode = rootNode->children; curNode; curNode = curNode->next )
        {
            if( strcmp( (const char *)curNode->name, "timestamp" ) == 0 )
            {
                xmlChar *content = xmlNodeGetContent( curNode );

                pstamp = boost::posix_time::from_iso_string( (const char *)content );

                boost::local_time::time_zone_ptr zone = get_system_timezone();
                boost::local_time::local_date_time ltstamp( pstamp, zone );

                timestamp = boost::posix_time::to_simple_string( ltstamp.local_time() );

                xmlFree( content );
            }
        }

        printf( "==== Current Status ====\n" );
        printf( "Current Time: %s\n", timestamp.c_str() );
    }

    xmlFreeDoc( doc );
}

void
get_calendar( std::string periodStr )
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Setup
    client.setRequest( RHC_REQTYPE_GET, "http://localhost:8200/schedule/calendar/" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );

    // Determine the data range parameters
    if( periodStr.empty() )
    {
        // Default to getting the current days events
        periodStr = "day";
    }

    boost::local_time::time_zone_ptr zone = get_system_timezone();
    boost::posix_time::time_duration std( 0, 0, 0, 0 );
    boost::posix_time::time_duration etd( 23, 59, 59, 0 );

    if( periodStr == "day" )
    {
        boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );

        boost::local_time::local_date_time start( ltstamp.date(), std, zone, false );  
        boost::local_time::local_date_time end( ltstamp.date(), etd, zone, false );  

        client.getOutboundRepresentation().addQueryParameter( "startTime", to_iso_string( start.utc_time() ) );
        client.getOutboundRepresentation().addQueryParameter( "endTime", to_iso_string( end.utc_time() ) );
    } 
    else if( periodStr == "week" )
    {
        boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );
        unsigned long dayofWeek = ltstamp.date().day_of_week();
        ltstamp -= boost::gregorian::days( dayofWeek );

        boost::local_time::local_date_time start( ltstamp.date(), std, zone, false );  
        boost::local_time::local_date_time end( (ltstamp.date() + boost::gregorian::days(6)), etd, zone, false );  

        client.getOutboundRepresentation().addQueryParameter( "startTime", to_iso_string( start.utc_time() ) );
        client.getOutboundRepresentation().addQueryParameter( "endTime", to_iso_string( end.utc_time() ) );
    }
    else if( periodStr == "twoweek" )
    {
        boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );
        unsigned long dayofWeek = ltstamp.date().day_of_week();
        ltstamp -= boost::gregorian::days( dayofWeek );

        boost::local_time::local_date_time start( ltstamp.date(), std, zone, false );  
        boost::local_time::local_date_time end( (ltstamp.date() + boost::gregorian::days(13)), etd, zone, false );  

        client.getOutboundRepresentation().addQueryParameter( "startTime", to_iso_string( start.utc_time() ) );
        client.getOutboundRepresentation().addQueryParameter( "endTime", to_iso_string( end.utc_time() ) );
    }
    else if( periodStr == "month" )
    {
        boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );

        unsigned long dayofMonth = ltstamp.date().day();
        unsigned long endofMonth = ltstamp.date().end_of_month().day();

        boost::local_time::local_date_time start( (ltstamp.date() - boost::gregorian::days(dayofMonth-1)), std, zone, false );  
        boost::local_time::local_date_time end( (start.date() + boost::gregorian::days(endofMonth-1)), etd, zone, false );  

        client.getOutboundRepresentation().addQueryParameter( "startTime", to_iso_string( start.utc_time() ) );
        client.getOutboundRepresentation().addQueryParameter( "endTime", to_iso_string( end.utc_time() ) );
    }
    else
    {
        std::cerr << "ERROR: Requested period is not supported." << std::endl;
        return;
    }

    // Acquire the calendar data
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return;
    }

    // Parse and display the response data
    printf("Return successfully parsed:\n");

    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( strcmp( (const char *)rootNode->name, "schedule-event-calendar" ) == 0 )
    {
        xmlNode *curNode = NULL;

        for( curNode = rootNode->children; curNode; curNode = curNode->next )
        {
            if( strcmp( (const char *)curNode->name, "period-start" ) == 0 )
            {

            }
            else if( strcmp( (const char *)curNode->name, "period-end" ) == 0 )
            {

            } 
            else if( strcmp( (const char *)curNode->name, "event-list" ) == 0 )
            {
                xmlNodePtr entryNode = NULL;

                boost::posix_time::ptime pstamp;
                std::string startTime;
                std::string endTime;
                std::string eventID;
                std::string zoneName;
                std::string triggerName;
                std::string duration;

                // <event><end-time>20150701T135200</end-time><id>zg1</id><start-time>20150701T135000</start-time><title>zr2-2015-Jul-01 13:50:00</title></event>
                for( entryNode = curNode->children; entryNode; entryNode = entryNode->next )
                {
                    xmlNodePtr childNode = NULL;

                    for( childNode = entryNode->children; childNode; childNode = childNode->next )
                    {
                        if( strcmp( (const char *)childNode->name, "start-time" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            startTime = (const char *)content;

                            pstamp = boost::posix_time::from_iso_string( startTime );

                            boost::local_time::time_zone_ptr zone = get_system_timezone();
                            boost::local_time::local_date_time ltstamp( pstamp, zone );

                            startTime = boost::posix_time::to_simple_string( ltstamp.local_time() );

                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "end-time" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            endTime = (const char *)content;

                            pstamp = boost::posix_time::from_iso_string( endTime );

                            boost::local_time::time_zone_ptr zone = get_system_timezone();
                            boost::local_time::local_date_time ltstamp( pstamp, zone );

                            endTime = boost::posix_time::to_simple_string( ltstamp.local_time() );

                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "id" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            eventID = (const char *)content;
                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "zone-name" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            zoneName = (const char *)content;
                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "trigger-name" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            triggerName = (const char *)content;
                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "duration" ) == 0 )
                        {
                            xmlChar *content = xmlNodeGetContent( childNode );
                            duration = (const char *)content;
                            xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "erID" ) == 0 )
                        {
                            //xmlChar *content = xmlNodeGetContent( childNode );
                            //eventTitle = (const char *)content;
                            //xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "zgID" ) == 0 )
                        {
                            //xmlChar *content = xmlNodeGetContent( childNode );
                            //eventTitle = (const char *)content;
                            //xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "zrID" ) == 0 )
                        {
                            //xmlChar *content = xmlNodeGetContent( childNode );
                            //eventTitle = (const char *)content;
                            //xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "tgID" ) == 0 )
                        {
                            //xmlChar *content = xmlNodeGetContent( childNode );
                            //eventTitle = (const char *)content;
                            //xmlFree( content );
                        }
                        else if( strcmp( (const char *)childNode->name, "trID" ) == 0 )
                        {
                            //xmlChar *content = xmlNodeGetContent( childNode );
                            //eventTitle = (const char *)content;
                            //xmlFree( content );
                        }

                    }

                    printf( "%-22s %-22s %-10s %-20s %-20s %s\n", startTime.c_str(), endTime.c_str(), duration.c_str(), zoneName.c_str(), triggerName.c_str(), eventID.c_str() );

                }
            }
        }
    }

    xmlFreeDoc( doc );

}

void 
get_objects_for_ids( std::string url, std::string objName, std::vector< std::string > &idList, std::vector< RESTContentNode > &objList )
{

    for( std::vector< std::string >::iterator it = idList.begin(); it != idList.end(); it++ )
    {
        RESTHttpClient     client;
        RESTContentHelper *helper;

        std::string extURL = url + "/" + *it;
 
        // Acquire the log data
        client.setRequest( RHC_REQTYPE_GET, extURL );
        client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
        client.makeRequest();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( &client.getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseRawData( &client.getInboundRepresentation() ); 

        // Copy it to the list
        objList.push_back( *(helper->getRootNode()) );

        // Free the helper
        RESTContentHelperFactory::freeContentHelper( helper );
    }
}

void 
get_list_of_ids( std::string url, std::string listName, std::vector< std::string > &idList )
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Acquire the log data
    client.setRequest( RHC_REQTYPE_GET, url );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return;
    }

    // Parse and display the response data
    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( strcmp( (const char *)rootNode->name, listName.c_str() ) == 0 )
    {
        xmlNode *curNode = NULL;

        for( curNode = rootNode->children; curNode; curNode = curNode->next )
        {
            if( strcmp( (const char *)curNode->name, "id" ) == 0 )
            {
                xmlChar *content = xmlNodeGetContent( curNode );
                idList.push_back( (const char *)content );
                xmlFree( content );
            }
        }
    }
}

void 
display_id_list( std::string title, std::vector< std::string > &idList )
{
    std::cout << "==== " << title << " ====" << std::endl;

    for( std::vector< std::string >::iterator it = idList.begin(); it != idList.end(); it++ )
    {
        std::cout << *it << std::endl;
    }
}

void 
display_rawobj( unsigned long offset, std::string title, RESTContentNode &obj )
{
    std::vector< RESTContentField* > fields;

    std::cout << std::setw( offset ) << " " << "==== " << title << " ====" << std::endl;

    fields = obj.getFieldList();
     
    for( std::vector< RESTContentField* >::iterator fit = fields.begin(); fit != fields.end(); fit++ )
    {
        std::cout << std::setw( offset+2 ) << " " << std::setw( 10 ) << std::left << (*fit)->getName() << ": " << (*fit)->getValue() << std::endl;
    }
}

void 
display_rawobj_list( unsigned long offset, std::string title, std::vector< RESTContentNode > &objList )
{
    for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
    {  
        std::string hdr = title + "( id: " + it->getID() + " )";
        display_rawobj( offset + 4, hdr, *it );
        std::cout << std::endl;
    }
}

void 
display_trigger_rule( unsigned long offset, RESTContentNode &obj )
{
    std::string tmpStr;
    std::string idStr;
    std::string typeStr;

    if( obj.getField( "type", typeStr ) == false )
    {
        return;
    }

    if( obj.getField( "id", idStr ) == false )
    {
        return;
    }

    if( "time" == typeStr )
    {
        std::string refTimeStr;
        std::string scopeStr;

        boost::posix_time::ptime pstamp;

        if( obj.getField( "scope", scopeStr ) == false )
        {
            return;
        } 

        if( obj.getField( "reftime", refTimeStr ) == false )
        {
            return;
        } 

        pstamp = boost::posix_time::from_iso_string( refTimeStr );

        boost::local_time::time_zone_ptr zone = get_system_timezone();
        boost::local_time::local_date_time ltstamp( pstamp, zone );

        if( gTRSScopeStrings[ TRS_REPEAT_MINUTE ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%S sec each minute") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_HOUR ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%M:%S min/sec each hour") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_DAY ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%I:%M:%S %p each day") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_WEEK ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%A %I:%M:%S %p of each week") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_EVEN_WEEK ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%A %I:%M:%S %p of EVEN weeks") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_ODD_WEEK ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%A at %I:%M:%S %p of ODD weeks") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }
        else if( gTRSScopeStrings[ TRS_REPEAT_YEAR ] == scopeStr )
        {
            boost::local_time::local_time_facet* ltfacet( new boost::local_time::local_time_facet("%x %I:%M:%S %p of each year") );
            std::cout.imbue( std::locale(std::cout.getloc(), ltfacet) );

            std::cout << std::setw( offset ) << " " << idStr << ": " << ltstamp << std::endl;
        }

    }
    else
    {
        display_rawobj( offset, "Trigger Rule", obj );
    }
}

void 
display_trigger_rule_list( unsigned long offset, std::vector< RESTContentNode > &objList )
{
    for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
    {  
        display_trigger_rule( offset, *it );
    }
}

void 
display_trigger_group( unsigned long offset, RESTContentNode &obj )
{
    std::string idStr;
    std::string nameStr;
    std::string descStr;

    obj.getField( "id", idStr );
    obj.getField( "name", nameStr );
    obj.getField( "desc", descStr );

    std::cout << std::setw( offset ) << " " << idStr << ": " << nameStr << " - " << descStr << std::endl;
}

void 
display_zone_rule( unsigned long offset, RESTContentNode &obj )
{
    std::string tmpStr;
    std::string idStr;
    std::string typeStr;

    if( obj.getField( "type", typeStr ) == false )
    {
        return;
    }

    if( obj.getField( "id", idStr ) == false )
    {
        return;
    }

    if( "fixedduration" == typeStr )
    {
        std::string durStr;
        std::string zoneIDStr;
        unsigned long duration;
        char tdStr[128];

        if( obj.getField( "duration", durStr ) == false )
        {
            return;
        } 

        if( obj.getField( "zoneid", zoneIDStr ) == false )
        {
            return;
        } 

        duration = strtol( durStr.c_str(), NULL, 0 );

        durStr.clear();

        if( duration > (60*60) )
        {
            unsigned long hours = duration / (60*60);
            sprintf(tdStr, "%ld hours ", hours);
            durStr += tdStr;
            duration -= (hours * 60 * 60);
        }

        if( duration > 60 )
        {
            unsigned long mins = duration / 60;
            sprintf(tdStr, "%ld minutes ", mins);
            durStr += tdStr;
            duration -= (mins * 60);
        }

        if( duration )
        {
            sprintf(tdStr, "%ld seconds ", duration);
            durStr += tdStr;
        }

        std::cout << std::setw( offset ) << " " << idStr << ": " << zoneIDStr  << " for " << durStr << std::endl;

    }
    else
    {
        display_rawobj( offset, "Zone Rule", obj );
    }
}

void 
display_zone_rule_list( unsigned long offset, std::vector< RESTContentNode > &objList )
{
    for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
    {  
        display_zone_rule( offset, *it );
    }
}

void 
display_zone_group( unsigned long offset, RESTContentNode &obj )
{
    std::string idStr;

    obj.getField( "id", idStr );

    std::cout << std::setw( offset ) << " " << idStr << ": " << "Sequential zone group" << std::endl;
}

void 
display_schedule_event( unsigned long offset, RESTContentNode &obj )
{
    std::string enableStr;
    std::string idStr;
    std::string nameStr;
    std::string descStr;

    if( obj.getField( "id", idStr ) == false )
    {
        return;
    }

    obj.getField( "name", nameStr );
    obj.getField( "desc", descStr );
    obj.getField( "enabled", enableStr );

    std::cout << std::setw( offset ) << " " << idStr << ": " << nameStr  << "( " << ( enableStr == "true" ? "enabled" : "disabled" ) << " )" << " - " << descStr << std::endl;
}

bool
get_zone_ids( std::vector< std::string > &idList )
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Acquire the log data
    client.setRequest( RHC_REQTYPE_GET, "http://localhost:8200/zones" );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return true;
    }

    // Parse and display the response data
    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( strcmp( (const char *)rootNode->name, "hnode-zonelist" ) == 0 )
    {
        xmlNodePtr idNode = NULL;

        for( idNode = rootNode->children; idNode; idNode = idNode->next ) 
        {
            std::cout << idNode->name << std::endl;

            if( ( idNode->type == XML_ELEMENT_NODE ) && ( strcmp( (const char *)idNode->name, "zoneid" ) == 0 ) )
            {
                xmlChar *idStr;
                idStr = xmlNodeGetContent( idNode );
                std::string tmpStr = (const char *)idStr;
                idList.push_back( tmpStr );
                xmlFree( idStr );
            }
        }
    }

    xmlFreeDoc(doc);

    return false;
}

bool
get_zone_object( std::string zoneID, std::map< std::string, std::string > &objFields )
{
    RESTHttpClient client;
    xmlDocPtr doc;
    std::string contentType;
    unsigned long dataLength;
    unsigned char *dataPtr;

    // Acquire the log data
    client.setRequest( RHC_REQTYPE_GET, "http://localhost:8200/zones/" + zoneID );
    client.getOutboundRepresentation().addHTTPHeader( "Accept", "*/*" );
    client.makeRequest();

    // Get access to the returned data
    dataPtr = client.getInboundRepresentation().getSimpleContentPtr( contentType, dataLength );

    // Run it through the xml parser
    doc = xmlReadMemory( (const char *)dataPtr, dataLength, "noname.xml", NULL, 0 );

    if( doc == NULL )
    {
        return true;
    }

    // Parse and display the response data
    xmlNodePtr rootNode = xmlDocGetRootElement( doc );

    if( ( !rootNode ) || ( strcmp( (const char *)rootNode->name, "zone" ) != 0 ) )
    {
        fprintf(stderr, "Unrecognized return data.\n");
	    return true;
    }
 
    xmlNodePtr zoneNode = NULL;

    for( zoneNode = rootNode->children; zoneNode; zoneNode = zoneNode->next ) 
    {
        if( zoneNode->type == XML_ELEMENT_NODE )
        {
            xmlChar *tmpStr;
            tmpStr = xmlNodeGetContent( zoneNode );
            std::pair< std::string, std::string > entry( (const char *)zoneNode->name, (const char *)tmpStr );
            objFields.insert( entry );
            xmlFree( tmpStr );
        }
    }

    xmlFreeDoc(doc);

    return false;
}

void
get_zone_list( bool detail )
{
    std::vector< std::string > idList;

    std::cout << "ZL: " << detail << std::endl;

    if( get_zone_ids( idList ) == true )
    {
        std::cerr << "ERROR: Unable to retrieve zone id list." << std::endl;
        return;
    }

    // Output the zone IDs
    std::cout << "=== Zone IDs (Count: " << idList.size() << " ) ===" << std::endl;
    for( std::vector< std::string >::iterator it = idList.begin(); it != idList.end(); it++ )
    {
        std::map< std::string, std::string > objFields;

        std::cout << "Zone ID: " << *it << std::endl;
        
        if( detail )
        {
            get_zone_object( *it, objFields );

            for( std::map< std::string, std::string >::iterator fi = objFields.begin(); fi != objFields.end(); ++fi )
            {
                std::cout << "    " << fi->first << " : " << fi->second << std::endl;
            }
        }
    }

    std::cout << std::endl;
}

void get_zone_group_rules( bool detail, std::string zgID, std::vector< std::string > &idList, std::vector< RESTContentNode > &objList )
{
    idList.clear();
    objList.clear();

    std::string url = "http://localhost:8200/schedule/zone-groups/" + zgID + "/members";

    get_list_of_ids( url, "zone-rule-list", idList );

    if( detail )
    {
        get_objects_for_ids( url, "tmpname", idList, objList );
    }

}

void 
get_zone_group_details( std::string zgID, RESTContentNode &zgObj, std::vector< RESTContentNode > &zroList )
{
    std::vector< std::string > idList;
    std::vector< RESTContentNode > objList;

    std::string url = "http://localhost:8200/schedule/zone-groups";

    idList.push_back( zgID );

    get_objects_for_ids( url, "tmpname", idList, objList );

    zgObj = objList[0];

    get_zone_group_rules( true, zgID, idList, zroList ); 
}

void 
get_zone_groups( bool detail )
{
    std::vector< std::string > idList;
    std::vector< RESTContentNode > objList;
    unsigned long offset = 0;

    std::string url = "http://localhost:8200/schedule/zone-groups";

    get_list_of_ids( url, "zone-group-list", idList );

    if( detail )
    {
        get_objects_for_ids( url, "tmpname", idList, objList );

        for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
        {  
            std::vector< std::string > zriList;
            std::vector< RESTContentNode > zroList;

            get_zone_group_rules( true, it->getID(), zriList, zroList ); 
    
    
            display_zone_group( offset, *it );
            display_zone_rule_list( offset+4, zroList );

            std::cout << std::endl;
        }
    }
    else
        display_id_list( "Zone Groups", idList );
}

void get_trigger_group_rules( bool detail, std::string tgID, std::vector< std::string > &idList, std::vector< RESTContentNode > &objList )
{
    idList.clear();
    objList.clear();

    std::string url = "http://localhost:8200/schedule/trigger-groups/" + tgID + "/members";

    get_list_of_ids( url, "trigger-rule-list", idList );

    if( detail )
    {
        get_objects_for_ids( url, "tmpname", idList, objList );
    }
}

void 
get_trigger_group_details( std::string tgID, RESTContentNode &tgObj, std::vector< RESTContentNode > &troList )
{
    std::vector< std::string > idList;
    std::vector< RESTContentNode > objList;

    std::string url = "http://localhost:8200/schedule/trigger-groups";

    idList.push_back( tgID );

    get_objects_for_ids( url, "tmpname", idList, objList );

    tgObj = objList[0];

    get_trigger_group_rules( true, tgID, idList, troList ); 
}

void 
get_trigger_groups( bool detail )
{
    std::vector< std::string > idList;
    std::vector< RESTContentNode > objList;
    unsigned long offset = 0;

    std::string url = "http://localhost:8200/schedule/trigger-groups";

    get_list_of_ids( url, "trigger-group-list", idList );

    if( detail )
    {
        get_objects_for_ids( url, "tmpname", idList, objList );

        for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
        {  
            std::vector< std::string > triList;
            std::vector< RESTContentNode > troList;

            get_trigger_group_rules( true, it->getID(), triList, troList ); 
    
            display_trigger_group( offset, *it );
            display_trigger_rule_list( offset+4, troList );

            std::cout << std::endl;
        }

    }
    else
        display_id_list( "Trigger Groups", idList );
}

void get_schedule_events( bool detail )
{
    std::vector< std::string > idList;
    std::vector< RESTContentNode > objList;

    std::string url = "http://localhost:8200/schedule/rules";

    get_list_of_ids( url, "event-rule-list", idList );

    if( detail )
    {
        get_objects_for_ids( url, "tmpname", idList, objList );

        for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); it++ )
        {  
            unsigned long offset = 0;
            RESTContentNode zgObj;
            std::vector< RESTContentNode > zroList;
            RESTContentNode tgObj;
            std::vector< RESTContentNode > troList;

            std::string tgID;
            it->getField( "trigger-group-id", tgID );
            if( tgID.empty() == false )
            {
                get_trigger_group_details( tgID, tgObj, troList );
            }

            std::string zgID;
            it->getField( "zone-group-id", zgID );
            if( zgID.empty() == false )
            {
                get_zone_group_details( zgID, zgObj, zroList );
            }

            display_schedule_event( offset, *it );
            std::cout << std::endl;

            offset += 4;

            if( tgID.empty() == false )
            {
                display_trigger_group( offset, tgObj );
                display_trigger_rule_list( offset+4, troList );
                std::cout << std::endl;
            }

            if( zgID.empty() == false )
            {
                display_zone_group( offset, zgObj );
                display_zone_rule_list( offset+4, zroList );
                std::cout << std::endl;
            }
        }

    }
    else
        display_id_list( "Schedule Rules", idList );
}

int main( int argc, char* argv[] )
{
    std::string descStr;
    std::string nameStr;

    std::string seID;
    std::string zgID;
    std::string tgID;
    std::string zrID;
    std::string trID;
    std::string zoneID;

    std::string zoneSpecStr;

    std::string timeSpecStr;
    std::string sdiListStr;

    std::string periodStr;

    // Declare the supported options.
    po::options_description desc("HNode Sprinkler Control Client");
    desc.add_options()
        ("help", "produce help message")

        ("get-event-log", "Get the event log.")
        ("get-status", "Get current schedule status.")
        ("get-calendar", "Get the potential events for a period of time.")

        ("period",  po::value<std::string>(&periodStr), "Specify a calendar period. (today, week, twoweek, month)")

        ("zone-list", "Get a list of available zones.")
        ("zone-groups", "Get a list of zone groups.  Specify the --detail parameter to get more than ids." )
        ("trigger-groups", "Get a list of trigger groups.  Specify the --detail parameter to get more than ids." )
        ("schedule-events", "Get a list of schedule events.  Specify the --detail parameter to get more than ids." )

        ("detail", "In addition to getting object ids; also get the objects contents and any referenced contents.")

        ("new-zone-group", "Create a new zone group.  Requires the --name and --desc parameters. Optional --zone-list parameter.")
        ("update-zone-group", "Update a zone group. Requires --zgID parameter.")
        ("delete-zone-group", "Delete a zone group. Requires --zgID parameter.")
        ("add-zone-rules", "Add new zone rules to a zone group. Requires the --zgID parameter.")
        ("delete-zone-rule", "Delete a zone from a zone group. Requires the --zgID and --zrID parameters.")

        ("new-trigger-group", "Create a new trigger group.  Requires the --name and --desc parameters. Optional --day-and-time-list and --start-duration-interval parameters.")
        ("update-trigger-group", "Update a trigger group.  Requires the --tgID parameter.")
        ("delete-trigger-group", "Delete a trigger group. Requires the --tgID parameter.")
        ("add-trigger-rules", "Add trigger rules to a trigger group. Requires --tgID parameter.")
        ("delete-trigger-rule", "Delete a trigger from a trigger group. Requires the --tgID and --triggerID parameters.")

        ("new-schedule-event", "Create a schedule event. Requires the --name, --desc, --zgID, --tgID parameters.")
        ("update-schedule-event", "Update a schedule event. Requires the --seID parameter.")
        ("delete-schedule-event", "Delete a schedule event. Requires the --seID parameter.")
        ("enable-schedule-event", "Set a schedule event to enabled. Requires the --seID parameter.")
        ("disable-schedule-event", "Set a schedule event to disabled. Requires the --seID parameter.")

        ("desc", po::value<std::string>(&descStr), "The value for the description field.")
        ("name", po::value<std::string>(&nameStr), "The value for the name field.")
        ("zone-spec", po::value<std::string>(&zoneSpecStr), "Specify a comma seperated ordered list of zones specifiers. (i.e. zone1:5m,zone2:10m,zone3:1m)")
        ("time-spec",  po::value<std::string>(&timeSpecStr), "Specify a list of days and times. (i.e. Mon:6am,7pm;Tue:5pm;Fri:6am,7pm")

        ("seID", po::value<std::string>(&seID), "Specify the ID of a schedule event rule.")        
        ("zgID", po::value<std::string>(&zgID), "Specify the ID of a zone group.")        
        ("tgID", po::value<std::string>(&tgID), "Specify the ID of a trigger group.")
        ("zrID", po::value<std::string>(&zrID), "Specify the ID of a zone rule.")        
        ("trID", po::value<std::string>(&trID), "Specify the ID of a trigger rule.")        
        ("zoneID", po::value<std::string>(&zoneID), "Specify the ID of a zone.")        
    ;

    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );    

    if( vm.count( "help" ) ) 
    {
        std::cout << desc << "\n";
        return 1;
    }

    // In windows, this will init the winsock stuff 
    //curl_global_init(CURL_GLOBAL_ALL);

    if( vm.count( "zone-list" ) )
    {
        get_zone_list( (vm.count( "detail" ) ? true : false) );
    }
    else if( vm.count( "get-event-log" ) )
    {
        get_event_log();
    }
    else if( vm.count( "get-status" ) )
    {
        get_status();
    }
    else if( vm.count( "get-calendar" ) )
    {
        get_calendar( periodStr );
    }
    else if( vm.count( "zone-groups" ) )
    {
        get_zone_groups( vm.count( "detail" ) ? true : false );
    }
    else if( vm.count( "trigger-groups" ) )
    {
        get_trigger_groups( vm.count( "detail" ) ? true : false );
    }
    else if( vm.count( "schedule-events" ) )
    {
        get_schedule_events( vm.count( "detail" ) ? true : false );
    }
    else if( vm.count( "new-zone-group" ) )
    {
        std::string newZGID;

        if( !vm.count( "name" ) )
        {
            std::cerr << "ERROR: The --name parameter is required when creating a zone group." << std::endl;
            return( -1 );
        }

        if( !vm.count( "desc" ) )
        {
            std::cerr << "ERROR: The --desc parameter is required when creating a zone group." << std::endl;
            return( -1 );
        }

        create_zone_group( nameStr, descStr, zoneSpecStr, newZGID );

        std::cout << "Zone Group created successfully. ( ID: " << newZGID << " )" << std::endl; 
    }
    else if( vm.count( "update-zone-group" ) )
    {
        if( !vm.count( "zgID" ) )
        {
            std::cerr << "ERROR: The --zgID parameter is required when updating a zone group." << std::endl;
            return( -1 );
        }

        update_zone_group( zgID, nameStr, descStr );
    }
    else if( vm.count( "delete-zone-group" ) )
    {
        if( !vm.count( "zgID" ) )
        {
            std::cerr << "ERROR: The --zgID parameter is required when deleting a zone group." << std::endl;
            return( -1 );
        }

        delete_zone_group( zgID );
    }
    else if( vm.count( "add-zone-rules" ) )
    {
        if( !vm.count( "zgID" ) )
        {
            std::cerr << "ERROR: The --zgID parameter is required when deleting a zone group." << std::endl;
            return( -1 );
        }

        add_zone_rules( zgID, zoneSpecStr );
    }
    else if( vm.count( "delete-zone-rule" ) )
    {
        if( !vm.count( "zgID" ) )
        {
            std::cerr << "ERROR: The --zgID parameter is required when deleting a zone rule." << std::endl;
            return( -1 );
        }

        if( !vm.count( "zrID" ) )
        {
            std::cerr << "ERROR: The --zrID parameter is required when deleting a zone rule." << std::endl;
            return( -1 );
        }

        delete_zone_rule( zgID, zrID );
    }
    else if( vm.count( "new-trigger-group" ) )
    {
        std::string tgID;

        if( !vm.count( "name" ) )
        {
            std::cerr << "ERROR: The --name parameter is required when creating a trigger group." << std::endl;
            return( -1 );
        }

        if( !vm.count( "desc" ) )
        {
            std::cerr << "ERROR: The --desc parameter is required when creating a trigger group." << std::endl;
            return( -1 );
        }

        create_trigger_group( nameStr, descStr, tgID );

        std::cout << "Trigger Group created successfully. ( ID: " << tgID << " )" << std::endl; 

        if( vm.count( "time-spec" ) )
        {
            add_trigger_rules( tgID, timeSpecStr );
        }
    }
    else if( vm.count( "update-trigger-group" ) )
    {
        if( !vm.count( "tgID" ) )
        {
            std::cerr << "ERROR: The --tgID parameter is required when updating a trigger group." << std::endl;
            return( -1 );
        }

        update_trigger_group( tgID, nameStr, descStr );
    }
    else if( vm.count( "delete-trigger-group" ) )
    {
        if( !vm.count( "tgID" ) )
        {
            std::cerr << "ERROR: The --tgID parameter is required when deleting a trigger group." << std::endl;
            return( -1 );
        }

        delete_trigger_group( tgID );
    }
    else if( vm.count( "add-trigger-rules" ) )
    {
        if( !vm.count( "tgID" ) )
        {
            std::cerr << "ERROR: The --tgID parameter is required when deleting a trigger group." << std::endl;
            return( -1 );
        }

        add_trigger_rules( tgID, timeSpecStr );
    }
    else if( vm.count( "delete-trigger-rule" ) )
    {
        if( !vm.count( "tgID" ) )
        {
            std::cerr << "ERROR: The --tgID parameter is required when deleting a trigger rule." << std::endl;
            return( -1 );
        }

        if( !vm.count( "trID" ) )
        {
            std::cerr << "ERROR: The --trID parameter is required when deleting a trigger rule." << std::endl;
            return( -1 );
        }

        delete_trigger_rule( tgID, trID );
    }
    else if( vm.count( "new-schedule-event" ) )
    {
        std::string erID;

        if( !vm.count( "name" ) )
        {
            std::cerr << "ERROR: The --name parameter is required when creating a schedule entry." << std::endl;
            return( -1 );
        }

        if( !vm.count( "desc" ) )
        {
            std::cerr << "ERROR: The --desc parameter is required when creating a schedule entry." << std::endl;
            return( -1 );
        }

        if( !vm.count( "tgID" ) )
        {
            std::cerr << "ERROR: The --tgID parameter is required when creating a schedule entry." << std::endl;
            return( -1 );
        }

        if( !vm.count( "zgID" ) )
        {
            std::cerr << "ERROR: The --zgID parameter is required when creating a schedule entry." << std::endl;
            return( -1 );
        }

        create_schedule_rule( nameStr, descStr, zgID, tgID, erID);
    }
    else if( vm.count( "update-schedule-event" ) )
    {
        if( !vm.count( "seID" ) )
        {
            std::cerr << "ERROR: The --seID parameter is required when deleting a schedule entry." << std::endl;
            return( -1 );
        }

        update_schedule_rule( seID, nameStr, descStr, zgID, tgID );
    }
    else if( vm.count( "delete-schedule-event" ) )
    {
        if( !vm.count( "seID" ) )
        {
            std::cerr << "ERROR: The --seID parameter is required when deleting a schedule entry." << std::endl;
            return( -1 );
        }

        delete_schedule_rule( seID );
    }
    else if( vm.count( "enable-schedule-event" ) )
    {
        if( !vm.count( "seID" ) )
        {
            std::cerr << "ERROR: The --seID parameter is required when deleting a schedule entry." << std::endl;
            return( -1 );
        }

        enable_schedule_rule( seID );
    }
    else if( vm.count( "disable-schedule-event" ) )
    {
        if( !vm.count( "seID" ) )
        {
            std::cerr << "ERROR: The --seID parameter is required when deleting a schedule entry." << std::endl;
            return( -1 );
        }

        disable_schedule_rule( seID );
    }


#if 0
    else if( vm.count( "ids" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;
        sscReadBuf rspData;

        if( vm.count( "event-rule" ) )
        {
            url = "http://localhost:8200/schedule/rules";
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://localhost:8200/schedule/zone-groups";
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://localhost:8200/schedule/trigger-groups";
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/zone-groups/" + objID + "/members";
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/trigger-groups/" + objID + "/members";
        }
 
        // get a curl handle 
        curl = curl_easy_init();

        if( curl ) 
        {
            struct curl_slist *slist = NULL;
	  
	        slist = curl_slist_append(slist, "Accept: */*");
	        // slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	        slist = curl_slist_append(slist, "Content-Type: text/xml");

            // Setup the post operation parameters				
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	        
            // Setup the read callback 
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *) &rspData );

            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "new" ) )
    {
        CURL *curl;
        CURLcode res;
 
        std::string post;
        std::string url;
 
        if( vm.count( "event-rule" ) )
        {
            post = "<schedule-event-rule><desc>Monday Morning</desc></schedule-event-rule>";
            url = "http://localhost:8200/schedule/rules";
        }
        else if( vm.count( "zone-group" ) )
        {
            post = "<schedule-zone-group><name>ZG Monday</name><policy>sequential</policy></schedule-zone-group>";
            url = "http://localhost:8200/schedule/zone-groups";
        }
        else if( vm.count( "trigger-group" ) )
        {
            post = "<schedule-trigger-group><name>TG Monday</name><type>time</type></schedule-trigger-group>";
            url = "http://localhost:8200/schedule/trigger-groups";
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            post = "<schedule-zone-rule><name>Zone Rule</name><type>fixedduration</type><duration>600</duration></schedule-zone-rule>";
            url = "http://localhost:8200/schedule/zone-groups/" + objID + "/members";
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            post = "<schedule-trigger-rule><name>Trigger Rule</name><type>time</type><scope>day</scope><reftime>20140507T173400</reftime></schedule-trigger-rule>";
            url = "http://localhost:8200/schedule/trigger-groups/" + objID + "/members";
        }

        // get a curl handle 
        curl = curl_easy_init();

        if( curl ) 
        {
            struct curl_slist *slist = NULL;
	  
	        slist = curl_slist_append(slist, "Accept: */*");
	        // slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	        slist = curl_slist_append(slist, "Content-Type: text/xml");

            // Setup the post operation parameters				
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "object" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;
        sscReadBuf rspData;

        // erID must have been specified
        if( !vm.count( "objid" ) )
        {
            fprintf( stderr, "The objid parameter is required\n" );
            return -1;
        }

        if( vm.count( "event-rule" ) )
        {
            url = "http://localhost:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://localhost:8200/schedule/zone-groups/" + objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://localhost:8200/schedule/trigger-groups/" + objID;
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
        }

        // get a curl handle 
        curl = curl_easy_init();

        if( curl ) 
        {
            struct curl_slist *slist = NULL;
	  
	        slist = curl_slist_append(slist, "Accept: */*");
	        // slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	        slist = curl_slist_append(slist, "Content-Type: text/xml");

            // Setup the post operation parameters				
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	        
            // Setup the read callback 
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *) &rspData );

            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "update" ) )
    {
        CURL *curl;
        CURLcode res;
 
        std::string putData;
        std::string url;
 
        // erID must have been specified
        if( !vm.count( "objid" ) )
        {
            fprintf( stderr, "The objid parameter is required\n" );
            return -1;
        }

        if( vm.count( "event-rule" ) )
        {
            putData = "<schedule-event-rule><desc>Monday Morning</desc></schedule-event-rule>";
            url = "http://localhost:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            putData = "<schedule-zone-group><name>Update Name</name></schedule-zone-group>";
            url = "http://localhost:8200/schedule/zone-groups/" +objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            putData = "<schedule-trigger-group><name>Update Name</name></schedule-trigger-group>";
            url = "http://localhost:8200/schedule/trigger-groups/" + objID;
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            putData = "<schedule-zone-rule><name>Update Rule</name><duration>300</duration></schedule-zone-rule>";
            url = "http://localhost:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            putData = "<schedule-trigger-rule><name>Update Rule</name></schedule-trigger-rule>";
            url = "http://localhost:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
        }

        // get a curl handle 
        curl = curl_easy_init();

        if( curl ) 
        {
            struct curl_slist *slist = NULL;
	  
	        slist = curl_slist_append(slist, "Accept: */*");
	        // slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	        slist = curl_slist_append(slist, "Content-Type: text/xml");

            // Setup the put operation parameters				
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_PUT, 1L);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
            curl_easy_setopt(curl, CURLOPT_READDATA, &putData);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) putData.size() );
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "delete" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;

        // erID must have been specified
        if( !vm.count( "objid" ) )
        {
            fprintf( stderr, "The objid parameter is required\n" );
            return -1;
        }

        if( vm.count( "event-rule" ) )
        {
            url = "http://localhost:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://localhost:8200/schedule/zone-groups/" + objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://localhost:8200/schedule/trigger-groups/" + objID;
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://localhost:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
        }

        // get a curl handle 
        curl = curl_easy_init();

        if( curl ) 
        {
            struct curl_slist *slist = NULL;
	  
	        slist = curl_slist_append(slist, "Accept: */*");
	        // slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	        slist = curl_slist_append(slist, "Content-Type: text/xml");

            // Setup the put operation parameters				
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }
#endif
 

    // Success
    return 0;
}


