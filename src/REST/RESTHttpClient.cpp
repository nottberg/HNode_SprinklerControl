#include <string.h>
#include <stdio.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <curl/curl.h>

#include "REST.hpp"

RESTHttpClient::RESTHttpClient()
{
    rType   = RHC_REQTYPE_GET;
    debug   = true;
    timeout = 30;
}

RESTHttpClient::~RESTHttpClient()
{

}

RESTRepresentation &
RESTHttpClient::getInboundRepresentation()
{
    return inData;
}

RESTRepresentation &
RESTHttpClient::getOutboundRepresentation()
{
    return outData;
}

void 
RESTHttpClient::setRequest( RHC_REQTYPE_T reqType, std::string baseURL )
{
    rType = reqType;
    bURL  = baseURL;
}

size_t
RESTHttpClient::processResponseHeader( void *buffer, size_t size, size_t nmemb, void *userp )
{
    RESTHttpClient *client = (RESTHttpClient *) userp;
    size_t length = size * nmemb;
    RESTRepresentation &dataRep = client->getInboundRepresentation();

    std::string dataStr( (const char *)buffer, length );

    boost::regex headerRE("([A-Za-z0-9]*): (.*)");
    boost::cmatch match;

    if( boost::regex_match( dataStr.c_str(), match, headerRE ) )
    {
        dataRep.addHTTPHeader( match[1].str(), match[2].str() );
    }

    return length;
}

size_t
RESTHttpClient::processInboundData( void *buffer, size_t size, size_t nmemb, void *userp )
{
    RESTHttpClient *client = (RESTHttpClient *) userp;
    size_t length = size * nmemb;
    RESTRepresentation &dataRep = client->getInboundRepresentation();

    dataRep.appendSimpleContent( (unsigned char *)buffer, length );

    return length;
}

size_t
RESTHttpClient::processOutboundData( void *buffer, size_t size, size_t nmemb, void *userp )
{
    RESTHttpClient *client = (RESTHttpClient *) userp;
    size_t length = size * nmemb;
    RESTRepresentation &dataRep = client->getOutboundRepresentation();
    std::string contentType;
    unsigned long contentLength;

    unsigned char *dataPtr = dataRep.getSimpleContentPtr( contentType, contentLength );

    size_t remaining = contentLength - client->sentLength;

    if( remaining < length )
       length = remaining;

    memcpy( buffer, &dataPtr[client->sentLength], length );

    client->sentLength += length;

    return length;
}

void 
RESTHttpClient::makeRequest()
{
    CURL *curl;
    CURLcode res;

    // Build up the url
    std::string url = bURL; 

    // Process possible headers 
    if( outData.hasQueryParameters() )
    {
        std::map< std::string, std::string > qParam;

        // Get the query parameters
        outData.getQueryParameterMap( qParam );

        // Add each one
        unsigned long pCnt = 0;
        for(  std::map< std::string, std::string >::iterator it = qParam.begin(); it != qParam.end(); it++ )
        {
            // Add the character that will proceed the parameter
            if( pCnt == 0 )
                url += "?";
            else
                url += "&";

            // Add the parameter text
            url += it->first + "=" + it->second;   
        }
    }

    // get a curl handle 
    curl = curl_easy_init();

    if( curl ) 
    {
        struct curl_slist *slist = NULL;
  	
        // Process possible headers 
        if( outData.hasHTTPHeaders() == false )
        {
	        slist = curl_slist_append( slist, "Accept: */*" );
    	    slist = curl_slist_append( slist, "Content-Type: text/xml" );
        }
        else
        {
            std::vector< std::string > hnList;

            outData.getHTTPHeaderNameList( hnList );

            for( std::vector< std::string >::iterator it = hnList.begin(); it != hnList.end(); it++ )
            {
                std::string value;
                std::string hStr;

                outData.getHTTPHeader( *it, value );

                hStr = *it + ": " + value;

          	    slist = curl_slist_append( slist, hStr.c_str() );          
            }
        }


        // If debug then turn on verbose
        if( debug == true )
            curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
        else
            curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
            
        // Return headers via the callback and not in datastream

        // Tell the otherside what sort of beasty we are.
	    curl_easy_setopt( curl, CURLOPT_USERAGENT,  "Linux C  libcurl" );

        // Set the URL
	    curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );

        // Set the timeout value
	    curl_easy_setopt( curl, CURLOPT_TIMEOUT, timeout );
	
        // Setup response header parsing
	    curl_easy_setopt( curl, CURLOPT_HEADER, 0 );
        curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, processResponseHeader );
        curl_easy_setopt( curl, CURLOPT_HEADERDATA, this );
       
        // Setup the read callback 
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, processInboundData );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );

        // Setup the write callback 
        curl_easy_setopt( curl, CURLOPT_READFUNCTION, processOutboundData );
        curl_easy_setopt( curl, CURLOPT_READDATA, this );

        // Specialize for each call type
        switch( rType )
        {       
            case RHC_REQTYPE_GET:
            break;

            case RHC_REQTYPE_POST:
                curl_easy_setopt(curl, CURLOPT_POST, 1L );
	            curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, outData.getSimpleContentLength() ); 
            break;

            case RHC_REQTYPE_PUT:
                curl_easy_setopt( curl, CURLOPT_UPLOAD, 1L );
                curl_easy_setopt( curl, CURLOPT_PUT, 1L );
                curl_easy_setopt( curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) outData.getSimpleContentLength() );
            break;

            case RHC_REQTYPE_DELETE:
                curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, "DELETE" );
            break;
        }

        // Set outbound headers		
	    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, slist );

        // Init tracking varaiables before execution
        sentLength = 0;

        // Perform the request, res will get the return code 
        res = curl_easy_perform(curl);

        // Check for errors 
        if( res != CURLE_OK )
        {
            fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
            return;
        }

        // always cleanup 
        curl_easy_cleanup( curl );
        curl_slist_free_all( slist );

    }
    else
    {

    }
}

unsigned int 
RESTHttpClient::reqResult()
{

}

bool
RESTHttpClient::getLocationHeaderURL( std::string &rtnStr )
{
    return inData.getHTTPHeader( "Location", rtnStr );
}

bool
RESTHttpClient::getLocationHeaderTerminal( std::string &rtnStr )
{
    std::string urlStr;

    if( getLocationHeaderURL( urlStr ) )
    {
        rtnStr.clear();
        return true;
    }

    boost::regex re("/+");
    boost::sregex_token_iterator i( urlStr.begin(), urlStr.end(), re, -1);
    boost::sregex_token_iterator j;

    while( i != j )
    {
        std::string tmpStr = *i;
        boost::trim( tmpStr );

        rtnStr = tmpStr;
        i++;
    }

    // Found the string
    return false;
}

