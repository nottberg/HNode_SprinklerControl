#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "REST.hpp"

#define PORT 8888
#define POSTBUFFERSIZE 512
#define MAXNAMESIZE 20
#define MAXANSWERSIZE 512
#define GET 0
#define POST 1

#if 0
struct connection_info_struct
{
    int connectiontype;
    char *answerstring;
    struct MHD_PostProcessor *postprocessor;
};

const char *askpage = "<html><body>\
What’s your name, Sir?<br>\
<form action=\"/namepost\" method=\"post\">\
<input name=\"name\" type=\"text\"\
<input type=\"submit\" value=\" Send \"></form>\
</body></html>";

const char *greetingpage =
"<html><body><h1>Welcome, %s!</center></h1></body></html>";

const char *errorpage =
"<html><body>This doesn’t seem to be right.</body></html>";
#endif

#if 0
static int
iterate_post( void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size )
{
    struct connection_info_struct *con_info = coninfo_cls;
    if( 0 == strcmp( key, "name" ) )
    {
        if( (size > 0) && (size <= MAXNAMESIZE) )
        {
            char *answerstring;
            answerstring = malloc( MAXANSWERSIZE );
            if( !answerstring )
                return MHD_NO;
            snprintf( answerstring, MAXANSWERSIZE, greetingpage, data );
            con_info->answerstring = answerstring;
        }   
        else
            con_info->answerstring = NULL;
        return MHD_NO;
    }
    return MHD_YES;
}
#endif

RESTDaemon::RESTDaemon()
{

}

RESTDaemon::~RESTDaemon()
{

}

int 
RESTDaemon::start()
{
    daemon = MHD_start_daemon( MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &connection_request, this, MHD_OPTION_NOTIFY_COMPLETED, request_completed, this, MHD_OPTION_END );

    if( NULL == daemon )
        return 1;

    return 0;
}

void 
RESTDaemon::stop()
{
    MHD_stop_daemon( daemon );
}

int
RESTDaemon::newRequest( struct MHD_Connection *connection,
                        std::string url, std::string method,
                        std::string version, const char *upload_data,
                        size_t *upload_data_size, void **con_cls )
{
    RESTRequest *request;

    // Check if this is a new request.
    if( NULL == *con_cls )
    {
        // Create a new request
        request = new RESTRequest();

        // Fill the request parameters
        request->setURL( url );
        request->decodeRequestMethod( method );
        request->setVersion( version );

        // Scan through the registered resources and see if 
        // a match exists.
        for( std::list<RESTResource *>::iterator it=resourceList.begin(); it != resourceList.end(); ++it )
        {
            if( (*it)->isURLMatch( request ) )
            {
                (*it)->processRequest( request );
            }
        }

        // No match for the resource so return a bad request indication

#if 0
        if( 0 == strcmp( method, "POST" ) )
        {
            con_info->postprocessor = MHD_create_post_processor( connection, POSTBUFFERSIZE, iterate_post, (void *) con_info );
            if( NULL == con_info->postprocessor )
            {
                free( con_info );
                return MHD_NO;
            }
            con_info->connectiontype = POST;
        }
        else
            con_info->connectiontype = GET;
            *con_cls = (void *) con_info;
            return MHD_YES;
    }
#endif
    } 

    // This is a continuing request for a POST/PUT upload.
    request = (RESTRequest *) *con_cls;

    // Process the continueing operation
    

#if 0
    if( 0 == strcmp( method, "GET" ) )
    {
        return send_page (connection, askpage);
    }

    if( 0 == strcmp( method, "POST" ) )
    {
        struct connection_info_struct *con_info = *con_cls;
        if( *upload_data_size != 0 )
        {
            MHD_post_process( con_info->postprocessor, upload_data, *upload_data_size );
            *upload_data_size = 0;
            return MHD_YES;
        }
        else if( NULL != con_info->answerstring )
            return send_page( connection, con_info->answerstring );
    }
    return send_page( connection, errorpage );
#endif
}

void
RESTDaemon::requestComplete( struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe )
{
#if 0
    struct connection_info_struct *con_info = *con_cls;

    if( NULL == con_info )
        return;

    if( con_info->connectiontype == POST )
    {
        MHD_destroy_post_processor( con_info->postprocessor );
        if( con_info->answerstring )
            free( con_info->answerstring );
    }

    free( con_info );
    *con_cls = NULL;
#endif
}

bool 
RESTDaemon::registerResource( RESTResource *resource )
{
    resourceList.push_back( resource );
}

// Static callback to go from flat C to the daemon object
int
RESTDaemon::connection_request( void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls )
{
    RESTDaemon *daemon = (RESTDaemon *)cls;

    return daemon->newRequest( connection, url, method, version, upload_data, upload_data_size, con_cls );
}

void
RESTDaemon::request_completed( void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe )
{
    RESTDaemon *daemon = (RESTDaemon *)cls;

    daemon->requestComplete( connection, con_cls, toe );
}


