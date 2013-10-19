#ifndef __MCP23008EXPANDER_H__
#define __MCP23008EXPANDER_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include "I2CExpander.hpp"
#include "mcp23008-obj.h"

class MCP23008Expander : public I2CExpander
{
    private:

        GMCP23008   *expander;

    public:
        MCP23008Expander();
       ~MCP23008Expander();        

        virtual bool isInput( Switch *io );
        virtual bool setAsOuput( Switch *io );
        virtual bool setAsInput( Switch *io );

        virtual bool getState( Switch *io, int &state );
        virtual bool setState( Switch *io, int state );

        virtual bool start();

        static std::string getDeviceTypeStr();

};

#endif // __MCP23008EXPANDER_H__

