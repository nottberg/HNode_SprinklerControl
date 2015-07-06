#include <stdio.h>
#include <curl/curl.h>

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

namespace po = boost::program_options;

class sscReadBuf
{
    private:
        unsigned char *data;
        unsigned long size;

    public:
        sscReadBuf();
       ~sscReadBuf();

        void appendData( void *data, unsigned long length );

        unsigned char *getDataPtr() { return data; }
        unsigned long getLength() { return size; }
};

sscReadBuf::sscReadBuf()
{
    data = NULL;
    size = 0;
}

sscReadBuf::~sscReadBuf()
{
    if( data != NULL )
        free( data );
}

void 
sscReadBuf::appendData( void *newdata, unsigned long length )
{
    data = (unsigned char *) realloc( data, size + length + 1 );
    if( data == NULL ) 
    {
        // out of memory! 
        printf("not enough memory (realloc returned NULL)\n");
        return;
    }
 
    memcpy( &(data[size]), newdata, length );
    size += length;
    data[ size ] = 0;
}

static size_t
WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    size_t realsize = size * nmemb;

    printf( "Content: %*.*s\n", (int)realsize, (int)realsize, (char*)contents );

    // FIXME: Broken, because data can come in chunks 
    ((sscReadBuf *) userp )->appendData( contents, realsize );

#if 0
 #endif

    return realsize;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stdstr)
{
  std::string *data = (std::string *)stdstr;

  size_t retcode;
 
  retcode = size * nmemb;

  memcpy( ptr, data->c_str(), retcode );
 
  return retcode;
}

bool
get_zone_list( std::vector< std::string > &idList )
{
    CURL *curl;
    CURLcode res;
    sscReadBuf rspData;
    std::string url;
    
    url = "http://localhost:8200/zones";
 
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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	        
        // Setup the read callback 
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&rspData );

        // Perform the request, res will get the return code 
        res = curl_easy_perform(curl);

        // Check for errors 
        if( res != CURLE_OK )
        {
            fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
            return true;
        }

        // always cleanup 
        curl_easy_cleanup( curl );
        curl_slist_free_all( slist );
    }

    xmlDocPtr doc; 

    // The document being in memory, it have no base per RFC 2396,
    // and the "noname.xml" argument will serve as its base.
    doc = xmlReadMemory( (const char *)rspData.getDataPtr(), rspData.getLength(), "noname.xml", NULL, 0 );
    if( doc == NULL ) 
    {
        fprintf(stderr, "Failed to parse document\n");
	    return true;
    }

    xmlNodePtr rootNode;

    rootNode = xmlDocGetRootElement( doc );

    if( ( !rootNode ) || ( strcmp( (const char *)rootNode->name, "hnode-zonelist" ) != 0 ) )
    {
        fprintf(stderr, "Unrecognized return data.\n");
	    return true;
    }
 
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

    xmlFreeDoc(doc);

    return false;
}

bool
get_zone_object( std::string zoneID, std::map< std::string, std::string > &objFields )
{
    CURL *curl;
    CURLcode res;
    sscReadBuf rspData;
    std::string url;

    if( zoneID.empty() )
        return true;    

    url = "http://localhost:8200/zones/" + zoneID;
 
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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	        
        // Setup the read callback 
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&rspData );

        // Perform the request, res will get the return code 
        res = curl_easy_perform(curl);

        // Check for errors 
        if( res != CURLE_OK )
        {
            fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
            return true;
        }

        // always cleanup 
        curl_easy_cleanup( curl );
        curl_slist_free_all( slist );
    }

    xmlDocPtr doc; 

    // The document being in memory, it have no base per RFC 2396,
    // and the "noname.xml" argument will serve as its base.
    doc = xmlReadMemory( (const char *)rspData.getDataPtr(), rspData.getLength(), "noname.xml", NULL, 0 );
    if( doc == NULL ) 
    {
        fprintf(stderr, "Failed to parse document\n");
	    return true;
    }

    xmlNodePtr rootNode;

    rootNode = xmlDocGetRootElement( doc );

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

bool hasEnding( std::string const &fullString, std::string const &ending ) 
{
    if( fullString.length() >= ending.length() ) 
    {
        return( 0 == fullString.compare( fullString.length() - ending.length(), ending.length(), ending ) );
    } 

    return false;
}

static size_t 
location_header_callback( char *buffer, size_t size, size_t nitems, void *userdata )
{
  size_t dataSize = nitems * size;
  std::string *strPtr = (std::string *) userdata;

  if( dataSize != 0 )
  {
      std::string dataStr( buffer, dataSize );

      if( dataStr.find( "Location:" ) != std::string::npos )
      {
          printf( "Return Header(%d) - %*.*s\n", (int)dataSize, (int)dataSize, (int)dataSize, buffer );

          std::string locData( buffer, dataSize );

          boost::regex re("/+");
          boost::sregex_token_iterator i( locData.begin(), locData.end(), re, -1);
          boost::sregex_token_iterator j;

          while( i != j )
          {
              std::string tmpStr = *i;
              boost::trim( tmpStr );

              std::cout << "Location ID: " << tmpStr << std::endl;
              *strPtr = tmpStr;
              i++;
          }
      }
  }

  return dataSize;
}

bool 
create_zone_rule( std::string parentID, std::string zoneID, int duration, std::string &zrID )
{
    CURL *curl;
    CURLcode res;
 
    std::ostringstream postStream;
    std::string post;

    std::string url;
 
    postStream << "<schedule-zone-rule>";
    postStream << "<zoneid>" << zoneID << "</zoneid>";
    postStream << "<type>" << "fixedduration" << "</type>";
    postStream << "<duration>" << duration << "</duration>";
    postStream << "</schedule-zone-rule>";

    post = postStream.str();

    std::cout << "create_zone_rule post: " << post << std::endl;

    url = "http://localhost:8200/schedule/zone-groups/" + parentID + "/members";

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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &zrID);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, location_header_callback);
	        
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



bool 
create_from_zone_list( std::string parentID, std::string name, std::string zoneList )
{
    std::string zrID;
    boost::regex re(",+");
    boost::sregex_token_iterator i( zoneList.begin(), zoneList.end(), re, -1);
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

        create_zone_rule( parentID, zoneID, duration, zrID );

        std::cout << "Created Zone Rule: " << zrID << std::endl;

        i++;
    }

}

bool 
create_zone_group( std::string name, std::string desc, std::string zones, std::string &zgID )
{
    CURL *curl;
    CURLcode res;
 
    std::string post;
    std::string url;
 
    post  = "<schedule-zone-group>";
    post += "<name>" + name + "</name>";
    post += "<desc>" + desc + "</desc>";
    post += "<policy>sequential</policy>";
    post += "</schedule-zone-group>";

    url = "http://localhost:8200/schedule/zone-groups";

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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &zgID);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, location_header_callback);
	        
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

        // If there is also a zone list then continue processing
        if( zones.size() )
        {
            create_from_zone_list( zgID, "zonerule", zones );
        }
    }


    curl_global_cleanup();
}


bool
create_trigger_group( std::string name, std::string desc, std::string &tgID )
{
    CURL *curl;
    CURLcode res;
 
    std::string post;
    std::string url;
 
    post  = "<schedule-trigger-group>";
    post += "<name>" + name + "</name>";
    post += "<desc>" + desc + "</desc>";
    post += "</schedule-trigger-group>";

    url = "http://localhost:8200/schedule/trigger-groups";

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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &tgID);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, location_header_callback);
	        
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

/*
        // If there is also a zone list then continue processing
        if( zones.size() )
        {
            create_from_zone_list( zgID, "zonerule", zones );
        }
*/
    }


    curl_global_cleanup();
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

bool 
create_trigger_rule( std::string parentID, TriggerRuleSpec &trSpec, std::string &trID )
{
    CURL *curl;
    CURLcode res;
 
    std::ostringstream postStream;
    std::string post;
    std::string url;
 
    postStream << "<schedule-trigger-rule>";
    postStream << "<type>" << "time" << "</type>";
    postStream << "<scope>" << trSpec.getScopeAsStr() << "</scope>";
    postStream << "<reftime>" << trSpec.getReftimeAsISOStr() << "</reftime>";
    postStream << "</schedule-trigger-rule>";

    post = postStream.str();

    std::cout << "create_trigger_rule post: " << post << std::endl;

    url = "http://localhost:8200/schedule/trigger-groups/" + parentID + "/members";

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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &trID);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, location_header_callback);
	        
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
create_schedule_rule( std::string name, std::string desc, std::string zgID, std::string tgID, std::string &erID )
{
    CURL *curl;
    CURLcode res;
 
    std::ostringstream postStream;
    std::string post;
    std::string url;

    postStream << "<schedule-event-rule>";
    postStream << "<name>" << name << "</name>";
    postStream << "<desc>" << desc << "</desc>";
    postStream << "<zone-group-id>" << zgID << "</zone-group-id>";
    postStream << "<trigger-group-id>" << tgID << "</trigger-group-id>";
    postStream << "<enabled>" << "true" << "</enabled>";
    postStream << "</schedule-event-rule>";

    post = postStream.str();

    std::cout << "create_schedule_rule post: " << post << std::endl;

    url = "http://localhost:8200/schedule/rules";

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
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size()); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &erID);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, location_header_callback);
	        
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

size_t
schedule_event_log_reader( void *buffer, size_t size, size_t nmemb, void *userp )
{
    xmlParserCtxtPtr *pctxt = (xmlParserCtxtPtr *) userp;
    xmlParserCtxtPtr ctxt   = *pctxt;
    size_t length = size * nmemb;

    if( ctxt == NULL )
    {
        ctxt = xmlCreatePushParserCtxt( NULL, NULL, (const char *) buffer, length, "dummy.xml" ); 
      
        *pctxt = ctxt;
    }
    else
    {
        xmlParseChunk( ctxt, (const char *) buffer, length, 0 );
    }

    return length;
}

int main( int argc, char* argv[] )
{
    std::string objID;
    std::string obj2ID;

    std::string descStr;
    std::string nameStr;

    std::string zgID;
    std::string tgID;
    std::string zoneID;
    std::string triggerID;

    std::string zoneListStr;

    std::string wksListStr;
    std::string sdiListStr;

    std::string periodStr;

    // Declare the supported options.
    po::options_description desc("HNode Sprinkler Control Client");
    desc.add_options()
        ("help", "produce help message")
        ("ids", "Retrieve a list of object ids")
        ("new", "Create a new object")
        ("object", "Get the fields for an object")
        ("update", "Update the fields in an object")
        ("delete", "Delete an existing object")

        ("desc", po::value<std::string>(&descStr), "The value for the description field.")
        ("name", po::value<std::string>(&nameStr), "The value for the name field.")

        ("get-zone-list", "Get a list of available zones.")

        ("new-zone-group", "Create a new zone group.  Requires the --name and --desc parameters. Optional --zone-list parameter.")
        ("add-zone", "Add a new zone to a zone group. Requires the --zgID parameter.")
        ("delete-zone", "Delete a zone from a zone group. Requires the --zgID and --zoneID parameters.")

        ("new-trigger-group", "Create a new trigger group.  Requires the --name and --desc parameters. Optional --day-and-time-list and --start-duration-interval parameters.")
        ("add-trigger", "Add a new trigger to a trigger group. Requires the --tgID parameter.")
        ("delete-trigger", "Delete a trigger from a trigger group. Requires the --tgID and --triggerID parameters.")

        ("new-schedule-entry", "Create a schedule entry. Requires the --name, --desc, --zgID, --tgID parameters.")

        ("zgID", po::value<std::string>(&zgID), "Specify the ID of a zone group to use with the command.")        
        ("tgID", po::value<std::string>(&tgID), "Specify the ID of a trigger group to use with the command.")        
        ("zoneID", po::value<std::string>(&zoneID), "Specify the ID of a zone to use with the command.")        
        ("triggerID", po::value<std::string>(&triggerID), "Specify the ID of a trigger to use with the command.")        

        ("zone-list", po::value<std::string>(&zoneListStr), "Specify a comma seperated ordered list of zones specifiers. (i.e. zone1:5m,zone2:10m,zone3:1m)")

        ("get-event-log", "Get the event log.")
        ("get-status", "Get current schedule status.")
        ("get-calendar", "Get the potential events for a period of time.")

        // scope, start-scope, time-list, interval, repeat; next-entry
        // scope is hourly, daily, weekly, monthly
        ("time-list",  po::value<std::string>(&wksListStr), "Specify a list of days and times. (i.e. Mon:6am,7pm;Tue:5pm;Fri:6am,7pm")

        ("detail", "In addition to getting object ids; also get the objects contents.")

        ("period",  po::value<std::string>(&periodStr), "Specify a calendar period. (today, week, twoweek, month)")

#if 0
        ("get-schedule-entries", 
        ("new-schedule-entry", "Create a new schedule entry.")
        ("add-time-trigger",
        ("remove-time-trigger",
        ("add-zone",
        ("remove-zone",

        ("zone-list","Specify a comma seperated list of zones by zoneID.")
        ("time",
#endif
        ("event-rule", "Operate on event rules.")
        ("zone-group", "Operate on zone groups.")
        ("trigger-group", "Operate on trigger groups")
        ("zone-rule", "Operate on a zone rule")
        ("trigger-rule", "Operate on a trigger rule")

        ("cmd", "dummy cmd")
        ("objid", po::value<std::string>(&objID), "The object id.")
        ("objid2", po::value<std::string>(&obj2ID), "The object id 2.")


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
    curl_global_init(CURL_GLOBAL_ALL);

    if( vm.count( "get-zone-list" ) )
    {
        std::vector< std::string > idList;

        if( get_zone_list( idList ) == true )
        {
            std::cerr << "ERROR: Unable to retrieve zone id list." << std::endl;
            return 2;
        }

        // Output the zone IDs
        std::cout << "=== Zone IDs (Count: " << idList.size() << " ) ===" << std::endl;
        for( std::vector< std::string >::iterator it = idList.begin(); it != idList.end(); it++ )
        {
            std::map< std::string, std::string > objFields;

            std::cout << "Zone ID: " << *it << std::endl;
        
            if( vm.count( "detail" ) )
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
    else if( vm.count( "new-zone-group" ) )
    {
        std::string zgID;

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

        create_zone_group( nameStr, descStr, zoneListStr, zgID );

        std::cout << "Zone Group created successfully. ( ID: " << zgID << " )" << std::endl; 
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

        if( vm.count( "time-list" ) )
        {
            std::vector< TriggerRuleSpec > trList;
            std::string trID;

            process_time_set_list( wksListStr, trList );

            for( std::vector< TriggerRuleSpec >::iterator it = trList.begin(); it != trList.end(); ++it )
            {
                it->debugPrint();
                create_trigger_rule( tgID, *it, trID );
            }    
        }
    }
    else if( vm.count( "new-schedule-entry" ) )
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
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
    
    if( vm.count( "get-event-log" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;

        xmlDocPtr doc;
        xmlParserCtxtPtr ctxt = NULL;

        url = "http://localhost:8200/schedule/event-log/";

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
	        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, schedule_event_log_reader );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, &ctxt );
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // Finished
            xmlParseChunk( ctxt, NULL, 0, 1 );

            doc = ctxt->myDoc;
            if( ctxt->wellFormed )
            {
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
                else
                {
                    fprintf( stderr, "schedule event log format error\n" );
                    return -1;
                }

            }
            else
            {
                fprintf( stderr, "Failed to parse return data.\n" );
            }

            xmlFreeParserCtxt( ctxt );
            xmlFreeDoc( doc );

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "get-status" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;

        xmlDocPtr doc;
        xmlParserCtxtPtr ctxt = NULL;

        url = "http://localhost:8200/schedule/status/";

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
	        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, schedule_event_log_reader );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, &ctxt );
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // Finished
            xmlParseChunk( ctxt, NULL, 0, 1 );

            doc = ctxt->myDoc;
            if( ctxt->wellFormed )
            {
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
                else
                {
                    fprintf( stderr, "schedule status format error\n" );
                    return -1;
                }
            }
            else
            {
                fprintf( stderr, "Failed to parse return data.\n" );
            }

            xmlFreeParserCtxt( ctxt );
            xmlFreeDoc( doc );

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    if( vm.count( "get-calendar" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;

        xmlDocPtr doc;
        xmlParserCtxtPtr ctxt = NULL;

        url = "http://localhost:8200/schedule/calendar/";

        if( vm.count( "period" ) == 0 )
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

            url += "?startTime=";
            url += to_iso_string( start.utc_time() );
            url += "&endTime=";
            url += to_iso_string( end.utc_time() );
        } 
        else if( periodStr == "week" )
        {
            boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );
            unsigned long dayofWeek = ltstamp.date().day_of_week();
            ltstamp -= boost::gregorian::days( dayofWeek );

            boost::local_time::local_date_time start( ltstamp.date(), std, zone, false );  
            boost::local_time::local_date_time end( (ltstamp.date() + boost::gregorian::days(6)), etd, zone, false );  

            url += "?startTime=";
            url += to_iso_string( start.utc_time() );
            url += "&endTime=";
            url += to_iso_string( end.utc_time() );
        }
        else if( periodStr == "twoweek" )
        {
            boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );
            unsigned long dayofWeek = ltstamp.date().day_of_week();
            ltstamp -= boost::gregorian::days( dayofWeek );

            boost::local_time::local_date_time start( ltstamp.date(), std, zone, false );  
            boost::local_time::local_date_time end( (ltstamp.date() + boost::gregorian::days(13)), etd, zone, false );  

            url += "?startTime=";
            url += to_iso_string( start.utc_time() );
            url += "&endTime=";
            url += to_iso_string( end.utc_time() );
        }
        else if( periodStr == "month" )
        {
            boost::local_time::local_date_time ltstamp = boost::local_time::local_sec_clock::local_time( zone );

            unsigned long dayofMonth = ltstamp.date().day();
            unsigned long endofMonth = ltstamp.date().end_of_month().day();

            boost::local_time::local_date_time start( (ltstamp.date() - boost::gregorian::days(dayofMonth-1)), std, zone, false );  
            boost::local_time::local_date_time end( (start.date() + boost::gregorian::days(endofMonth-1)), etd, zone, false );  

            url += "?startTime=";
            url += to_iso_string( start.utc_time() );
            url += "&endTime=";
            url += to_iso_string( end.utc_time() );
        }
        else
        {
            std::cerr << "ERROR: Requested period is not supportted." << std::endl;
            return -1;
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
	        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT,  "Linux C  libcurl");
	        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, schedule_event_log_reader );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, &ctxt );
	        
            // Perform the request, res will get the return code 
            res = curl_easy_perform(curl);

            // Check for errors 
            if( res != CURLE_OK )
            {
                fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
                return -1;
            }

            // Finished
            xmlParseChunk( ctxt, NULL, 0, 1 );

            doc = ctxt->myDoc;
            if( ctxt->wellFormed )
            {
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
            }
            else
            {
                fprintf( stderr, "Failed to parse return data.\n" );
            }

            xmlFreeParserCtxt( ctxt );
            xmlFreeDoc( doc );

            // always cleanup 
            curl_easy_cleanup( curl );
            curl_slist_free_all( slist );
        }

        curl_global_cleanup();
    }

    // Success
    return 0;
}


