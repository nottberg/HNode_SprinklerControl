#ifndef __SWITCHRESOURCE_H__
#define __SWITCHRESOURCE_H__

#include "REST/REST.hpp"
#include "SwitchManager.hpp"

class SwitchListResource : public RESTResource
{
    private:
        SwitchManager *swManager;

    public:
        SwitchListResource();
       ~SwitchListResource();

        void setSwitchManager( SwitchManager *swMgr );

        virtual void restGet( RESTRequest *request );
};

class SwitchResource : public RESTResource
{
    private:
        SwitchManager *swManager;

    public:
        SwitchResource();
       ~SwitchResource();

        void setSwitchManager( SwitchManager *swMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
};

#endif //__SWITCHRESOURCE_H__
