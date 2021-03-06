#ifndef __SCHEDULE_MANAGER_H__
#define __SCHEDULE_MANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <exception>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" 
#include "boost/date_time/local_time/local_time.hpp"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "REST/REST.hpp"

#include "ZoneManager.hpp"

using namespace boost::posix_time;
using namespace boost::local_time;
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
    friend class ScheduleLocalDateTime;

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

        long getSecondOfMinute();
        long getMinuteOfHour();
        long getHourOfDay();
        long getDayOfWeek();
        long getWeekNumber();

        void addSeconds( long seconds );
        void addMinutes( long minutes );
        void addHours( long hours );
        void addDays( long days );
        void addWeeks( long weeks );

        void subSeconds( long seconds );
        void subMinutes( long minutes );
        void subHours( long hours );
        void subDays( long days );

        void replaceSecondScope( ScheduleDateTime &time );
        void replaceMinuteScope( ScheduleDateTime &time );
        void replaceHourScope( ScheduleDateTime &time );

        void advanceToMatchingWeekDay( ScheduleDateTime &time );

        void retreatToStartOfDay();
        void advanceToEndOfDay();

        bool isBefore( ScheduleDateTime &time );
        bool isAfter( ScheduleDateTime &time ); 

        bool isSameDay( ScheduleDateTime &time );
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

        void setFromSeconds( unsigned long seconds );

        void setFromString( std::string timeStr );

        void setFromTimeDifference( ScheduleDateTime &start, ScheduleDateTime &end );

        std::string getISOString();
        std::string getSecondsString();
        std::string getAsHMSString();

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

class ScheduleTimezone
{
    friend class ScheduleLocalDateTime;

    private:

    protected:
        std::string olsenZone;
        std::string posixStr;
 
        time_zone_ptr ptz;

    public:

        ScheduleTimezone();
       ~ScheduleTimezone();

        void initFromSystemFiles();

        std::string getOlsenStr();
        std::string getPosixStr();
};

class ScheduleLocalDateTime
{
    
    protected:
        ScheduleTimezone  tzObj;
        ScheduleDateTime  utcObj;

    public:

        ScheduleLocalDateTime();
        ScheduleLocalDateTime( ScheduleDateTime &timestamp );
        ScheduleLocalDateTime( ScheduleDateTime &timestamp, ScheduleTimezone &timezone );

       ~ScheduleLocalDateTime();

        void setTimezone( ScheduleTimezone &timezone );
        void setTime( ScheduleDateTime &time );

        void setFromCurrentSystemTime();

        ScheduleDateTime getUTCTime();

        std::string getSimpleString();
        std::string getISOString();
        std::string getExtendedISOString();

        void retreatToStartOfDay();
        void advanceToEndOfDay();
};

class SMException : public std::exception
{
    private:
        unsigned long eCode;
        std::string eMsg;

    public:
        SMException( unsigned long errCode, std::string errMsg )
        {
            eCode = errCode;
            eMsg  = errMsg;
        }

       ~SMException() throw() {};

        virtual const char* what() const throw()
        {
            return eMsg.c_str();
        }

        unsigned long getErrorCode() const throw()
        {
            return eCode;
        }

        std::string getErrorMsg() const throw()
        {
            return eMsg;
        }
};

class ScheduleEventLogEntry
{
    private:
        ScheduleDateTime  timestamp;
        unsigned long     seqNum;
        std::string       id;
        std::string       msg;
        RESTContentNode   extra;

    public:
        ScheduleEventLogEntry();
       ~ScheduleEventLogEntry();

        void setEvent( unsigned long seqNumber, std::string eventID, std::string eventMsg );
        void setEvent( unsigned long seqNumber, std::string eventID, std::string eventMsg, RESTContentNode &eventData );
        
        bool onThisDay( ScheduleDateTime &targetDay );
        bool isBetween( ScheduleDateTime startTime, ScheduleDateTime endTime );

        std::string   getTimestampAsStr();
        unsigned long getSequenceNumber();
        std::string   getEventID();
        std::string   getEventMsg();

        void setContentNodeFromFields( RESTContentNode *objCN );
};

class ScheduleEventLog
{
    private:
        unsigned long maxSize;

        std::list< ScheduleEventLogEntry > logData;

        unsigned long nextSeqNumber;

    public:
        ScheduleEventLog( unsigned long maxEntries );
       ~ScheduleEventLog();

        void populateContentNode( RESTContentNode *rootNode );

        void populateTodaysEventsNode( RESTContentNode *rootNode, ScheduleDateTime &targetDay );

        void addLogEntry( std::string eventID, std::string eventMsg );
        void addLogEntry( std::string eventID, std::string eventMsg, RESTContentNode &eventData );
};

typedef enum ScheduleRestObjectTemplateID
{
    SCH_ROTID_ROOT         = 0,
    SCH_ROTID_ZONEGROUP    = 1,
    SCH_ROTID_ZONERULE     = 2,
    SCH_ROTID_TRIGGERGROUP = 3,
    SCH_ROTID_TRIGGERRULE  = 4,
    SCH_ROTID_EVENTRULE    = 5
}SCH_ROTID_T;

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

#if 0
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
#endif

class ScheduleTriggerRecord
{
    private:
        std::string  eventRuleID;
        std::string  groupID;
        std::string  ruleID;
        std::string  triggerName;

        ScheduleDateTime eventTime;

        bool             manual;

    public:
        ScheduleTriggerRecord();
       ~ScheduleTriggerRecord();

        void setManualTrigger();
        void clearManualTrigger();
        bool wasManuallyTriggered();

        void setERID( std::string idStr );
        void setGroupID( std::string idStr );
        void setRuleID( std::string idStr );
        void setTriggerName( std::string nameStr );
        void setEventTime( ScheduleDateTime &evTime );
       
        std::string getERID();
        std::string getGroupID();
        std::string getRuleID();
        std::string getTriggerName();
        ScheduleDateTime &getEventTime();

};

class ScheduleZoneRecord
{
    private:
        std::string  eventRuleID;
        std::string  groupID;
        std::string  ruleID;
        std::string  zoneName;  

    public:
        ScheduleZoneRecord();
       ~ScheduleZoneRecord();

        void setERID( std::string idStr );
        void setGroupID( std::string idStr );
        void setRuleID( std::string idStr );
        void setZoneName( std::string nameStr );

        std::string getERID();
        std::string getGroupID();
        std::string getRuleID();
        std::string getZoneName();
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
        SESTATE            state;

        ScheduleTriggerRecord tRecord;
        ScheduleZoneRecord    zRecord;

        ScheduleDateTime   start;
        ScheduleDateTime   end;
        ScheduleDateTime   recycle;
       
        ScheduleAction    *actionObj;

    public:
        ScheduleEvent();
       ~ScheduleEvent();

        std::string getId();
        std::string getDescription();
        std::string getDurationStr();

        void setTriggerRecord( ScheduleTriggerRecord &tRec );
        ScheduleTriggerRecord &getTriggerRecord();

        void setZoneRecord( ScheduleZoneRecord &tRec );
        ScheduleZoneRecord &getZoneRecord();

        void setStartTime( ScheduleDateTime &time );
        void getStartTime( ScheduleDateTime &time );

        void setEndTime( ScheduleDateTime &time );
        void getEndTime( ScheduleDateTime &time );

        void setAction( ScheduleAction *action );

        void setReady();
        SESTATE getState();

        bool processCurrent( ScheduleDateTime &curTime, ScheduleEventLog &log );
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

        void sortAscendingByStartTime();

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

typedef enum ScheduleZoneRuleType
{
    SZR_TYPE_NOTSET = 0,
    SZR_TYPE_FIXED_DURATION = 1
}SZR_TYPE_T;

class ScheduleZoneRule : public RESTContentNode, public ScheduleAction
{
    private:
        RESTContentManager     &objManager;

        SZR_TYPE_T              ruleType;

        std::string             zoneID;
        ScheduleTimeDuration    duration;

    public:
        ScheduleZoneRule( RESTContentManager &objMgr );
       ~ScheduleZoneRule();

        virtual SZR_TYPE_T getType();       
        virtual std::string getTypeStr();       
        static  std::string getStaticTypeStr();

        static std::string getElementName();

        void setName( std::string idValue );
        std::string getName();

        void setZoneID( std::string idStr );
        std::string getZoneID();
        std::string getZoneName();

        void setDuration( ScheduleTimeDuration &td );
        ScheduleTimeDuration getDuration();   
       
        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN ); 
        virtual void setContentNodeFromFields( RESTContentNode *objCN );
};

typedef enum ScheduleEventRuleZoneEventPolicy 
{
    SER_ZEP_NOTSET     = 0,
    SER_ZEP_SEQUENCIAL = 1,
}SER_ZONE_EVENT_POLICY;

class ScheduleZoneGroup : public RESTContentNode
{
    private:
        RESTContentManager &objManager;

        std::string name;

        SER_ZONE_EVENT_POLICY zonePolicy;

    public:
        ScheduleZoneGroup( RESTContentManager &objMgr );
       ~ScheduleZoneGroup();      

        void setName( std::string value );
        std::string getName();

        void setZoneEventPolicy( SER_ZONE_EVENT_POLICY eventPolicy );
        void setZoneEventPolicyFromStr( std::string value );

        SER_ZONE_EVENT_POLICY getZoneEventPolicy();
        std::string getZoneEventPolicyStr();

        void setZoneDuration( std::string zoneID, ScheduleTimeDuration &td );

//        unsigned int getZoneRuleCount();
//        ScheduleZoneRule *getZoneRuleByIndex( unsigned int index );
//        ScheduleZoneRule *getZoneRuleByID( std::string ruleID );

        void createZoneEvents( ScheduleEventList &activeEvents, ScheduleTriggerRecord &tRecord, bool serializeEvents, ScheduleDateTime &rearmTime );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );
};



typedef enum ScheduleTriggerRuleType
{
    STR_TYPE_NOTSET = 0,
    STR_TYPE_TIME   = 1
}STR_TYPE_T;

typedef enum ScheduleTimeTriggerScope 
{
    SER_TT_SCOPE_NOTSET     = 0,
    SER_TT_SCOPE_NEVER      = 1,
    SER_TT_SCOPE_MINUTE     = 2,
    SER_TT_SCOPE_HOUR       = 3,
    SER_TT_SCOPE_DAY        = 4,
    SER_TT_SCOPE_WEEK       = 5,
    SER_TT_SCOPE_EVEN_WEEK  = 6,
    SER_TT_SCOPE_ODD_WEEK   = 7,
    SER_TT_SCOPE_YEAR       = 8
}SER_TT_SCOPE;

static const char* SERScopeString[] =
{
    "notset",    // TRS_REPEAT_NOTSET,
    "none",      // TRS_REPEAT_NEVER,
    "minute",    // TRS_REPEAT_MINUTE,
    "hour",      // TRS_REPEAT_HOUR,
    "day",       // TRS_REPEAT_DAY,
    "week",      // TRS_REPEAT_WEEK,
    "even_week", // TRS_REPEAT_EVEN_WEEK,
    "odd_week",  // TRS_REPEAT_ODD_WEEK,
    "year"       // TRS_REPEAT_YEAR
};

typedef enum ScheduleEventRuleType 
{
    SER_TYPE_NOTSET     = 0,
    SER_TYPE_DAY        = 1,
    SER_TYPE_WEEKLY     = 2,
    SER_TYPE_INTERVAL   = 3,
}SER_OCURRENCE_TYPE;

class ScheduleTriggerRule : public RESTContentNode
{
    private:
        RESTContentManager &objManager;

        STR_TYPE_T         ruleType;

        SER_TT_SCOPE       scope;
        ScheduleDateTime   refTime;

        bool checkForTimeTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord );
        void getPotentialTimeTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList );

    public:
        ScheduleTriggerRule( RESTContentManager &objMgr );
       ~ScheduleTriggerRule();

        virtual STR_TYPE_T getType();       
        virtual std::string getTypeStr();       
        static  std::string getStaticTypeStr();       
   
        void setScope( SER_TT_SCOPE scopeValue );
        SER_TT_SCOPE getScope();

        bool setScopeFromStr( std::string scopeStr );
        std::string getScopeStr();        

        void setRefTime( ScheduleDateTime &refTimeValue );
        ScheduleDateTime getRefTime();        

        bool checkForTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord ); //ScheduleDateTime &eventTime, std::string &triggerRuleID );
        void getPotentialTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList );

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();
};

// Manage a set of triggers to cause the event to fire.
class ScheduleTriggerGroup : public RESTContentNode
{
    private:
        RESTContentManager &objManager;

        std::string name;
        std::string desc;

    public:
        ScheduleTriggerGroup( RESTContentManager &objMgr);
       ~ScheduleTriggerGroup();
 
        void setName( std::string idValue );
        std::string getName(); 

        void setDescription( std::string idValue );
        std::string getDescription(); 

//        unsigned int getTriggerRuleCount();
//        ScheduleTriggerRule *getTriggerRuleByIndex( unsigned int index );
//        ScheduleTriggerRule *getTriggerRuleByID( std::string ruleID );

        bool checkForTrigger( ScheduleDateTime &curTime, ScheduleTriggerRecord &tRecord ); //ScheduleDateTime &eventTime );
        void getPotentialTriggersForPeriod( ScheduleDateTime &startTime, ScheduleDateTime &endTime, std::vector< ScheduleTriggerRecord > &tList );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );
};

class ScheduleEventRule : public RESTContentNode
{
    private:

        RESTContentManager &objManager;

        bool enabled;

        std::string      name;
        std::string      desc;

        std::string      url;

        std::string      zoneGroupID;
        std::string      triggerGroupID;

        bool fireManually;

        bool eventsPending;
        ScheduleDateTime rearmTime;

    public:
        ScheduleEventRule( RESTContentManager &objMgr );
       ~ScheduleEventRule();

        void setEnabled();
        void clearEnabled();
        bool getEnabled();

        void startManually();

        void setName( std::string nameValue );
        std::string getName();
    
        void setDescription( std::string descValue );
        std::string getDescription();
    
        void setURL( std::string urlValue );
        std::string getURL();

        void setZoneGroupID( std::string zgID );
        std::string getZoneGroupID();

        void setTriggerGroupID( std::string tgID );
        std::string getTriggerGroupID();
 
        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN ); 
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        void updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime, bool serializeEvents );
        void getPotentialEventsForPeriod( ScheduleEventList &activeEvents, ScheduleDateTime startTime, ScheduleDateTime endTime );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

};

typedef enum ScheduleManagerStatusProviderIDs
{
    SCHRSRC_STATID_STATUS,
    SCHRSRC_STATID_EVENTLOG,
    SCHRSRC_STATID_CALENDAR
}SCHRSCRC_STATID_T;

class ScheduleManager : public RESTContentManager
{
    private:
        std::string cfgPath;

        unsigned long nextID;

        bool masterEnable;

        bool serializeEvents;

        ScheduleTimezone localtz;

        ScheduleEventLog  eventLog;

        ScheduleEventList activeEvents;

        ZoneManager *zoneMgr;

        virtual RESTContentNode* newObject( unsigned int type );
        virtual void freeObject( RESTContentNode *objPtr );

        virtual unsigned int getTypeFromObjectElementName( std::string name );
        virtual RESTContentTemplate *getContentTemplateForType( unsigned int type );

        virtual void notifyCfgChange();

        virtual void populateContentNodeFromStatusProvider( unsigned int id, RESTContentNode *outNode, std::map< std::string, std::string > paramMap );

    public:
        ScheduleManager();
       ~ScheduleManager();

        void setZoneManager( ZoneManager *zoneMgr );

        std::string getZoneName( std::string zoneID );

        void setZoneStateOn( std::string zoneID );
        void setZoneStateOff( std::string zoneID );

        unsigned int getZoneGroupCount();
        ScheduleZoneGroup *getZoneGroupByIndex( unsigned int index );
        ScheduleZoneGroup *getZoneGroupByID( std::string zgID );

        unsigned int getTriggerGroupCount();
        ScheduleTriggerGroup *getTriggerGroupByIndex( unsigned int index );
        ScheduleTriggerGroup *getTriggerGroupByID( std::string tgID );

        unsigned int getEventRuleCount();
        ScheduleEventRule *getEventRuleByIndex( unsigned int index );
        ScheduleEventRule *getEventRuleByID( std::string erID );

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();
        bool saveConfiguration();

        void processCurrentEvents( ScheduleDateTime &curTime );

        ScheduleEventList *getActiveEvents();
        ScheduleEventList *getPotentialEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime );

        void freeScheduleEventList( ScheduleEventList *listPtr );

};

#endif // __SCHEDULE_MANAGER_H__

