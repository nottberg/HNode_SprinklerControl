#include "Switch.hpp"

Switch::Switch( SwitchDevice *parentPtr )
{
    parent = parentPtr;
    
    mapped     = false;
    controlled = false;
    capOnOff   = false;
}

Switch::~Switch()
{

}

void 
Switch::setID( std::string idStr )
{
    id = idStr;
}

std::string 
Switch::getID()
{
    return id;
}

void 
Switch::setAddress( std::string addressStr )
{
    address = addressStr;
}

std::string 
Switch::getAddress()
{
    return address;
}

void 
Switch::setName( std::string nameStr )
{
    name = nameStr;
}

std::string 
Switch::getName()
{
    return name;
}

void 
Switch::setDescription( std::string descStr )
{
    desc = descStr;
}

std::string 
Switch::getDescription()
{
    return desc;
}

bool 
Switch::isStateOn()
{
    int state;

    // Try to get the switch state
    if( parent->getState( this, state ) )
    {
        // Error - say the switch is off.
        return false;
    }

    // If the switch is on then return true
    if( state )
        return true;

    // Otherwise false
    return false;
}

bool 
Switch::setStateOn()
{
    return parent->setState( this, 1 );
}

bool 
Switch::setStateOff()
{
    return parent->setState( this, 0 );
}

void 
Switch::setMapped()
{
    mapped = true;
}

void 
Switch::clearMapped()
{
    mapped = false;
}

bool 
Switch::isMapped()
{
    return mapped;
}

void 
Switch::setControlled()
{
    controlled = true;
}

void 
Switch::clearControlled()
{
    controlled = false;
}

bool 
Switch::isControlled()
{
    return controlled;
}

void 
Switch::setCapOnOff()
{
    capOnOff = true;
}

void 
Switch::clearCapOnOff()
{
    capOnOff = false;
}

bool 
Switch::hasCapOnOff()
{
    return capOnOff;
}


