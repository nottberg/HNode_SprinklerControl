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

class RESTRepresentation
{
    private:

    public:
        RESTRepresentation();
       ~RESTRepresentation();

};

typedef enum RESTResourceMethodFlags
{
    REST_RMETHOD_NONE   = 0x00,
    REST_RMETHOD_GET    = 0x01,
    REST_RMETHOD_PUT    = 0x02,
    REST_RMETHOD_POST   = 0x04,
    REST_RMETHOD_DELETE = 0x08,
}REST_RMETHOD_T;

class RESTRequest
{
    private:
        int connectiontype;
        char *answerstring;
        struct MHD_PostProcessor *postprocessor;
 
        std::string urlStr;

        REST_RMETHOD_T reqMethod;

        std::map<std::string, std::string> reqParams;

    public:
        RESTRequest();
       ~RESTRequest();

        int sendPage( struct MHD_Connection *connection, const char *page );

        void setURL( std::string url );
        std::string getURL();

        void decodeRequestMethod( std::string method );
        void setVersion( std::string version );

        REST_RMETHOD_T getMethod();

        int getResponseCode();

        void clearParameters();
        void setParameter( std::string name, std::string value );
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

        bool processRequest( RESTRequest *request, RESTRepresentation *data );

        virtual void restGet( RESTRequest *request, RESTRepresentation *data );

        virtual void restPut( RESTRequest *request, RESTRepresentation *data );

        virtual void restPost( RESTRequest *request, RESTRepresentation *data );

        virtual void restDelete( RESTRequest *request, RESTRepresentation *data );

};

class RESTDaemon
{
    private:
        struct MHD_Daemon *daemon;

        std::list<RESTResource *> resourceList;

        static int connection_request( void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls );

        static void request_completed( void *cls, struct MHD_Connection *connection, 
                                       void **con_cls, enum MHD_RequestTerminationCode toe );

        int newRequest( struct MHD_Connection *connection,
                        std::string url, std::string method,
                        std::string version, const char *upload_data,
                        size_t *upload_data_size, void **con_cls );

        void requestComplete( struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe );

    public:
        RESTDaemon();
       ~RESTDaemon();

        int start();
        void stop();

        bool registerResource( RESTResource *resource );
};

#endif // __REST_H__

