#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>

class SwitchDevice;

class Switch
{
    private:

        SwitchDevice  *parent;

        std::string  address;
        std::string  id;
        std::string  name;
        std::string  desc;

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
