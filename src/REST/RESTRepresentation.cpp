#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <list>

#include "REST.hpp"

RESTRepDataItem::RESTRepDataItem()
{
    type = REST_RDI_TYPE_NOTSET;

    bufferLength = 0;
    dataLength   = 0;
    bufferPtr    = NULL;
}

RESTRepDataItem::~RESTRepDataItem()
{

}

void 
RESTRepDataItem::setType( REST_RDI_TYPE_T typeValue )
{
    type = typeValue;
}

REST_RDI_TYPE_T RESTRepDataItem::getType()
{
    return type;
}

void 
RESTRepDataItem::setKey( std::string keyValue )
{
    key = keyValue;
}

std::string 
RESTRepDataItem::getKey()
{
    return key;
}

void 
RESTRepDataItem::setContentType( std::string ctValue )
{
    contentType = ctValue;
}

std::string 
RESTRepDataItem::getContentType()
{
    return contentType;
}

void 
RESTRepDataItem::setFilename( std::string fname )
{
    filename = fname;
}

std::string 
RESTRepDataItem::getFilename()
{
    return filename;
}

void 
RESTRepDataItem::generateTmpFilePath()
{
    char templateStr[] = "/tmp/hrestXXXXXX";
    int fd;

    // Create a temporary file with a unique prefix
    fd = mkstemp( templateStr );

    if( fd != -1 )
    {
        // Copy it into the member string
        tmpfilepath = templateStr;

        // Close the file for now
        close( fd );
    }
}

std::string 
RESTRepDataItem::getTempFilePath()
{
    return tmpfilepath;
}

unsigned long  
RESTRepDataItem::getLength()
{
    return dataLength;
}

unsigned char *
RESTRepDataItem::getBuffer()
{
    return bufferPtr;
}

void 
RESTRepDataItem::resetData()
{
    dataLength = 0;
}

size_t 
RESTRepDataItem::appendDataToBuffer( const char *buffer, size_t length )
{
    unsigned char *tmpBuf;
    unsigned long  neededBytes;

    printf("RESTRepDataItem::appendData -- size: %ld\n", length);
    
    // Calculate the number of bytes needed
    neededBytes = dataLength + length;

    // Check if there is already enough space in the buffer
    if( neededBytes > bufferLength )
    {
        // Buffer needs to be increased in size.  Calculate the size of
        // new buffer.
        neededBytes = ( neededBytes & ~(1024) ) + 1024;
 
        tmpBuf = (unsigned char *) malloc( neededBytes );
        bufferLength = neededBytes;

        if( dataLength )
        {
            memcpy( tmpBuf, bufferPtr, dataLength );
            free( bufferPtr );
            bufferPtr = NULL;
        }

        bufferPtr = tmpBuf;
    }

    memcpy( &tmpBuf[ dataLength ], buffer, length );
    dataLength += length;

    return 0;
}

size_t 
RESTRepDataItem::appendDataToFile( const char *buffer, size_t length )
{
     FILE   *fp;
     size_t  written;

     fp = fopen( tmpfilepath.c_str(), "a" );

     written = fwrite( buffer, 1, length, fp );

     fclose( fp );
}

size_t 
RESTRepDataItem::addData( unsigned long offset, const char *buffer, size_t length )
{
    // Check if the offset is going to cause problems

    // Check if this is a file or in memory buffer.
    if( type == REST_RDI_TYPE_POST_FILE )
    {
        // Add the contents to the temporary file
        return appendDataToFile( buffer, length );
    }

    // Add content to in memory buffer.
    return appendDataToBuffer( buffer, length );
}



std::string
RESTRepDataItem::getDataAsStr()
{
    std::string resultStr( (const char *)bufferPtr, dataLength );
    return resultStr;
}

RESTRepresentation::RESTRepresentation()
{
//    rspDataBufferLength = 0;
//    rspDataLength       = 0;
//    rspData             = NULL;

//    contentLength       = 0;
    simpleContent = NULL;
}

RESTRepresentation::~RESTRepresentation()
{
//    if( rspDataBufferLength )
//        free( rspData );
}

void 
RESTRepresentation::clearURIParameters()
{

}

void 
RESTRepresentation::addURIParameter( std::string name, std::string value )
{
    RESTRepDataItem *diPtr;

    diPtr = new RESTRepDataItem();
    if( diPtr == NULL )
    {
        return;
    }

    diPtr->setType( REST_RDI_TYPE_URI_PARAMETER );
    diPtr->setKey( name );

    diPtr->addData( 0, value.c_str(), value.size() );

    std::pair<std::string, RESTRepDataItem *> nvpair( name, diPtr );
    diMap.insert( nvpair );

    diList.push_back( diPtr );
}

bool 
RESTRepresentation::getURIParameter( std::string name, std::string &value )
{
    std::map<std::string, RESTRepDataItem *>::iterator it;

    value.clear();

    it = diMap.find( name );

    if( it == diMap.end() )
    {
        return true;
    }   

    value = it->second->getDataAsStr();
    
    return false;
}

void 
RESTRepresentation::clearHTTPHeaders()
{

}

void 
RESTRepresentation::addHTTPHeader( std::string name, std::string value )
{
    RESTRepDataItem *diPtr;

    diPtr = new RESTRepDataItem();
    if( diPtr == NULL )
    {
        return;
    }

    diPtr->setType( REST_RDI_TYPE_HTTP_HEADER );
    diPtr->setKey( name );

    diPtr->addData( 0, value.c_str(), value.size() );

    std::pair<std::string, RESTRepDataItem *> nvpair( name, diPtr );
    diMap.insert( nvpair );

    diList.push_back( diPtr );
}

bool 
RESTRepresentation::getHTTPHeader( std::string name, std::string &value )
{
    std::map<std::string, RESTRepDataItem *>::iterator it;

    value.clear();

    it = diMap.find( name );

    if( it == diMap.end() )
    {
        return true;
    }   

    value = it->second->getDataAsStr();
    
    return false;
}

void 
RESTRepresentation::clearPOSTParameters()
{

}

bool 
RESTRepresentation::getPOSTParameter( std::string name, std::string &value )
{

}

void 
RESTRepresentation::updatePOSTParameter( std::string key, const char* data, unsigned long off, unsigned long size )
{
    printf("updatePOSTParameter -- key: %s, off: %ld, size: %ld\n", key.c_str(), off, size);

    std::map<std::string, RESTRepDataItem *>::iterator it;

    it = diMap.find( key );

    if( it == diMap.end() )
    {
        return;
    }   

    RESTRepDataItem *diPtr = it->second;

    diPtr->addData( off, (const char*)data, size );
}

void 
RESTRepresentation::addPOSTParameter( std::string key, std::string contentType, const char *data, unsigned long off, unsigned long size )
{
    printf("updatePOSTParameter -- key: %s, off: %ld, size: %ld, contentType: %s\n", key.c_str(), off, size, contentType.c_str());

    RESTRepDataItem *diPtr;

    diPtr = new RESTRepDataItem();
    if( diPtr == NULL )
    {
        return;
    }

    diPtr->setType( REST_RDI_TYPE_POST_PARAMETER );
    diPtr->setContentType( contentType );

    diPtr->addData( off, (const char*)data, size );

    diList.push_back( diPtr );
}

void 
RESTRepresentation::clearPOSTFile()
{

}

bool 
RESTRepresentation::getPOSTFile( std::string name, std::string &filepath )
{

}

void 
RESTRepresentation::updatePOSTFile( std::string key, const char* data, unsigned long off, unsigned long size )
{
    printf("updatePOSTFile -- key: %s, off: %ld, size: %ld\n", key.c_str(), off, size);

    std::map<std::string, RESTRepDataItem *>::iterator it;

    it = diMap.find( key );

    if( it == diMap.end() )
    {
        return;
    }   

    RESTRepDataItem *diPtr = it->second;

    diPtr->addData( off, (const char*)data, size );
}

void 
RESTRepresentation::addPOSTFile( std::string key, std::string filename, std::string contentType, const char *data, unsigned long off, unsigned long size )
{
    printf("addPOSTFile -- key: %s, off: %ld, size: %ld, filename: %s, contentType: %s\n", key.c_str(), off, size, filename.c_str(), contentType.c_str());

    RESTRepDataItem *diPtr;

    diPtr = new RESTRepDataItem();
    if( diPtr == NULL )
    {
        return;
    }

    diPtr->setType( REST_RDI_TYPE_POST_FILE );
    diPtr->setContentType( contentType );

    diPtr->setFilename( filename );
    diPtr->generateTmpFilePath();

    diPtr->addData( off, (const char*)data, size );

    diList.push_back( diPtr );
}

void 
RESTRepresentation::clearCookie()
{

}

void 
RESTRepresentation::addCookie( std::string name, std::string cookie )
{

}

bool 
RESTRepresentation::getCookie( std::string name, std::string &cookie )
{

}

void 
RESTRepresentation::clearSimpleContent()
{
    if( simpleContent != NULL )
        delete simpleContent;

    simpleContent = NULL;
}

void 
RESTRepresentation::setSimpleContent( unsigned char *contentPtr, unsigned long contentLength )
{
    std::string contentType;

    getHTTPHeader( "Content-Type", contentType );

    setSimpleContent( contentType, contentPtr, contentLength ); 
}

void 
RESTRepresentation::setSimpleContent( std::string contentType, unsigned char *contentPtr, unsigned long contentLength )
{
    RESTRepDataItem *diPtr;

    printf("setSimpleContent: %ld\n", contentLength);

    diPtr = new RESTRepDataItem();
    if( diPtr == NULL )
    {
        return;
    }

    diPtr->setType( REST_RDI_TYPE_SIMPLE_CONTENT );
    diPtr->setContentType( contentType );
    diPtr->addData( 0, (const char*)contentPtr, contentLength );

    simpleContent = diPtr;
}

void 
RESTRepresentation::appendSimpleContent( unsigned char *contentPtr, unsigned long contentLength )
{
    if( simpleContent == NULL )
        return;

    printf( "appendSimpleContent -- new Length: %ld\n", contentLength );
    printf( "appendSimpleContent -- cur Length: %ld\n", simpleContent->getLength() );

    simpleContent->addData( simpleContent->getLength(), (const char *)contentPtr, contentLength ); 
}

bool
RESTRepresentation::hasSimpleContent()
{
    if( simpleContent != NULL )
        return true;

    return false;
}

unsigned char* 
RESTRepresentation::getSimpleContentPtr( std::string &contentType, unsigned long &contentLength )
{
    contentType.clear();
    contentLength = 0;

    if( simpleContent == NULL )
        return NULL;

    contentType   = simpleContent->getContentType();
    contentLength = simpleContent->getLength();
    return simpleContent->getBuffer();
}

#if 0
void 
RESTRepresentation::setContentType( std::string cType )
{
//    contentType = cType;
}

std::string 
RESTRepresentation::getContentType()
{
//    return contentType;
}

void 
RESTRepresentation::setContentLength( std::string contentLength )
{
//    contentLength = strtol( contentLength.c_str(), NULL, 0 );
}

unsigned long 
RESTRepresentation::getContentLength()
{
//    return contentLength;
}

void 
RESTRepresentation::addParameter( std::string name, std::string value )
{
    std::pair<std::string, std::string> nvpair(name,value);
    printf( "RESTRepresentation::addParameter -- name: %s, value: %s\n", name.c_str(), value.c_str() );

    //paramMap.insert( nvpair );
}

unsigned long
RESTRepresentation::getLength()
{
    //return rspDataLength;
}

unsigned char *
RESTRepresentation::getBuffer()
{
    //return rspData;
}

void 
RESTRepresentation::resetData()
{
    //rspDataLength = 0;
}

size_t 
RESTRepresentation::appendData( const char *buffer, size_t length )
{
    unsigned char *tmpBuf;
    unsigned long  neededBytes;

    printf("RESTRepresentation::appendData -- size: %ld\n", length);
    
    // Calculate the number of bytes needed
    neededBytes = rspDataLength + length;

    // Check if there is already enough space in the buffer
    if( neededBytes > rspDataBufferLength )
    {
        // Buffer needs to be increased in size.  Calculate the size of
        // new buffer.
        neededBytes = ( neededBytes & ~(1024) ) + 1024;
 
        tmpBuf = (unsigned char *) malloc( neededBytes );
        rspDataBufferLength = neededBytes;

        if( rspDataLength )
        {
            memcpy( tmpBuf, rspData, rspDataLength );
            free( rspData );
            rspData = NULL;
        }

        rspData = tmpBuf;
    }

    memcpy( &tmpBuf[rspDataLength], buffer, length );
    rspDataLength += length;

    return 0;
}
#endif




