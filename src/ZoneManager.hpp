#ifndef __ZONE_H__
#define __ZONE_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include "I2CExpander.hpp"

class Zone
{
    private:

        std::vector<Switch *> valveList;

        std::string id;
        std::string name;
        std::string desc;

        std::string diagramOutlinePath;
        std::string diagramActivePath;

    public:
        Zone();
       ~Zone();

        void setID( std::string idValue );
};

class ZoneManager
{
    private:
        std::string cfgPath;

        std::vector<Zone *> zoneList;

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool addZone( xmlDocPtr doc, xmlNode *devElem );

    public:
        ZoneManager();
       ~ZoneManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        bool start();
};

#endif // __ZONE_H__

