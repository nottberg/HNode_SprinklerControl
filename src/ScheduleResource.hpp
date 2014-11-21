#ifndef __SCHEDULERESOURCE_H__
#define __SCHEDULERESOURCE_H__

#include "REST/REST.hpp"
#include "ScheduleManager.hpp"

class ScheduleZoneGroupListResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleZoneGroupListResource();
       ~ScheduleZoneGroupListResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPost( RESTRequest *request );

};

class ScheduleZoneGroupResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleZoneGroupResource();
       ~ScheduleZoneGroupResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
        virtual void restDelete( RESTRequest *request );

};

class ScheduleZoneRuleListResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleZoneRuleListResource();
       ~ScheduleZoneRuleListResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPost( RESTRequest *request );

};

class ScheduleZoneRuleResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleZoneRuleResource();
       ~ScheduleZoneRuleResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
        virtual void restDelete( RESTRequest *request );

};

class ScheduleTriggerGroupListResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleTriggerGroupListResource();
       ~ScheduleTriggerGroupListResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPost( RESTRequest *request );

};

class ScheduleTriggerGroupResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleTriggerGroupResource();
       ~ScheduleTriggerGroupResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
        virtual void restDelete( RESTRequest *request );

};

class ScheduleRuleListResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleRuleListResource();
       ~ScheduleRuleListResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPost( RESTRequest *request );
};

class ScheduleRuleResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleRuleResource();
       ~ScheduleRuleResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
        virtual void restDelete( RESTRequest *request );

};

class ScheduleCalendarEventResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleCalendarEventResource();
       ~ScheduleCalendarEventResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
};

#endif //__SCHEDULERESOURCE_H__
