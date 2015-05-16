#include <stdio.h>
#include <curl/curl.h>

#include <iostream>
#include <vector>
#include <boost/program_options.hpp>

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

    printf( "Content: %*.*s\n", realsize, realsize, contents );

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
    
    url = "http://192.168.1.128:8200/zones";
 
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

    url = "http://192.168.1.128:8200/zones/" + zoneID;
 
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

static size_t 
location_header_callback( char *buffer, size_t size, size_t nitems, void *userdata )
{
  size_t dataSize = nitems * size;

  if( dataSize != 0 )
  {
      printf( "Return Header(%d) - %*.*s\n", dataSize, dataSize, dataSize, buffer );
  }

  return dataSize;
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

    url = "http://192.168.1.128:8200/schedule/zone-groups";

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
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

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
create_trigger_group( std::string name, std::string desc, std::string times, std::string &tgID )
{

}

bool
create_schedule_rule( std::string name, std::string desc, std::string zgID, std::string tgID, std::string &erID )
{

}

int main( int argc, char* argv[] )
{
    std::string objID;
    std::string obj2ID;

    std::string descStr;
    std::string nameStr;
    std::string zoneListStr;
    std::string timeListStr;

    // Declare the supported options.
    po::options_description desc("HNode Sprinkler Control Client");
    desc.add_options()
        ("help", "produce help message")
        ("ids", "Retrieve a list of object ids")
        ("new", "Create a new object")
        ("object", "Get the fields for an object")
        ("update", "Update the fields in an object")
        ("delete", "Delete an existing object")

        ("get-zone-list", "Get a list of available zones.")

        ("new-schedule-entry", "Create a complete schedule entry, including the schedule-rule, zone-group, and trigger group.")

        ("desc", po::value<std::string>(&descStr), "The value for the description field.")
        ("name", po::value<std::string>(&nameStr), "The value for the name field.")

        ("zone-list", po::value<std::string>(&zoneListStr), "Specify a comma seperated ordered list of zones by zoneID.")
        ("time-list",  po::value<std::string>(&timeListStr), "Specify a comma seperated list of trigger times.")

        ("detail", "In addition to getting object ids; also get the objects contents.")
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
    else if( vm.count( "new-schedule-entry" ) )
    {
        std::string zgID, tgID, erID;

        create_zone_group( "zgname", "zgdesc", "zone1, zone2", zgID );
        //create_trigger_group( "tgname", "tgdesc", "", tgID );
        //create_schedule_rule( "name", "desc", zgID, tgID, erID);
    }
    else if( vm.count( "ids" ) )
    {
        CURL *curl;
        CURLcode res;
        std::string url;

        if( vm.count( "event-rule" ) )
        {
            url = "http://192.168.1.128:8200/schedule/rules";
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/zone-groups";
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/trigger-groups";
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID + "/members";
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid" ) )
            {
                fprintf( stderr, "The objid parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID + "/members";
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
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)NULL );

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
            url = "http://192.168.1.128:8200/schedule/rules";
        }
        else if( vm.count( "zone-group" ) )
        {
            post = "<schedule-zone-group><name>ZG Monday</name><policy>sequential</policy></schedule-zone-group>";
            url = "http://192.168.1.128:8200/schedule/zone-groups";
        }
        else if( vm.count( "trigger-group" ) )
        {
            post = "<schedule-trigger-group><name>TG Monday</name><type>time</type></schedule-trigger-group>";
            url = "http://192.168.1.128:8200/schedule/trigger-groups";
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
            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID + "/members";
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
            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID + "/members";
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

        // erID must have been specified
        if( !vm.count( "objid" ) )
        {
            fprintf( stderr, "The objid parameter is required\n" );
            return -1;
        }

        if( vm.count( "event-rule" ) )
        {
            url = "http://192.168.1.128:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID;
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
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
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)NULL );

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
            url = "http://192.168.1.128:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            putData = "<schedule-zone-group><name>Update Name</name></schedule-zone-group>";
            url = "http://192.168.1.128:8200/schedule/zone-groups/" +objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            putData = "<schedule-trigger-group><name>Update Name</name></schedule-trigger-group>";
            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID;
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
            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
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
            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
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
            url = "http://192.168.1.128:8200/schedule/rules/" + objID;
        }
        else if( vm.count( "zone-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID;
        }
        else if( vm.count( "trigger-group" ) )
        {
            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID;
        }
        else if( vm.count( "zone-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/zone-groups/" + objID + "/members/" + obj2ID;
        }
        else if( vm.count( "trigger-rule" ) )
        {
            // objID must have been specified
            if( !vm.count( "objid2" ) )
            {
                fprintf( stderr, "The objid2 parameter is required\n" );
                return -1;
            }

            url = "http://192.168.1.128:8200/schedule/trigger-groups/" + objID + "/members/" + obj2ID;
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
    
    // Success
    return 0;
}


