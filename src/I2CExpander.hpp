#ifndef __I2CEXPANDER_H__
#define __I2CEXPANDER_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "Switch.hpp"

class I2CExpander : public SwitchDevice
{
    private:
        int deviceIndex;
        int busAddress;

        bool getAttributeInt( xmlNode *elem, std::string attrName, int &result );

    public:

        I2CExpander();
       ~I2CExpander();

        void setI2CAddressing( int i2cDeviceIndex, int i2cBusAddress );
        
        int getDeviceNumber();
        int getBusAddress();

        bool setFromConfiguration( xmlDocPtr doc, xmlNode *i2cElem );

      
};

#endif // __I2CEXPANDER_H__

