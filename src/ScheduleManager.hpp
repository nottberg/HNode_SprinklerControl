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

typedef enum ScheduleZoneRuleType
{
    SZR_TYPE_NOTSET = 0,
    SZR_TYPE_FIXED_DURATION = 1
}SZR_TYPE_T;

class ScheduleZoneRule : public RESTContentNode, public ScheduleAction
{
    private:
        RESTContentManager     &objManager;
    
        //std::string             id;
        std::string             name;

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

        //void setID( std::string idValue );
        //std::string getID();

        void setName( std::string idValue );
        std::string getName();

        void setZoneID( std::string idStr );
        std::string getZoneID();

        void setDuration( ScheduleTimeDuration &td );
        ScheduleTimeDuration getDuration();   
       
        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN ); 
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        virtual bool buildRCNodeTree( RESTContentNode *rootCN );
        static bool buildRCTemplateTree( RESTContentTemplate *rootCN );
};

#if 0
// Run the associated zone for a fixed duration.
class SZRFixedDuration : public ScheduleZoneRule
{
    private:
        Zone                 *zone;
        ScheduleTimeDuration  duration;

    public:
        SZRFixedDuration( RESTContentManager &objMgr );
       ~SZRFixedDuration();

        virtual SZR_TYPE_T getType();       
        virtual std::string getTypeStr();       
        static  std::string getStaticTypeStr();

        void setZone( Zone *zone );
        Zone *getZone();
        std::string getZoneID();

        void setDuration( ScheduleTimeDuration &td );
        ScheduleTimeDuration getDuration();        

        virtual void start( ScheduleDateTime &curTime );
        virtual void poll( ScheduleDateTime &curTime );
        virtual void complete( ScheduleDateTime &curTime );

        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        virtual bool buildRCNodeTree( RESTContentNode *rootCN );
};
#endif

typedef enum ScheduleEventRuleZoneEventPolicy 
{
    SER_ZEP_NOTSET     = 0,
    SER_ZEP_SEQUENCIAL = 1,
}SER_ZONE_EVENT_POLICY;

class ScheduleZoneGroup : public RESTContentNode
{
    private:
        RESTContentManager &objManager;

        //std::string id;
        std::string name;

        //std::vector< RESTContentRef > zoneList;

        SER_ZONE_EVENT_POLICY zonePolicy;

    public:
        ScheduleZoneGroup( RESTContentManager &objMgr );
       ~ScheduleZoneGroup();

        //void setID( std::string idValue );
        //std::string getID();        

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

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        bool buildRCNodeTree( RESTContentNode *rootCN );
        static bool buildRCTemplateTree( RESTContentTemplate *rootCN );
};

typedef enum ScheduleTriggerRuleType
{
    STR_TYPE_NOTSET = 0,
    STR_TYPE_TIME   = 1
}STR_TYPE_T;

typedef enum ScheduleTimeTriggerScope 
{
    SER_TT_SCOPE_NOTSET     = 0,
    SER_TT_SCOPE_DAY        = 1,
}SER_TT_SCOPE;

static const char* SERScopeString[] =
{
    "notset",  // 0
    "day"     // 1
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

        STR_TYPE_T     ruleType;

        //std::string    id;
        std::string    name;

        SER_TT_SCOPE       scope;
        ScheduleDateTime   refTime;

    public:
        ScheduleTriggerRule( RESTContentManager &objMgr );
       ~ScheduleTriggerRule();

        virtual STR_TYPE_T getType();       
        virtual std::string getTypeStr();       
        static  std::string getStaticTypeStr();

        //void setID( std::string idValue );
        //std::string getID();        
   
        void setName( std::string idValue );
        std::string getName();

        void setScope( SER_TT_SCOPE scopeValue );
        SER_TT_SCOPE getScope();

        bool setScopeFromStr( std::string scopeStr );
        std::string getScopeStr();        

        void setRefTime( ScheduleDateTime &refTimeValue );
        ScheduleDateTime getRefTime();        

        virtual bool checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime );

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

        virtual bool buildRCNodeTree( RESTContentNode *rootCN );
        static bool buildRCTemplateTree( RESTContentTemplate *rootCN );
};

#if 0
// A time based trigger to cause a schedule rule to be 
// evaluated
class ScheduleTimeTrigger : public ScheduleTriggerRule
{
    private:
        //std::string        id;

        SER_TT_SCOPE       scope;
        ScheduleDateTime   refTime;

    public:
        ScheduleTimeTrigger( RESTContentManager &objMgr );
       ~ScheduleTimeTrigger();

        virtual STR_TYPE_T getType();       
        virtual std::string getTypeStr();       
        static  std::string getStaticTypeStr();

        void setScope( SER_TT_SCOPE scopeValue );
        SER_TT_SCOPE getScope();

        bool setScopeFromStr( std::string scopeStr );
        std::string getScopeStr();        

        void setRefTime( ScheduleDateTime &refTimeValue );
        ScheduleDateTime getRefTime();        

        virtual bool checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime );

        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        virtual bool buildRCNodeTree( RESTContentNode *rootCN );
};
#endif

// Manage a set of triggers to cause the event to fire.
class ScheduleTriggerGroup : public RESTContentNode
{
    private:
        RESTContentManager &objManager;

        //std::string id;
        std::string name;

        //std::vector< RESTContentRef > ruleList;

    public:
        ScheduleTriggerGroup( RESTContentManager &objMgr);
       ~ScheduleTriggerGroup();

        //void setID( std::string idValue );
        //std::string getID();        

        void setName( std::string idValue );
        std::string getName(); 

        void clearTriggerRuleList();
        void addTriggerRule( ScheduleTriggerRule *newTrigger );
        void updateTriggerRule( std::string ruleID, RESTContentNode *objCN );
        void deleteTriggerRule( std::string id );

        unsigned int getTriggerRuleCount();
        ScheduleTriggerRule *getTriggerRuleByIndex( unsigned int index );
        ScheduleTriggerRule *getTriggerRuleByID( std::string ruleID );

        bool checkForTrigger( ScheduleDateTime &curTime, ScheduleDateTime &eventTime );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();

        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN );
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        bool buildRCNodeTree( RESTContentNode *rootCN );
        static bool buildRCTemplateTree( RESTContentTemplate *rootCN );

};

class ScheduleEventRule : public RESTContentNode
{
    private:

        RESTContentManager &objManager;

        bool enabled;

        //SER_OCURRENCE_TYPE type;

        //std::string      id;
        std::string      name;
        std::string      desc;

        std::string      url;

        //std::vector<ScheduleRecurrenceRule> recurrenceList;
        //std::vector<ScheduleZoneRule> zoneRuleList;
        //ScheduleZoneGroup *zoneGroup;
        //RESTContentRef zoneRef;

        //ScheduleDateTime  refTime;
        //ScheduleTriggerGroup *triggerGroup;
        //RESTContentRef triggerRef;

        bool fireManually;

        bool eventsPending;
        ScheduleDateTime rearmTime;

        //void createZoneEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

    public:
        ScheduleEventRule( RESTContentManager &objMgr );
       ~ScheduleEventRule();

        void setEnabled();
        void clearEnabled();
        bool getEnabled();

        void startManually();

        //void setID( std::string idValue );
        //std::string getID();

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
 
        virtual unsigned int getObjType();
        virtual void setFieldsFromContentNode( RESTContentNode *objCN ); 
        virtual void setContentNodeFromFields( RESTContentNode *objCN );

        bool buildRCNodeTree( RESTContentNode *rootCN );

        void updateActiveEvents( ScheduleEventList &activeEvents, ScheduleDateTime &curTime );

        //static void initRequiredFieldMap( std::map< std::string, std::string > &fieldMap );
        //static void initOptionalFieldMap( std::map< std::string, std::string > &fieldMap );
        //static void initUpdateFieldMap( std::map< std::string, std::string > &fieldMap );

        static std::string getElementName();

        static RESTContentTemplate *generateContentTemplate();
        static bool buildRCTemplateTree( RESTContentTemplate *rootCN );
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

class ScheduleManager : public RESTContentManager, RESTContentObjectCallback, RESTContentListCallback
{
    private:
        std::string cfgPath;
        xmlTextWriterPtr writer;

        unsigned long nextID;

        //std::vector<ScheduleEventRule *> eventRuleList;

        ScheduleEventList activeEvents;

        ZoneManager *zoneMgr;
        //std::vector<ScheduleZoneGroup *> zoneGroupList;

        //std::vector<ScheduleTriggerGroup *> triggerGroupList;

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

        bool saveConfigNew();
        bool loadConfigNew();

        virtual RESTContentNode* newObject( unsigned int type );
        virtual void freeObject( RESTContentNode *objPtr );

        virtual unsigned int getTypeFromObjectElementName( std::string name );
        virtual RESTContentTemplate *getContentTemplateForType( unsigned int type );

        virtual void notifyCfgChange();

    public:
        ScheduleManager();
       ~ScheduleManager();

        void setZoneManager( ZoneManager *zoneMgr );

#if 0
        ScheduleZoneGroup *createNewZoneGroup();
        void freeNewZoneGroup( ScheduleZoneGroup *grpObj );
        void addNewZoneGroup( RESTContentNode *rootCN, ScheduleZoneGroup **newEvent );
        void updateZoneGroup( std::string zgID, RESTContentNode *rootCN );
        void deleteZoneGroup( std::string zgID );

        ScheduleZoneRule *createNewZoneRule( RESTContentNode *rootCN );
        void freeNewZoneRule( ScheduleZoneRule *ruleObj );
        void addNewZoneRule( std::string zgID, RESTContentNode *rootCN, ScheduleZoneRule **newEvent );
        void updateZoneRule( std::string zgID, std::string ruleID, RESTContentNode *rootCN );
        void deleteZoneRule( std::string zgID, std::string ruleID );
#endif

        unsigned int getZoneGroupCount();
        ScheduleZoneGroup *getZoneGroupByIndex( unsigned int index );
        ScheduleZoneGroup *getZoneGroupByID( std::string zgID );

        void generateZoneGroupListContent( RESTContentNode *rootCN ); 
        void generateZoneGroupContent( std::string zoneGroupID, RESTContentNode *rootCN ); 
        void generateZoneGroupRuleListContent( std::string zoneGroupID, RESTContentNode *rootCN ); 
        void generateZoneGroupRuleContent( std::string zoneGroupID, std::string ruleID, RESTContentNode *rootCN ); 

#if 0
        ScheduleTriggerGroup *createNewTriggerGroup();
        void freeNewTriggerGroup( ScheduleTriggerGroup *ruleObj );
        void addNewTriggerGroup( RESTContentNode *rootCN, ScheduleTriggerGroup **newEvent );
        void updateTriggerGroup( std::string zgID, RESTContentNode *rootCN );
        void deleteTriggerGroup( std::string zgID );
#endif
        unsigned int getTriggerGroupCount();
        ScheduleTriggerGroup *getTriggerGroupByIndex( unsigned int index );
        ScheduleTriggerGroup *getTriggerGroupByID( std::string tgID );

#if 0
        ScheduleTriggerRule *createNewTriggerRule( RESTContentNode *rootCN );
        void freeNewTriggerRule( ScheduleTriggerRule *ruleObj );
        void addNewTriggerRule( std::string tgID, RESTContentNode *rootCN, ScheduleTriggerRule **newEvent );
        void updateTriggerRule( std::string tgID, std::string ruleID, RESTContentNode *rootCN );
        void deleteTriggerRule( std::string tgID, std::string ruleID );
#endif
        void generateTriggerGroupListContent( RESTContentNode *rootCN ); 
        void generateTriggerGroupContent( std::string ruleID, RESTContentNode *rootCN ); 
        void generateTriggerGroupRuleListContent( std::string triggerGroupID, RESTContentNode *rootCN ); 
        void generateTriggerGroupRuleContent( std::string triggerGroupID, std::string ruleID, RESTContentNode *rootCN ); 

        unsigned int getEventRuleCount();
        ScheduleEventRule *getEventRuleByIndex( unsigned int index );
        ScheduleEventRule *getEventRuleByID( std::string erID );

//        ScheduleEventRule *createNewEventRule();
//        void freeNewEventRule( ScheduleEventRule *ruleObj );
//        void addNewEventRule( ScheduleEventRule *event );
//        void addNewEventRule( RESTContentNode *rootCN, ScheduleEventRule **newEvent );
//        void updateEventRule( std::string erID, RESTContentNode *rootCN );

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

//        virtual bool lookupObj( RESTContentIDStack &idStack, RESTContentTemplate *ctObj, std::string objType );
//        virtual bool createObj( RESTContentIDStack &idStack, RESTContentTemplate *ctObj, std::string objType, std::string &objID );



        virtual void startObject( std::string objID );
        virtual void fieldsValid( std::string objID );
        virtual void startChild( std::string objID );
        virtual void endChild( std::string objID );
        virtual void endObject( std::string objID );
        virtual void updateField( std::string objID, std::string name, std::string value );
        virtual void updateRef( std::string objID, std::string name, std::string value );
        virtual void updateTag( std::string objID, std::string name, std::string value );

        virtual void addListMember( std::string objID, std::string listID, std::string childID );
};

#endif // __SCHEDULE_MANAGER_H__

