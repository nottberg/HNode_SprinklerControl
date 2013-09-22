#include <string.h>

#include "REST.hpp"

RESTRequest::RESTRequest()
{
    //int connectiontype;
    //char *answerstring;
    //struct MHD_PostProcessor *postprocessor;
    
    reqMethod = REST_RMETHOD_NONE;
}

RESTRequest::~RESTRequest()
{

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

int
RESTRequest::sendPage( struct MHD_Connection *connection, const char *page )
{
#if 0
    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer( strlen( page ), (void *)page, MHD_RESPMEM_PERSISTENT );

    if( !response )
        return MHD_NO;

    ret = MHD_queue_response( connection, MHD_HTTP_OK, response );

    MHD_destroy_response( response );

    return ret;
#endif
}

int 
RESTRequest::getResponseCode()
{

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

