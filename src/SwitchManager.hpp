#ifndef __SWITCHMANAGER_H__
#define __SWITCHMANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "Switch.hpp"
#include "I2CExpander.hpp"

class SwitchManager
{
    private:

        std::string cfgPath;

        std::vector<Switch *> switchList;

        std::vector<I2CExpander *> i2cExpanderList;

        bool addI2CExpander( xmlDocPtr doc, xmlNode *devElem );

        bool getAttribute( xmlNode *elem, std::string attrName, std::string &result );

    public:
        SwitchManager();
       ~SwitchManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();

        int getSwitchCount();
        Switch *getSwitchByIndex( int index );

        Switch *getSwitchByID( std::string switchID );
};

#endif // __SWITCHMANAGER_H__

