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

        Switch *ioPin;

    public:
        Zone();
       ~Zone();

};

class ZoneManager
{
    private:
        std::string cfgPath;

        std::vector<Zone> ioList;

        std::vector<I2CExpander> ioExpanderList;

    public:
        ZoneManager();
       ~ZoneManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();
};

#endif // __ZONE_H__

