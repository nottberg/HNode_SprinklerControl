#ifndef __ZONE_H__
#define __ZONE_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include "I2CExpander.hpp"
#include "SwitchManager.hpp"

class Zone
{
    private:

        std::vector<Switch *> valveList;

        std::string id;
        std::string name;
        std::string desc;
        std::string mapPathID;

        std::string diagramLocate;
        std::string diagramActive;

    public:
        Zone();
       ~Zone();

        void setID( std::string idValue );
        void setName( std::string nameValue );
        void setDescription( std::string descValue );
        void setMapPathID( std::string pathID );
        void setDiagramLocate( std::string filename );
        void setDiagramActive( std::string filename );

        std::string getID();
        std::string getName();
        std::string getDescription();
        std::string getMapPathID();

        bool getDiagramLocate( std::string &diagramData );
        bool getDiagramActive( std::string &diagramData );

        void setStateOn( std::string origin );
        void setStateOff( std::string origin );

        void clearSwitchList();
        void addSwitch( Switch *swObj );
};

class ZoneManager
{
    private:
        std::string cfgPath;

        std::vector<Zone *> zoneList;

        SwitchManager *switchManager;

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );
        bool getChildContent( xmlNode *elem, std::string childName, std::string &result );
        bool getDiagrams( xmlNode *elem, Zone *zoneObj );
        bool getValves( xmlNode *elem, Zone *zoneObj );

        bool addZone( xmlDocPtr doc, xmlNode *devElem );

    public:
        ZoneManager();
       ~ZoneManager();

        void setSwitchManager( SwitchManager *switchManager );

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        int getZoneCount();
        Zone *getZoneByIndex( int index );

        Zone *getZoneByID( std::string zoneID );

        bool start();

/*
        void turnOnZoneById( std::string zoneID );
        void turnOffZoneById( std::string zoneID );
*/
};

#endif // __ZONE_H__

