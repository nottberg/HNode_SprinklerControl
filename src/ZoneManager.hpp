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

        std::string diagramLocate;
        std::string diagramActive;

    public:
        Zone();
       ~Zone();

        void setID( std::string idValue );
        void setName( std::string nameValue );
        void setDescription( std::string descValue );
        void setDiagramLocate( std::string filename );
        void setDiagramActive( std::string filename );

        std::string getID();
        std::string getName();
        std::string getDescription();
        std::string getDiagramLocate();
        std::string getDiagramActive();
};

class ZoneManager
{
    private:
        std::string cfgPath;

        std::vector<Zone *> zoneList;

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool getChildContent( xmlNode *elem, std::string childName, std::string &result );
        bool getDiagrams( xmlNode *elem, Zone *zoneObj );
        bool getValves( xmlNode *elem, Zone *zoneObj );

        bool addZone( xmlDocPtr doc, xmlNode *devElem );

    public:
        ZoneManager();
       ~ZoneManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        int getZoneCount();
        Zone *getZoneByIndex( int index );

        Zone *getZoneByID( std::string zoneID );

        bool start();
};

#endif // __ZONE_H__

