#ifndef __SCHEDULE_MANAGER_H__
#define __SCHEDULE_MANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" 

using namespace boost::posix_time;
using namespace boost::gregorian;

// Forward Declaration
class ScheduleEventRule;

class ScheduleDateTime
{
    private:
        //ptime time;
        //date dt;

    protected:
        ptime time;

    public:

        ScheduleDateTime();
       ~ScheduleDateTime();

        void getCurrentTime();

        void setTime( ScheduleDateTime &time );
        void addDuration( time_duration &td );

        std::string getSimpleString();
        std::string getISOString();

        long getHour();
        long getMinute();
        long getSecond();

        long getDayOfWeek();

        void addSeconds( long seconds );
        void addMinutes( long minutes );
        void addHours( long hours );

        void subSeconds( long seconds );
        void subMinutes( long minutes );
        void subHours( long hours );

        void replaceTimeOfDay( ScheduleDateTime &time );

        bool isBefore( ScheduleDateTime &time );
        bool isAfter( ScheduleDateTime &time ); 
};

class ScheduleDuration
{
    private:

    protected:
        time_duration td;

    public:

        ScheduleDuration();
       ~ScheduleDuration();
};

class ScheduleAction
{
    private:
        std::vector<std::string> zoneIDList;

    public:

        ScheduleAction();
       ~ScheduleAction();
};

typedef enum ScheduleEventExecState
{
    SESTATE_IDLE,
    SESTATE_READY,
    SESTATE_RUNNING,
    SESTATE_COMPLETE,
    SESTATE_RECYCLE
}SESTATE;

class ScheduleEvent
{
    private:

        ScheduleEventRule *parentRule;

        SESTATE            state;

        std::string        id;
        std::string        title;
        std::string        url;

        ScheduleDateTime   start;
        ScheduleDateTime   end;
        ScheduleDateTime   recycle;
       
        ScheduleAction     startAction;
        ScheduleAction     pollAction;
        ScheduleAction     endAction;

    public:
        ScheduleEvent();
       ~ScheduleEvent();

        void setId( std::string );
        std::string getId();

        void setTitle( std::string );
        std::string getTitle();

        void setStartTime( ScheduleDateTime &time );
        void getStartTime( ScheduleDateTime &time );

        void setEndTime( ScheduleDateTime &time );
        void getEndTime( ScheduleDateTime &time );

        void setStartAction( ScheduleAction &action );
        void getStartAction( ScheduleAction &action );

        void setPollAction( ScheduleAction &action );
        void getPollAction( ScheduleAction &action );

        void setEndAction( ScheduleAction &action );
        void getEndAction( ScheduleAction &action );

        void setReady();
        SESTATE getState();

        bool processCurrent( ScheduleDateTime &curTime );
        void processFinal();

        bool isComplete();
        void setComplete();

        bool isRecyclable();
};

class ScheduleRecurrenceRule
{
    private:
        // Frequency

        // Interval

        // Count

        // Week Start

        // By Day

        // By Month Day

        // By Year Day

        // Excluding

    public:

        ScheduleRecurrenceRule();
       ~ScheduleRecurrenceRule();
};

class ScheduleZoneRule
{
    private:
        std::string   zoneID;
        unsigned int  duration;

    public:
        ScheduleZoneRule();
       ~ScheduleZoneRule();
};

class ScheduleEventList
{
    private:
        std::vector<ScheduleEvent *> eventList;

    public:
        ScheduleEventList();
       ~ScheduleEventList();
     
        void addEvent( ScheduleEvent *event );
        void reapEvents();

        unsigned int getEventCount();
        ScheduleEvent *getEvent( unsigned int index );
};

typedef enum ScheduleEventRuleZoneEventPolicy 
{
    SER_ZEP_NOTSET     = 0,
    SER_ZEP_SEQUENCIAL = 1,
}SER_ZONE_EVENT_POLICY;

typedef enum ScheduleEventRuleType 
{
    SER_TYPE_NOTSET     = 0,
    SER_TYPE_DAY        = 1,
    SER_TYPE_WEEKLY     = 2,
    SER_TYPE_INTERVAL   = 3,
}SER_OCURRENCE_TYPE;

class ScheduleEventRule
{
    private:

        bool enabled;

        SER_OCURRENCE_TYPE type;

        std::string      id;
        std::string      title;

        std::string      url;

        //std::vector<ScheduleRecurrenceRule> recurrenceList;
        std::vector<std::string> zoneIDList;

        ScheduleDateTime refTime;

        SER_ZONE_EVENT_POLICY zonePolicy;

        bool fireManually;
        bool eventsPending;

        void createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

    public:
        ScheduleEventRule();
       ~ScheduleEventRule();

        void setEnabled();
        void clearEnabled();
        bool getEnabled();

        void startManually();

        void setID( std::string idValue );
        std::string getID();

        void setTitle( std::string titleValue );
        std::string getTitle();
        
        void setURL( std::string urlValue );
        std::string getURL();

        void setOcurrenceType( SER_OCURRENCE_TYPE typeValue );
        SER_OCURRENCE_TYPE getOcurrenceType();

        void setReferenceTime( ScheduleDateTime &time );
        void getReferenceTime( ScheduleDateTime &time );

        void setZoneEventPolicy( SER_ZONE_EVENT_POLICY eventPolicy );
        SER_ZONE_EVENT_POLICY getZoneEventPolicy();

        void clearZoneList();
        void addZoneId( std::string zoneId );

        void updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

};

class ScheduleManager
{
    private:
        std::string cfgPath;

        std::vector<ScheduleEventRule *> eventRuleList;

        ScheduleEventList activeEvents;

    public:
        ScheduleManager();
       ~ScheduleManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        void processCurrentEvents( ScheduleDateTime &curTime );

        ScheduleEventList *getActiveEvents();
        ScheduleEventList *getEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime );

        void freeScheduleEventList( ScheduleEventList *listPtr );
};

#endif // __SCHEDULE_MANAGER_H__

