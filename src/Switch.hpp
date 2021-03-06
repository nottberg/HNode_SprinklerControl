#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>

class SwitchDevice;

class SwitchLogEntry
{
    private:
        unsigned long epochTime;
        unsigned int  relativeTime;
        std::string   message;
        std::string   origin;
        std::string   startState;
        std::string   endState;

    public:
        SwitchLogEntry();
       ~SwitchLogEntry();

        void createEntry( std::string msg, std::string origin, std::string sState, std::string eState );

        unsigned long getEpochTime();
        unsigned int  getRelativeTime();
        std::string   getEpochTimeString();
        std::string   getRelativeTimeString();
        std::string   getMessage();
        std::string   getOrigin();
        std::string   getStartState();
        std::string   getEndState();
};

class Switch
{
    private:

        SwitchDevice  *parent;

        std::string  address;
        std::string  id;
        std::string  name;
        std::string  desc;

        // This switch has an entry in the mapping
        // configuration file.
        bool         mapped;

        // This switch is controlled by a higher level
        // object. (i.e. a irrigation zone controlling a valve)
        bool         controlled;

        // This switch can be turned on and off, as opposed
        // to being a read only input.
        bool         capOnOff;

        // Path to the persisted log entries.
        std::string logPath;

        // An in memory copy of the switch log entries.
        std::list<SwitchLogEntry *> logList; 

        void appendLogEntry( std::string msg, std::string origin, std::string startState, std::string endState );
        void initLog();
        void persistLog();

    public:
        Switch( SwitchDevice *parentPtr );
       ~Switch();

        void setAddress( std::string address );
        std::string getAddress();

        void setID( std::string idStr );
        std::string getID();

        void setName( std::string nameStr );
        std::string getName();

        void setDescription( std::string descStr );
        std::string getDescription();

        void setMapped();
        void clearMapped();
        bool isMapped();

        void setControlled();
        void clearControlled();
        bool isControlled();

        void setCapOnOff();
        void clearCapOnOff();
        bool hasCapOnOff();

        bool isStateOn();
        bool setStateOn( std::string origin );
        bool setStateOff( std::string origin );

        unsigned int getLogEntryCount();
        SwitchLogEntry* getLogEntry( unsigned int Index );
  
};

class SwitchDevice
{
    private:
        std::string id;

        std::vector<Switch> SwitchList;
 
        bool getAttributeInt( xmlNode *elem, std::string attrName, int &result );
        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool getChildContent( xmlNode *elem, std::string childName, std::string &result );

        bool setSwitchFromConfiguration( xmlDocPtr doc, xmlNode *switchElem );
        bool setCapabilitiesFromConfiguration( xmlDocPtr doc, xmlNode *switchElem );

    public:

        SwitchDevice();
       ~SwitchDevice();

        virtual void setID( std::string idStr );
        virtual std::string getID();

        void  clearSwitchList();
        int   appendSwitch( std::string defaultId, std::string addressStr );
 
        int   getSwitchCount();
        Switch *getSwitchByIndex(int Index);

        bool setSwitchMapFromConfiguration( xmlDocPtr doc, xmlNode *gpioMapElem );

        virtual bool isInput( Switch *io )        = 0;
        virtual bool setAsOuput( Switch *io )     = 0;
        virtual bool setAsInput( Switch *io )     = 0;

        virtual bool getState( Switch *io, int &state ) = 0;
        virtual bool setState( Switch *io, int state ) = 0;

        virtual bool start() = 0;
};

#endif // __SWITCH_H__

