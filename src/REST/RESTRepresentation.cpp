#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "REST.hpp"

RESTRepresentation::RESTRepresentation()
{
    std::string testdata = "<html><body><h1>Test Data</h1><p>Test Data</p></body></html>";

    testDataLength = testdata.size();
    memcpy( testData, testdata.c_str(), testDataLength );

    contentLength = 0;
}

RESTRepresentation::~RESTRepresentation()
{


}

void 
RESTRepresentation::setContentType( std::string cType )
{
    contentType = cType;
}

std::string 
RESTRepresentation::getContentType()
{
    return contentType;
}

void 
RESTRepresentation::setContentLength( std::string contentLength )
{
    contentLength = strtol( contentLength.c_str(), NULL, 0 );
}

unsigned long 
RESTRepresentation::getContentLength()
{
    return contentLength;
}

void 
RESTRepresentation::addParameter( std::string name, std::string value )
{
    std::pair<std::string, std::string> nvpair(name,value);
    printf( "RESTRepresentation::addParameter -- name: %s, value: %s\n", name.c_str(), value.c_str() );

    paramMap.insert( nvpair );
}

unsigned long
RESTRepresentation::getLength()
{
    return testDataLength;
}

unsigned char *
RESTRepresentation::getBuffer()
{
    return testData;
}

size_t 
RESTRepresentation::addUploadData( const char *buffer, size_t length )
{
    printf("RESTRepresentation::addUploadData -- size: %ld\n", length);

    return 0;
}





