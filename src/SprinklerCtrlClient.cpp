#include <stdio.h>
#include <curl/curl.h>

#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

static size_t
WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    size_t realsize = size * nmemb;

    printf( "Content: %*.*s\n", realsize, realsize, contents );

#if 0
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;
 
    mem->memory = realloc( mem->memory, mem->size + realsize + 1 );
    if( mem->memory == NULL ) 
    {
        // out of memory! 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
 
    memcpy( &(mem->memory[mem->size]), contents, realsize );
    mem->size += realsize;
    mem->memory[ mem->size ] = 0;
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

int main( int argc, char* argv[] )
{
    std::string objID;
    std::string obj2ID;

    // Declare the supported options.
    po::options_description desc("HNode Sprinkler Control Client");
    desc.add_options()
        ("help", "produce help message")
        ("ids", "Retrieve a list of object ids")
        ("new", "Create a new object")
        ("object", "Get the fields for an object")
        ("update", "Update the fields in an object")
        ("delete", "Delete an existing object")

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

    if( vm.count( "ids" ) )
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


