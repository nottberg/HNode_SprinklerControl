#include <string>
#include <list>
#include <vector>
#include <map>

#include "I2CExpander.hpp"
#include "MCP23008Expander.hpp"

#include "mcp23008-obj.h"

std::string
MCP23008Expander::getDeviceTypeStr()
{
    return "mcp23008";
}

MCP23008Expander::MCP23008Expander()
{
    // Get the access object
    expander = g_mcp23008_new();

    // Add a record for all 8 gpio
    appendSwitch( "switch0", "0" );
    appendSwitch( "switch1", "1" );
    appendSwitch( "switch2", "2" );
    appendSwitch( "switch3", "3" );
    appendSwitch( "switch4", "4" );
    appendSwitch( "switch5", "5" );
    appendSwitch( "switch6", "6" );
    appendSwitch( "switch7", "7" );

}

MCP23008Expander::~MCP23008Expander()
{

}

bool 
MCP23008Expander::isInput( Switch *io )
{

}

bool 
MCP23008Expander::setAsOuput( Switch *io )
{

}

bool 
MCP23008Expander::setAsInput( Switch *io )
{

}

bool 
MCP23008Expander::getState( Switch *io, int &state )
{

}

bool 
MCP23008Expander::setState( Switch *io, int state )
{

}

bool 
MCP23008Expander::start()
{
    // Init the implementation object
    g_mcp23008_set_i2c_address( expander, getDeviceNumber(), getBusAddress() );

    // Start things up
    //return g_mcp23008_start( expander );
    return false;
}


