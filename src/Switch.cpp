#include <sys/time.h>

#include "Switch.hpp"

SwitchLogEntry::SwitchLogEntry()
{
    epochTime    = 0;
    relativeTime = 0;
}

SwitchLogEntry::~SwitchLogEntry()
{

}

void
SwitchLogEntry::createEntry( std::string msg, std::string org, std::string sState, std::string eState )
{
    struct timeval curTime;

    if( gettimeofday( &curTime, NULL ) == 0 )
    {
        epochTime    = curTime.tv_sec;
        relativeTime = (curTime.tv_usec / 1000);
    }

    message    = msg;
    origin     = org;
    startState = sState;
    endState   = eState;
}

unsigned long 
SwitchLogEntry::getEpochTime()
{
    return epochTime;
}

unsigned int  
SwitchLogEntry::getRelativeTime()
{
    return relativeTime;
}

std::string   
SwitchLogEntry::getMessage()
{
    return message;
}

std::string   
SwitchLogEntry::getOrigin()
{
    return origin;
}

std::string   
SwitchLogEntry::getStartState()
{
    return startState;
}

std::string   
SwitchLogEntry::getEndState()
{
    return endState;
}

std::string   
SwitchLogEntry::getEpochTimeString()
{
    char tmpStr[64];
    std::string rtnStr;

    sprintf(tmpStr, "%d", epochTime);

    rtnStr = tmpStr;

    return rtnStr;
}

std::string   
SwitchLogEntry::getRelativeTimeString()
{
    char tmpStr[64];
    std::string rtnStr;

    sprintf(tmpStr, "%d", relativeTime);

    rtnStr = tmpStr;

    return rtnStr;
}

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
Switch::setStateOn( std::string origin )
{
    SwitchLogEntry *logEntry = new SwitchLogEntry();

    logEntry->createEntry( "Turn Switch On.", origin, "off", "on" );

    logList.push_back( logEntry );

    return parent->setState( this, 1 );
}

bool 
Switch::setStateOff( std::string origin )
{
    SwitchLogEntry *logEntry = new SwitchLogEntry();

    logEntry->createEntry( "Turn Switch Off.", origin, "on", "off" );

    logList.push_back( logEntry );

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

unsigned int 
Switch::getLogEntryCount()
{
    return logList.size();
}

SwitchLogEntry* 
Switch::getLogEntry( unsigned int Index )
{
    SwitchLogEntry *rtnValue;

    if( Index >= logList.size() )
        return NULL;

    for( std::list<SwitchLogEntry *>::iterator it=logList.begin(); it != logList.end(); ++it)
    {
        if(Index == 0)
            return *it;

        Index -= 1;
    }

    return NULL;
}


