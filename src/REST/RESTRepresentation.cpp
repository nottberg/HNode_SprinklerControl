#include <string.h>

#include "REST.hpp"

RESTRepresentation::RESTRepresentation()
{
    std::string testdata = "<html><body><h1>Test Data</h1><p>Test Data</p></body></html>";

    testDataLength = testdata.size();
    memcpy( testData, testdata.c_str(), testDataLength );
}

RESTRepresentation::~RESTRepresentation()
{


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

