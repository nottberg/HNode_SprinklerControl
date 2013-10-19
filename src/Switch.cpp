#include "Switch.hpp"

Switch::Switch( SwitchDevice *parentPtr )
{
    parent = parentPtr;
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


