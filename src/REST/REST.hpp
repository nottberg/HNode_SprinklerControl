#ifndef __REST_H__
#define __REST_H__

#include <string>
#include <list>

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

class RESTRequest
{
    private:
        int connectiontype;
        char *answerstring;
        struct MHD_PostProcessor *postprocessor;

    public:
        RESTRequest();
       ~RESTRequest();

        int sendPage( struct MHD_Connection *connection, const char *page );

        void setURL( std::string url );
        void decodeRequestMethod( std::string method );
        void setVersion( std::string version );

        int getResponseCode();

        bool hasResponseRepresentation();
        RESTRepresentation getResponseRepresentation();
};

typedef enum RESTResourceMethodFlags
{
    REST_RMETHOD_GET    = 0x01,
    REST_RMETHOD_PUT    = 0x02,
    REST_RMETHOD_POST   = 0x04,
    REST_RMETHOD_DELETE = 0x08,
}REST_RMETHOD_T;

class RESTResource
{
    private:
        std::string    urlPattern;
        REST_RMETHOD_T methods;

    public:
        RESTResource();
       ~RESTResource();

        void setURLPattern( std::string pattern, REST_RMETHOD_T methodFlags );

        bool isURLMatch( RESTRequest *request );
        bool processRequest( RESTRequest *request );

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

