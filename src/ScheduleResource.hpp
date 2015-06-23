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

class ScheduleZoneRuleListResource : public RESTResourceRESTContentList
{
    private:

    public:
        ScheduleZoneRuleListResource( ScheduleManager &mgr );
       ~ScheduleZoneRuleListResource();

};

class ScheduleZoneRuleResource : public RESTResourceRESTContentObject
{
    private:

    public:
        ScheduleZoneRuleResource( ScheduleManager &mgr );
       ~ScheduleZoneRuleResource();

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

class ScheduleTriggerRuleListResource : public RESTResourceRESTContentList
{
    private:

    public:
        ScheduleTriggerRuleListResource( ScheduleManager &mgr );
       ~ScheduleTriggerRuleListResource();
};

class ScheduleTriggerRuleResource : public RESTResourceRESTContentObject
{
    private:

    public:
        ScheduleTriggerRuleResource( ScheduleManager &mgr );
       ~ScheduleTriggerRuleResource();
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

class ScheduleEventLogResource : public RESTResourceRESTStatusProvider
{
    private:

    public:
        ScheduleEventLogResource( ScheduleManager &mgr );
       ~ScheduleEventLogResource();
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
