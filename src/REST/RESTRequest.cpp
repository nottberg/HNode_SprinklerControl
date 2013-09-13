
#include "REST.hpp"

RESTRequest::RESTRequest()
{
    //int connectiontype;
    //char *answerstring;
    //struct MHD_PostProcessor *postprocessor;

}

RESTRequest::~RESTRequest()
{

}

void 
RESTRequest::setURL( std::string url )
{

}

void 
RESTRequest::decodeRequestMethod( std::string method )
{

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

bool
RESTRequest::hasResponseRepresentation()
{

}

RESTRepresentation 
RESTRequest::getResponseRepresentation()
{

}

