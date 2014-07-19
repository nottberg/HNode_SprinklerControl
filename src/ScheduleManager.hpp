#ifndef __SCHEDULE_MANAGER_H__
#define __SCHEDULE_MANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" 

#include "ZoneManager.hpp"

using namespace boost::posix_time;
using namespace boost::gregorian;

// Forward Declaration
class ScheduleEventRule;
class ScheduleTimeDuration;
class ScheduleTimePeriod;
class ScheduleTimeIterator;

class ScheduleDateTime
{
    friend class ScheduleTimeDuration;
    friend class ScheduleTimePeriod;
    friend class ScheduleTimeIterator;

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
        std::string getExtendedISOString();

        void setTimeFromISOString( std::string isoTime );

        long getHour();
        long getMinute();
        long getSecond();

        long getDayOfWeek();

        void addSeconds( long seconds );
        void addMinutes( long minutes );
        void addHours( long hours );
        void addDays( long days );

        void subSeconds( long seconds );
        void subMinutes( long minutes );
        void subHours( long hours );
        void subDays( long days );

        void replaceTimeOfDay( ScheduleDateTime &time );

        bool isBefore( ScheduleDateTime &time );
        bool isAfter( ScheduleDateTime &time ); 
};

class ScheduleTimeDuration
{
    friend class ScheduleTimePeriod;
    friend class ScheduleTimeIterator;

    private:

    protected:
        time_duration td;

    public:

        ScheduleTimeDuration();
       ~ScheduleTimeDuration();

        void setFromString( std::string timeStr );

        long asTotalSeconds();
};

class ScheduleTimePeriod
{
    private:

    protected:
        time_period *tp;

    public:

        ScheduleTimePeriod();
       ~ScheduleTimePeriod();

        void setFromStartAndEnd( ScheduleDateTime &start, ScheduleDateTime &end );
};

class ScheduleTimeIterator
{
    private:

    protected:
        time_iterator *ti;

    public:

        ScheduleTimeIterator();
       ~ScheduleTimeIterator();

        void setFromStartAndInterval( ScheduleDateTime &start, ScheduleTimeDuration &interval );
};

class ScheduleAction
{
    private:

    public:

        ScheduleAction();
       ~ScheduleAction();

        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );
};

class ZoneAction : public ScheduleAction
{
    private:
        
        std::vector<Zone *> zoneList;

    public:

        ZoneAction();
       ~ZoneAction();

        void clearZoneList();
        void addZone( Zone *zone );

        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );
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
       
        ScheduleAction    *actionObj;

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

        void setAction( ScheduleAction *action );

        void setReady();
        SESTATE getState();

        bool processCurrent( ScheduleDateTime &curTime );
        void processFinal();

        bool isComplete();
        void setComplete();

        bool isRecyclable();
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

class ScheduleZoneRule : public ScheduleAction
{
    private:
        Zone                 *zone;
        ScheduleTimeDuration  duration;

    public:
        ScheduleZoneRule();
       ~ScheduleZoneRule();

        void setZone( Zone *zone );
        Zone *getZone();
        std::string getZoneID();

        void setDuration( ScheduleTimeDuration &td );
        ScheduleTimeDuration getDuration();
        
        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );
};

typedef enum ScheduleEventRuleZoneEventPolicy 
{
    SER_ZEP_NOTSET     = 0,
    SER_ZEP_SEQUENCIAL = 1,
}SER_ZONE_EVENT_POLICY;

class ScheduleZoneGroup
{
    private:
        std::string id;

        std::vector< ScheduleZoneRule > zoneList;

        SER_ZONE_EVENT_POLICY zonePolicy;

    public:
        ScheduleZoneGroup();
       ~ScheduleZoneGroup();

        void setID( std::string idValue );
        std::string getID();        

        void setZoneEventPolicy( SER_ZONE_EVENT_POLICY eventPolicy );
        SER_ZONE_EVENT_POLICY getZoneEventPolicy();

        void clearZoneList();
        void addZone( Zone *zone );
        void setZoneDuration( std::string zoneID, ScheduleTimeDuration &td );

        unsigned int getZoneRuleCount();
        ScheduleZoneRule *getZoneRuleByIndex( unsigned int index );
        ScheduleZoneRule *getZoneRuleByID( std::string ruleID );

        void createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );
};

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
        std::string      name;
        std::string      desc;

        std::string      url;

        //std::vector<ScheduleRecurrenceRule> recurrenceList;
        //std::vector<ScheduleZoneRule> zoneRuleList;
        ScheduleZoneGroup *zoneGroup;
        ScheduleDateTime  refTime;


        bool fireManually;
        bool eventsPending;

        //void createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

    public:
        ScheduleEventRule();
       ~ScheduleEventRule();

        void setEnabled();
        void clearEnabled();
        bool getEnabled();

        void startManually();

        void setID( std::string idValue );
        std::string getID();

        void setName( std::string nameValue );
        std::string getName();
    
        void setDescription( std::string descValue );
        std::string getDescription();
    
        void setURL( std::string urlValue );
        std::string getURL();

        void setOcurrenceType( SER_OCURRENCE_TYPE typeValue );
        SER_OCURRENCE_TYPE getOcurrenceType();

        void setReferenceTime( ScheduleDateTime &time );
        void getReferenceTime( ScheduleDateTime &time );

        //void clearZoneList();
        //void addZoneRule( Zone *zone, ScheduleTimeDuration &td );

        void setZoneGroup( ScheduleZoneGroup *zg );

        void updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

};

class ScheduleManager
{
    private:
        std::string cfgPath;

        std::vector<ScheduleEventRule *> eventRuleList;

        ScheduleEventList activeEvents;

        ZoneManager *zoneMgr;
        std::vector<ScheduleZoneGroup *> zoneGroupList;

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool getChildContent( xmlNode *elem, std::string childName, std::string &result );
        bool parseActionList( xmlDocPtr doc, xmlNode *ruleElem, ScheduleEventRule *ruleObj );
        bool addRule( xmlDocPtr doc, xmlNode *ruleElem );
        bool addZoneGroup( xmlDocPtr doc, xmlNode *zgElem );
        bool parseZoneRuleList( xmlDocPtr doc, xmlNode *zgElem, ScheduleZoneGroup *zgObj );

    public:
        ScheduleManager();
       ~ScheduleManager();

        void setZoneManager( ZoneManager *zoneMgr );

        unsigned int getZoneGroupCount();
        ScheduleZoneGroup *getZoneGroupByIndex( unsigned int index );
        ScheduleZoneGroup *getZoneGroupByID( std::string zgID );

        unsigned int getEventRuleCount();
        ScheduleEventRule *getEventRuleByIndex( unsigned int index );
        ScheduleEventRule *getEventRuleByID( std::string erID );

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        void processCurrentEvents( ScheduleDateTime &curTime );

        ScheduleEventList *getActiveEvents();
        ScheduleEventList *getEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime );

        void freeScheduleEventList( ScheduleEventList *listPtr );
};

#endif // __SCHEDULE_MANAGER_H__

