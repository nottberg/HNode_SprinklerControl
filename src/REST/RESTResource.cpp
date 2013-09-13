#include "REST.hpp"

RESTResource::RESTResource()
{

}

RESTResource::~RESTResource()
{

}

void 
RESTResource::setURLPattern( std::string pattern, REST_RMETHOD_T methodFlags )
{
    urlPattern = pattern;
    methods = methodFlags;
}

bool 
RESTResource::isURLMatch( RESTRequest *request )
{

}

bool 
RESTResource::processRequest( RESTRequest *request )
{

}

