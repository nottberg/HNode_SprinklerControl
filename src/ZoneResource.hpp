#ifndef __ZONERESOURCE_H__
#define __ZONERESOURCE_H__

#include "REST/REST.hpp"
#include "ZoneManager.hpp"

class ZoneListResource : public RESTResource
{
    private:
        ZoneManager *zoneManager;

    public:
        ZoneListResource();
       ~ZoneListResource();

        void setZoneManager( ZoneManager *swMgr );

        virtual void restGet( RESTRequest *request );
};

class ZoneResource : public RESTResource
{
    private:
        ZoneManager *zoneManager;

    public:
        ZoneResource();
       ~ZoneResource();

        void setZoneManager( ZoneManager *swMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
};

class ZoneDiagramResource : public RESTResource
{
    private:
        ZoneManager *zoneManager;

    public:
        ZoneDiagramResource();
       ~ZoneDiagramResource();

        void setZoneManager( ZoneManager *swMgr );

        virtual void restGet( RESTRequest *request );
};

#endif //__ZONERESOURCE_H__
