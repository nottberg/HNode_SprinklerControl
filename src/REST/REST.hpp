#ifndef __REST_H__
#define __REST_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#define REST_DAEMON_DEFAULT_PORT 8200

typedef enum RestRepDataItemType
{
   REST_RDI_TYPE_NOTSET,
   REST_RDI_TYPE_URI_PARAMETER,
   REST_RDI_TYPE_HTTP_HEADER,
   REST_RDI_TYPE_QUERY_PARAMETER,
   REST_RDI_TYPE_ENCODED_PARAMETER,
   REST_RDI_TYPE_ENCODED_FILE,
   REST_RDI_TYPE_COOKIE,
   REST_RDI_TYPE_SIMPLE_CONTENT,
}REST_RDI_TYPE_T;

class RESTRepDataItem
{
    private:
        REST_RDI_TYPE_T type;

        std::string     key;
        std::string     contentType;

        std::string     filename;
        std::string     tmpfilepath;

        unsigned long   bufferLength;
        unsigned long   dataLength;
        unsigned char  *bufferPtr;
 
        size_t appendDataToBuffer( const char *buffer, size_t length );
        size_t appendDataToFile( const char *buffer, size_t length );

    public:
        RESTRepDataItem();
       ~RESTRepDataItem();
 
        void setType( REST_RDI_TYPE_T typeValue );
        REST_RDI_TYPE_T getType();

        void setKey( std::string keyValue );
        std::string getKey();

        void setContentType( std::string contentTypeValue );
        std::string getContentType();

        void setFilename( std::string filename );
        std::string getFilename();

        void generateTmpFilePath();
        std::string getTempFilePath();

        unsigned long  getLength();
        unsigned char *getBuffer();

        void resetData();
        size_t addData( unsigned long offset, const char *buffer, size_t length );

        std::string getDataAsStr();
};

class RESTRepresentation
{
    private:
        std::map<std::string, RESTRepDataItem *> uriParam;
        std::map<std::string, RESTRepDataItem *> queryParam;
        std::map<std::string, RESTRepDataItem *> httpHeader;
        std::map<std::string, RESTRepDataItem *> encodedParam;
        std::map<std::string, RESTRepDataItem *> cookieMap;
 
        RESTRepDataItem *simpleContent;

    public:
        RESTRepresentation();
       ~RESTRepresentation();

        // REST_RDI_TYPE_URI_PARAMETER
        bool hasURIParameters();
        void clearURIParameters();
        void addURIParameter( std::string name, std::string value );
        bool getURIParameter( std::string name, std::string &value );

        // REST_RDI_TYPE_HTTP_HEADER
        bool hasHTTPHeaders();
        void clearHTTPHeaders();
        void addHTTPHeader( std::string name, std::string value );
        bool getHTTPHeader( std::string name, std::string &value );

        // REST_RDI_TYPE_QUERY_PARAMETER
        bool hasQueryParameters();
        void clearQueryParameters();
        void addQueryParameter( std::string name, std::string value );
        bool getQueryParameter( std::string name, std::string &value );

        // REST_RDI_TYPE_ENCODED_PARAMETER
        // REST_RDI_TYPE_ENCODED_FILE
        bool hasEncodedParameters();
        void clearEncodedParameters();

        void addEncodedParameter( std::string key, std::string contentType, const char *data, unsigned long offset, unsigned long size );
        void addEncodedFile( std::string key, std::string filename, std::string contentType, const char *data, unsigned long offset, unsigned long size );
        void updateEncodedData( std::string key, const char* data, unsigned long offset, unsigned long size );
 
        bool getEncodedParamInfo( std::string name, bool &isFile, std::string &contentType, unsigned long &contentLength );
        bool getEncodedFileInfo( std::string name, std::string &filename, std::string &localpath );

        bool getEncodedDataAsStr( std::string name, std::string &value );
        unsigned char *getEncodedDataAsPtr( std::string name, unsigned long offset, unsigned long windowLength );

        // REST_RDI_TYPE_COOKIE
        bool hasCookies();
        void clearCookies();
        void addCookie( std::string name, std::string cookie );
        bool getCookie( std::string name, std::string &cookie );

        // REST_RDI_TYPE_SIMPLE_CONTENT,
        bool hasSimpleContent();
        void clearSimpleContent();

        void setSimpleContent( std::string contentType );

        void setSimpleContent( unsigned char *contentPtr, unsigned long contentLength );
        void setSimpleContent( std::string contentType, unsigned char *contentPtr, unsigned long contentLength );

        void appendSimpleContent( unsigned char *contentPtr, unsigned long contentLength );


        unsigned char* getSimpleContentPtr( std::string &contentType, unsigned long &contentLength );
};

class RESTContentField
{
    private:
        std::string name;
        std::string value;
        bool        required;
        
    public:
        RESTContentField();
       ~RESTContentField();

        void setName( std::string nameStr );
        std::string getName();

        void setValue( std::string valueStr );
        std::string getValue();
};

typedef enum RESTContentNodeType
{
    RCNT_NOTSET,
    RCNT_ARRAY,
    RCNT_ID,
    RCNT_OBJECT,
}RCN_TYPE;

class RESTContentNode
{
    private:
        std::string name;

        std::map< std::string, RESTContentField > fieldValues;

        RESTContentNode *parent;        
        std::vector< RESTContentNode * > children;

        RCN_TYPE type;

    public:
        RESTContentNode();
       ~RESTContentNode();

        void setAsArray( std::string name );
        void setAsID( std::string idValue );
        void setAsObject( std::string name );

        RCN_TYPE getType();

        bool isArray();
        bool isID();
        bool isObj();

        void setID( std::string idValue );
        std::string getID();

        void setName( std::string nameValue );
        bool hasName();
        std::string getName();

        void defineField( std::string name, bool required );
        void setField( std::string name, std::string value );
        bool clearField( std::string name );

        bool getFieldInfo( std::string name, bool &required );
        bool getField( std::string name, std::string &value );

        std::vector< RESTContentField* > getFieldList();

        void setParent( RESTContentNode *parentPtr );
        RESTContentNode *getParent();

        void addChild( RESTContentNode *childPtr );
        unsigned long getChildCount();
        RESTContentNode *getChildByIndex( unsigned long index );

        bool getRequiredFields( std::map< std::string, std::string > &fields );
        bool getOptionalFields( std::map< std::string, std::string > &fields );
        bool getUpdateFields( std::map< std::string, std::string > &fields );
};

class RESTContentException : public std::exception
{
    private:
        unsigned long eCode;
        std::string eMsg;

    public:
        RESTContentException( unsigned long errCode, std::string errMsg )
        {
            eCode = errCode;
            eMsg  = errMsg;
        }

       ~RESTContentException() throw() {};

        virtual const char* what() const throw()
        {
            return eMsg.c_str();
        }

        unsigned long getErrorCode() const throw()
        {
            return eCode;
        }

        std::string getErrorMsg() const throw()
        {
            return eMsg;
        }
};

class RESTContentHelper
{
    private:
        RESTContentNode *rootNode;

    public:
        RESTContentHelper();
       ~RESTContentHelper();

        void clearRootNode();
        RESTContentNode *getRootNode();
        RESTContentNode *detachRootNode();
        static void freeDetachedRootNode( RESTContentNode *objPtr );

        bool hasRootObject( std::string objectName, RESTContentNode **objPtr );

        RESTContentNode *getObject( std::string objectName );

        virtual bool parseRawData( RESTRepresentation *repPtr ) = 0;
        virtual bool parseWithTemplate( RESTContentNode *templateCN, RESTRepresentation *repPtr ) = 0;

        virtual bool generateContentRepresentation( RESTRepresentation *repPtr ) = 0;
};

class RESTContentHelperXML : public RESTContentHelper
{
    private:

        bool getAttribute( void *docPtr, void *nodePtr, std::string attrName, std::string &result );
        bool getChildContent( void *docPtr, void *nodePtr, std::string childName, std::string &result );
        void addFieldValues( void *docPtr, void *nodePtr, RESTContentNode *cnPtr );
        void parseTree( void *docPtr, void *nodePtr, RESTContentNode *cnPtr );

        bool findFieldValue( std::string fieldName, RESTContentNode *cnPtr, void *docPtr, void *nodePtr );

        bool generateChildContent(  RESTContentNode *childCN, RESTRepresentation *repPtr );
        bool generateArrayContent( RESTContentNode *arrCN, RESTRepresentation *repPtr );
        bool generateObjectContent( RESTContentNode *objCN, RESTRepresentation *repPtr );
        bool generateIDContent( RESTContentNode *idCN, RESTRepresentation *repPtr );

    public:
        RESTContentHelperXML();
       ~RESTContentHelperXML();
 
        static bool supportsContentDecode( std::string contentType );
        static bool supportsContentCreate( std::string contentType );

        virtual bool parseRawData( RESTRepresentation *repPtr );
        virtual bool parseWithTemplate( RESTContentNode *templateCN, RESTRepresentation *repPtr );

        virtual bool generateContentRepresentation( RESTRepresentation *repPtr );

};

class RESTContentHelperJSON : public RESTContentHelper
{
    private:

    public:
        RESTContentHelperJSON();
       ~RESTContentHelperJSON();
 
};

class RESTContentHelperFactory
{
    private:

    public:
        //static RESTContentHelper *decodeSimpleContent( RESTRepresentation *repPtr );
        //static RESTContentHelper *decodeSimpleContent( RESTContentNode *templateRoot, RESTRepresentation *repPtr );

        static RESTContentNode *newContentNode();
        static void freeContentNode( RESTContentNode *nodePtr );

        static RESTContentHelper *getRequestSimpleContentHelper( RESTRepresentation *repPtr );
        static RESTContentHelper *getResponseSimpleContentHelper( RESTRepresentation *repPtr );

        static void freeContentHelper( RESTContentHelper *chPtr );
};

typedef enum RESTResourceMethodFlags
{
    REST_RMETHOD_NONE   = 0x00,
    REST_RMETHOD_GET    = 0x01,
    REST_RMETHOD_PUT    = 0x02,
    REST_RMETHOD_POST   = 0x04,
    REST_RMETHOD_DELETE = 0x08,
}REST_RMETHOD_T;

typedef enum RESTResponseCode
{
    REST_HTTP_RCODE_NONE             = 0,
    REST_HTTP_RCODE_OK               = 200,
    REST_HTTP_RCODE_CREATED          = 201,
    REST_HTTP_RCODE_ACCEPTED         = 202,
    REST_HTTP_RCODE_PARTIALINFO      = 203,
    REST_HTTP_RCODE_NO_RESPONSE      = 204,
    REST_HTTP_RCODE_MOVED            = 301,
    REST_HTTP_RCODE_FOUND            = 302,
    REST_HTTP_RCODE_METHOD           = 303,
    REST_HTTP_RCODE_NOT_MODIFIED     = 304,
    REST_HTTP_RCODE_BAD_REQUEST      = 400,
    REST_HTTP_RCODE_UNAUTHORIZED     = 401,
    REST_HTTP_RCODE_PAYMENT_REQUIRED = 402,
    REST_HTTP_RCODE_FORBIDDEN        = 403,
    REST_HTTP_RCODE_NOT_FOUND        = 404,
    REST_HTTP_RCODE_INTERNAL_ERROR   = 500,
    REST_HTTP_RCODE_NOT_IMPLEMENTED  = 501,
    REST_HTTP_RCODE_OVERLOADED       = 502,
    REST_HTTP_RCODE_GATEWAY_TIMEOUT  = 503,
}REST_HTTP_RCODE_T;

class RESTRequest
{
    private:
        int connectiontype;
        char *answerstring;
        //struct MHD_PostProcessor *postprocessor;
        
        struct MHD_Connection    *connection;
        struct MHD_PostProcessor *postProcessor;

        RESTRepresentation  inRepresentation;
        RESTRepresentation  outRepresentation;

        std::string urlStr;

        REST_RMETHOD_T reqMethod;

        //std::map<std::string, std::string> reqParams;
        //std::map<std::string, std::string> headerParams;

        REST_HTTP_RCODE_T rspCode;

        void *resourcePtr;
        void *daemonPtr;

        static int iterate_post( void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                                 const char *filename, const char *content_type,
                                 const char *transfer_encoding, const char *data, uint64_t off,
                                 size_t size );

        static int iterate_headers( void *cls, MHD_ValueKind kind, const char *key, const char *value );
        static int iterate_arguments( void *cls, MHD_ValueKind kind, const char *key, const char *value );


    public:
        RESTRequest( struct MHD_Connection *connection, void *daemonPtr );
       ~RESTRequest();

        struct MHD_Connection *getConnection();

        RESTRepresentation *getInboundRepresentation();
        RESTRepresentation *getOutboundRepresentation();

        void setURL( std::string url );
        std::string getURL();

        void decodeRequestMethod( std::string method );
        void setVersion( std::string version );

        REST_RMETHOD_T getMethod();

        void setResponseCode( REST_HTTP_RCODE_T code );
        REST_HTTP_RCODE_T getResponseCode();

        void clearURIParameters();
        void setURIParameter( std::string name, std::string value );
        bool getURIParameter( std::string name, std::string &value );

        int processUploadData( const char *upload_data, size_t upload_data_size );

        void requestRXData();

        int processHeader( enum MHD_ValueKind kind, const char *key, const char *value );
        int processUrlArg( enum MHD_ValueKind kind, const char *key, const char *value );
        int processDataIteration( enum MHD_ValueKind kind, const char *key,
                           const char *filename, const char *content_type,
                           const char *transfer_encoding, const char *data, uint64_t off,
                           size_t size );

        void requestHeaderData();

        void setLink( void *resource );

        void execute();

        void sendResourceCreatedResponse( std::string resID );

        void sendErrorResponse( REST_HTTP_RCODE_T httpCode, unsigned long errCode, std::string errStr );

        void sendResponse();
};

typedef enum RESTResourcePatternElementMatchType
{
    REST_RPE_MATCH_NOTSET = 0,
    REST_RPE_MATCH_STRING = 1,
    REST_RPE_MATCH_PARAM  = 2,
}REST_RPE_MATCH_T;

class RESTResourcePE
{
    private:
        REST_RPE_MATCH_T  matchType;
        std::string       patternStr;

    public:
        
        RESTResourcePE();
       ~RESTResourcePE();

        void setMatch( REST_RPE_MATCH_T matchType, std::string elemStr );

        REST_RPE_MATCH_T getMatchType();

        std::string getMatchElement();

        bool isMatch( std::string testElem );
};



class RESTResource
{
    private:
        std::string    urlPattern;
        REST_RMETHOD_T methods;

        std::vector<RESTResourcePE> patternElements;

        void parse(const std::string& url_s);

    public:
        RESTResource();
       ~RESTResource();

        void setURLPattern( std::string pattern, REST_RMETHOD_T methodFlags );

        bool linkRequest( RESTRequest *request );

        bool executeRequest( RESTRequest *request );

        virtual void restGet( RESTRequest *request );

        virtual void restPut( RESTRequest *request );

        virtual void restPost( RESTRequest *request );

        virtual void restDelete( RESTRequest *request );

};

class RESTDaemon
{
    private:
        struct MHD_Daemon *daemon;

        std::list<RESTResource *> resourceList;

        static int connection_request( void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls);

        static void request_completed( void *cls, struct MHD_Connection *connection, 
                                       void **con_cls, enum MHD_RequestTerminationCode toe );

        int newRequest( RESTRequest *request, const char *upload_data, size_t *upload_data_size );
        int continueRequest( RESTRequest *request, const char *upload_data, size_t *upload_data_size );
        int requestReady( RESTRequest *request );

        void requestComplete( struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe );

    public:
        RESTDaemon();
       ~RESTDaemon();

        int start();
        void stop();

        bool registerResource( RESTResource *resource );

        int sendResponse( RESTRequest *request );

};

#endif // __REST_H__

