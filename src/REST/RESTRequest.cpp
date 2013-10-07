#include <string.h>
#include <stdio.h>

#include "REST.hpp"

#define POSTBUFFERSIZE 1024

RESTRequest::RESTRequest(struct MHD_Connection *newConnection )
{
    //int connectiontype;
    //char *answerstring;
    //struct MHD_PostProcessor *postprocessor;
    connection = newConnection;

    reqMethod = REST_RMETHOD_NONE;

    rspCode = REST_HTTP_RCODE_NONE;

    resourcePtr = NULL;
}

RESTRequest::~RESTRequest()
{
    if( postProcessor )
        MHD_destroy_post_processor( postProcessor );
}

void 
RESTRequest::setLink( void *resource )
{
    resourcePtr = resource;
}

void 
RESTRequest::setURL( std::string url )
{
    urlStr = url;
}

std::string 
RESTRequest::getURL()
{
    return urlStr;
}

struct MHD_Connection *
RESTRequest::getConnection()
{
    return connection;
}

RESTRepresentation *
RESTRequest::getInboundRepresentation()
{
    return &inRepresentation;
}

RESTRepresentation *
RESTRequest::getOutboudRepresentation()
{
    return &outRepresentation;
}

void 
RESTRequest::decodeRequestMethod( std::string method )
{
    reqMethod = REST_RMETHOD_NONE;

    if( "GET" == method )
        reqMethod = REST_RMETHOD_GET;
    else if( "PUT" == method )
        reqMethod = REST_RMETHOD_PUT;
    else if( "POST" == method )
        reqMethod = REST_RMETHOD_POST;
    else if( "DELETE" == method )
        reqMethod = REST_RMETHOD_DELETE;

}

void 
RESTRequest::setVersion( std::string version )
{

}

void 
RESTRequest::setResponseCode( REST_HTTP_RCODE_T code )
{
    rspCode = code;
}

REST_HTTP_RCODE_T
RESTRequest::getResponseCode()
{
    return rspCode;
}

REST_RMETHOD_T 
RESTRequest::getMethod()
{
    return reqMethod;
}

void 
RESTRequest::clearParameters()
{
    reqParams.clear();
}

void 
RESTRequest::setParameter( std::string name, std::string value )
{
    reqParams.insert( std::pair<std::string, std::string>(name, value) );
}

int
RESTRequest::processUploadData( const char *upload_data, size_t upload_data_size )
{
    if( postProcessor == NULL )
    {
        postProcessor = MHD_create_post_processor( connection, POSTBUFFERSIZE, RESTRequest::iterate_post, (void *) this );
    }

    if( NULL == postProcessor )
    {
        return 0;
    }

    int result = MHD_post_process( postProcessor, upload_data, upload_data_size );

    printf( "processUploadData: %d\n", result );

    // Attempt to do form data processing.
    if( result == MHD_NO )
    {
        // The data wansn't in url-encoded form to just store it raw.
        inRepresentation.addUploadData( upload_data, upload_data_size );
    }

    return 0;
}

void 
RESTRequest::requestHeaderData()
{
    MHD_get_connection_values( connection, MHD_HEADER_KIND, RESTRequest::iterate_headers, this);	

    MHD_get_connection_values( connection, MHD_GET_ARGUMENT_KIND, RESTRequest::iterate_arguments, this);	

}

int
RESTRequest::processDataIteration( enum MHD_ValueKind kind, const char *key,
                                   const char *filename, const char *content_type,
                                   const char *transfer_encoding, const char *data, uint64_t off,
                                   size_t size )
{
    printf("processDataIteration -- kind: %d\n", kind);

    if( key )
        printf("processDataIteration -- key: %s\n", key);

    if( filename )
        printf("processDataIteration -- filename: %s\n", filename);

    if( content_type )
        printf("processDataIteration -- content_type: %s\n", content_type);

    if( transfer_encoding )
        printf("processDataIteration -- transfer_encoding: %s\n", transfer_encoding);

    printf("processDataIteration -- offset: %ld\n", off);
    printf("processDataIteration -- size: %ld\n", size);

    return MHD_YES;
}

int
RESTRequest::processHeader( enum MHD_ValueKind kind, const char *key, const char *value )
{
    printf( "processHeaderValue -- kind: %d, key: %s, value: %s\n", kind, key, value );

    return MHD_YES;
}

int
RESTRequest::processUrlArg( enum MHD_ValueKind kind, const char *key, const char *value )
{
    printf( "processUrlArg -- kind: %d, key: %s, value: %s\n", kind, key, value );

    return MHD_YES;
}

int
RESTRequest::iterate_post( void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                                  const char *filename, const char *content_type,
                                  const char *transfer_encoding, const char *data, uint64_t off,
                                  size_t size )
{
    RESTRequest *request = (RESTRequest *) coninfo_cls;

    return request->processDataIteration( kind, key, filename, content_type, transfer_encoding, data, off, size );
}


int
RESTRequest::iterate_headers( void *cls, MHD_ValueKind kind, const char *key, const char *value )
{
    RESTRequest *request = (RESTRequest *) cls;

    return request->processHeader( kind, key, value );
}

int
RESTRequest::iterate_arguments( void *cls, MHD_ValueKind kind, const char *key, const char *value )
{
    RESTRequest *request = (RESTRequest *) cls;

    return request->processUrlArg( kind, key, value );
}

void 
RESTRequest::execute()
{
    printf( "execute\n" );
    if( resourcePtr )
    {
        ((RESTResource *)resourcePtr)->executeRequest( this );
    }
}


