#ifndef __SCHEDULERESOURCE_H__
#define __SCHEDULERESOURCE_H__

#include "REST/REST.hpp"
#include "ScheduleManager.hpp"

class ScheduleZoneGroupListResource : public RESTResourceRESTContentList
{
    private:

    public:
        ScheduleZoneGroupListResource( ScheduleManager &mgr );
       ~ScheduleZoneGroupListResource();
};

class ScheduleZoneGroupResource : public RESTResourceRESTContentObject
{
    private:

    public:
        ScheduleZoneGroupResource( ScheduleManager &mgr );
       ~ScheduleZoneGroupResource();
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

class ScheduleTriggerGroupListResource : public RESTResourceRESTContentList
{
    private:

    public:
        ScheduleTriggerGroupListResource( ScheduleManager &mgr );
       ~ScheduleTriggerGroupListResource();
};

class ScheduleTriggerGroupResource : public RESTResourceRESTContentObject
{
    private:

    public:
        ScheduleTriggerGroupResource( ScheduleManager &mgr );
       ~ScheduleTriggerGroupResource();
};

class ScheduleTriggerRuleListResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleTriggerRuleListResource();
       ~ScheduleTriggerRuleListResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPost( RESTRequest *request );

};

class ScheduleTriggerRuleResource : public RESTResource
{
    private:
        ScheduleManager *schManager;

    public:
        ScheduleTriggerRuleResource();
       ~ScheduleTriggerRuleResource();

        void setScheduleManager( ScheduleManager *schMgr );

        virtual void restGet( RESTRequest *request );
        virtual void restPut( RESTRequest *request );
        virtual void restDelete( RESTRequest *request );

};

class ScheduleRuleListResource : public RESTResourceRESTContentList
{
    private:

    public:
        ScheduleRuleListResource( ScheduleManager &mgr );
       ~ScheduleRuleListResource();
};

class ScheduleRuleResource : public RESTResourceRESTContentObject
{
    private:

    public:
        ScheduleRuleResource( ScheduleManager &mgr );
       ~ScheduleRuleResource();
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
