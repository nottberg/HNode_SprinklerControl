#ifndef __SCHEDULE_MANAGER_H__
#define __SCHEDULE_MANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <exception>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" 

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "REST/REST.hpp"

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
        std::string getISOString();
  
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
        std::string           id;
        Zone                 *zone;
        ScheduleTimeDuration  duration;

    public:
        ScheduleZoneRule();
       ~ScheduleZoneRule();

        void setID( std::string idValue );
        std::string getID();

        void setZone( Zone *zone );
        Zone *getZone();
        std::string getZoneID();

        void setDuration( ScheduleTimeDuration &td );
        ScheduleTimeDuration getDuration();
        
        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );

        static RESTContentNode *generateCreateTemplate();
        static RESTContentNode *generateUpdateTemplate();

        void setFieldsFromContentNode( RESTContentNode *objCN ); 
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
        std::string name;

        std::vector< ScheduleZoneRule * > zoneList;

        SER_ZONE_EVENT_POLICY zonePolicy;

    public:
        ScheduleZoneGroup();
       ~ScheduleZoneGroup();

        void setID( std::string idValue );
        std::string getID();        

        void setZoneEventPolicy( SER_ZONE_EVENT_POLICY eventPolicy );
        void setZoneEventPolicyFromStr( std::string value );

        SER_ZONE_EVENT_POLICY getZoneEventPolicy();
        std::string getZoneEventPolicyStr();

        void clearZoneList();
        void addZoneRule( ScheduleZoneRule *zoneRule );
        void updateZoneRule( std::string ruleID, RESTContentNode *objCN );
        void deleteZoneRule( std::string ruleID );

        void addZone( Zone *zone );
        void setZoneDuration( std::string zoneID, ScheduleTimeDuration &td );
        void removeZone( std::string zoneID );

        unsigned int getZoneRuleCount();
        ScheduleZoneRule *getZoneRuleByIndex( unsigned int index );
        ScheduleZoneRule *getZoneRuleByID( std::string ruleID );

        void createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime, ScheduleDateTime &rearmTime );

        static RESTContentNode *generateCreateTemplate();
        static RESTContentNode *generateUpdateTemplate();

        void setFieldsFromContentNode( RESTContentNode *objCN ); 
};

typedef enum ScheduleTimeTriggerScope 
{
    SER_TT_SCOPE_NOTSET     = 0,
    SER_TT_SCOPE_DAY        = 1,
}SER_TT_SCOPE;

typedef enum ScheduleEventRuleType 
{
    SER_TYPE_NOTSET     = 0,
    SER_TYPE_DAY        = 1,
    SER_TYPE_WEEKLY     = 2,
    SER_TYPE_INTERVAL   = 3,
}SER_OCURRENCE_TYPE;

// A time based trigger to cause a schedule rule to be 
// evaluated
class ScheduleTimeTrigger
{
    private:
        std::string        id;

        SER_TT_SCOPE       scope;
        ScheduleDateTime   refTime;

    public:
        ScheduleTimeTrigger();
       ~ScheduleTimeTrigger();

        void setID( std::string idValue );
        std::string getID();        

        void setScope( SER_TT_SCOPE scopeValue );
        SER_TT_SCOPE getScope();
        std::string getScopeStr();        

        void setRefTime( ScheduleDateTime &refTimeValue );
        ScheduleDateTime getRefTime();        
};

// Manage a set of triggers to cause the event to fire.
class ScheduleTriggerGroup
{
    private:
        std::string id;
        std::string name;

        std::vector< ScheduleTimeTrigger > timeList;

    public:
        ScheduleTriggerGroup();
       ~ScheduleTriggerGroup();

        void setID( std::string idValue );
        std::string getID();        

        void setName( std::string idValue );
        std::string getName(); 

        void clearTimeTriggerList();
        void addTimeTrigger( std::string id, SER_TT_SCOPE scope, ScheduleDateTime &refTime );
        void removeTimeTrigger( std::string id );

        unsigned int getTimeTriggerCount();
        ScheduleTimeTrigger *getTimeTriggerByIndex( unsigned int index );
        ScheduleTimeTrigger *getTimeTriggerByID( std::string ruleID );

        bool checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime );

        static RESTContentNode *generateCreateTemplate();
        static RESTContentNode *generateUpdateTemplate();

        void setFieldsFromContentNode( RESTContentNode *objCN );
};

class ScheduleEventRule
{
    private:

        bool enabled;

        //SER_OCURRENCE_TYPE type;

        std::string      id;
        std::string      name;
        std::string      desc;

        std::string      url;

        //std::vector<ScheduleRecurrenceRule> recurrenceList;
        //std::vector<ScheduleZoneRule> zoneRuleList;
        ScheduleZoneGroup *zoneGroup;
        //ScheduleDateTime  refTime;
        ScheduleTriggerGroup *triggerGroup;

        bool fireManually;

        bool eventsPending;
        ScheduleDateTime rearmTime;

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

        //void setOcurrenceType( SER_OCURRENCE_TYPE typeValue );
        //SER_OCURRENCE_TYPE getOcurrenceType();

        //void setReferenceTime( ScheduleDateTime &time );
        //void getReferenceTime( ScheduleDateTime &time );

        //void clearZoneList();
        //void addZoneRule( Zone *zone, ScheduleTimeDuration &td );

        void setZoneGroup( ScheduleZoneGroup *zg );
        std::string getZoneGroupID();

        void setTriggerGroup( ScheduleTriggerGroup *tg );
        std::string getTriggerGroupID();
 
        void setFieldsFromContentNode( RESTContentNode *objCN ); 

        void updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

        //static void initRequiredFieldMap( std::map< std::string, std::string > &fieldMap );
        //static void initOptionalFieldMap( std::map< std::string, std::string > &fieldMap );
        //static void initUpdateFieldMap( std::map< std::string, std::string > &fieldMap );

        static RESTContentNode *generateCreateTemplate();
        static RESTContentNode *generateUpdateTemplate();

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

class ScheduleManager
{
    private:
        std::string cfgPath;
        xmlTextWriterPtr writer;

        unsigned long nextID;

        std::vector<ScheduleEventRule *> eventRuleList;

        ScheduleEventList activeEvents;

        ZoneManager *zoneMgr;
        std::vector<ScheduleZoneGroup *> zoneGroupList;

        std::vector<ScheduleTriggerGroup *> triggerGroupList;

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool getChildContent( xmlNode *elem, std::string childName, std::string &result );
        bool parseActionList( xmlDocPtr doc, xmlNode *ruleElem, ScheduleEventRule *ruleObj );
        bool addRule( xmlDocPtr doc, xmlNode *ruleElem );
        bool addZoneGroup( xmlDocPtr doc, xmlNode *zgElem );
        bool parseZoneRuleList( xmlDocPtr doc, xmlNode *zgElem, ScheduleZoneGroup *zgObj );
        bool addTriggerGroup( xmlDocPtr doc, xmlNode *tgElem );

        bool saveZoneGroup( ScheduleZoneGroup *zgObj );
        bool saveZoneGroupList();
        bool saveTriggerGroup( ScheduleTriggerGroup *tgObj );
        bool saveTriggerGroupList();
        bool saveEventRule( ScheduleEventRule *erObj );
        bool saveEventRuleList();

    public:
        ScheduleManager();
       ~ScheduleManager();

        void setZoneManager( ZoneManager *zoneMgr );

        ScheduleZoneGroup *createNewZoneGroup();
        void freeNewZoneGroup( ScheduleZoneGroup *grpObj );
        void addNewZoneGroup( RESTContentNode *rootCN, ScheduleZoneGroup **newEvent );
        void updateZoneGroup( std::string zgID, RESTContentNode *rootCN );
        void deleteZoneGroup( std::string zgID );

        ScheduleZoneRule *createNewZoneRule();
        void freeNewZoneRule( ScheduleZoneRule *ruleObj );
        void addNewZoneRule( std::string zgID, RESTContentNode *rootCN, ScheduleZoneRule **newEvent );
        void updateZoneRule( std::string zgID, std::string ruleID, RESTContentNode *rootCN );
        void deleteZoneRule( std::string zgID, std::string ruleID );

        unsigned int getZoneGroupCount();
        ScheduleZoneGroup *getZoneGroupByIndex( unsigned int index );
        ScheduleZoneGroup *getZoneGroupByID( std::string zgID );

        void generateZoneGroupListContent( RESTContentNode *rootCN ); 
        void generateZoneGroupContent( std::string zoneGroupID, RESTContentNode *rootCN ); 
        void generateZoneGroupRuleListContent( std::string zoneGroupID, RESTContentNode *rootCN ); 
        void generateZoneGroupRuleContent( std::string zoneGroupID, std::string ruleID, RESTContentNode *rootCN ); 

        ScheduleTriggerGroup *createNewTriggerGroup();
        void freeNewTriggerGroup( ScheduleTriggerGroup *ruleObj );
        void addNewTriggerGroup( RESTContentNode *rootCN, ScheduleTriggerGroup **newEvent );
        void updateTriggerGroup( std::string zgID, RESTContentNode *rootCN );
        void deleteTriggerGroup( std::string zgID );

        unsigned int getTriggerGroupCount();
        ScheduleTriggerGroup *getTriggerGroupByIndex( unsigned int index );
        ScheduleTriggerGroup *getTriggerGroupByID( std::string tgID );

        void generateTriggerGroupListContent( RESTContentNode *rootCN ); 
        void generateTriggerGroupContent( std::string ruleID, RESTContentNode *rootCN ); 

        unsigned int getEventRuleCount();
        ScheduleEventRule *getEventRuleByIndex( unsigned int index );
        ScheduleEventRule *getEventRuleByID( std::string erID );

        ScheduleEventRule *createNewEventRule();
        void freeNewEventRule( ScheduleEventRule *ruleObj );
        void addNewEventRule( ScheduleEventRule *event );
        void addNewEventRule( RESTContentNode *rootCN, ScheduleEventRule **newEvent );
        void updateEventRule( std::string erID, RESTContentNode *rootCN );

        void generateScheduleRuleListContent( RESTContentNode *rootCN ); 
        void generateEventRuleContent( std::string ruleID, RESTContentNode *rootCN ); 

        void deleteEventRuleByID( std::string erID );

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();
        bool saveConfiguration();

        void processCurrentEvents( ScheduleDateTime &curTime );

        ScheduleEventList *getActiveEvents();
        ScheduleEventList *getEventsForPeriod( ScheduleDateTime startTime, ScheduleDateTime endTime );

        void freeScheduleEventList( ScheduleEventList *listPtr );
};

#endif // __SCHEDULE_MANAGER_H__

