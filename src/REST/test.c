#include <stdio.h>

#include "REST.hpp"

int
main()
{
    RESTDaemon   rest;
    RESTResource controller;
    RESTResource toggles;
    RESTResource zones;
    RESTResource zone;

    controller.setURLPattern( "/irrigation", REST_RMETHOD_GET );
    rest.registerResource( &controller );

    toggles.setURLPattern( "/irrigation/toggles", REST_RMETHOD_GET );
    rest.registerResource( &toggles );

    zones.setURLPattern( "/irrigation/zones", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
    rest.registerResource( &zones );

    zone.setURLPattern( "/irrigation/zones/{zoneid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE) ); 
    rest.registerResource( &zone );

    rest.start();

    getchar();

    rest.stop();

    return 0;
#if 0
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);

    if( NULL == daemon )
        return 1;
    getchar();
    MHD_stop_daemon( daemon );
    return 0;
#endif
}

