#ifndef __SCHEDULE_CONFIG_H__
#define __SCHEDULE_CONFIG_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "REST/REST.hpp"

class ScheduleConfig
{
    private:
        //RESTContentNode *rootNode;
        xmlTextWriterPtr writer;

        bool getAttribute( void *docPtr, void *nodePtr, std::string attrName, std::string &result );
        bool getChildContent( void *docPtr, void *nodePtr, std::string childName, std::string &result );
        void addFieldValues( void *docPtr, void *nodePtr, RESTContentTemplate *cnPtr );

        std::string getTypeField( void *docPtr, void *nodePtr );
        std::string getIDField( void *docPtr, void *nodePtr );

        void *findMatchingElement( void *docPtr, void *nodePtr, std::string elemName );
        void parseTree( void *docPtr, void *nodePtr, RESTContentIDStack &idStack, RESTContentTemplate *cnPtr, bool create );

        bool findFieldValue( std::string fieldName, RESTContentNode *cnPtr, void *docPtr, void *nodePtr );

        bool generateChildContent(  RESTContentNode *childCN );
        bool generateArrayContent( RESTContentNode *arrCN );
        bool generateObjectContent( RESTContentNode *objCN );
        bool generateIDContent( RESTContentNode *idCN );
        bool generateEdgeContent( RESTContentEdge *edgeCE );

        void readObject( xmlDocPtr doc, xmlNodePtr objNode, RESTContentManager *objMgr );
        void readEdge( xmlDocPtr doc, xmlNodePtr edgeNode, RESTContentManager *objMgr );

    public:
        ScheduleConfig();
       ~ScheduleConfig();

        //void clearRootNode();
        //RESTContentNode *getRootNode();
        //RESTContentNode *detachRootNode();
        //static void freeDetachedRootNode( RESTContentNode *objPtr );

        //bool hasRootObject( std::string objectName, RESTContentNode **objPtr );

        //RESTContentNode *getObject( std::string objectName );

        //virtual bool parseRawData( RESTRepresentation *repPtr ) = 0;
        //virtual bool parseWithTemplate( RESTContentNode *templateCN, RESTRepresentation *repPtr ) = 0;

        bool writeConfig( std::string filePath, RESTContentManager *cnPtr );
        bool readConfig( std::string filePath, RESTContentManager *cnPtr );

};

#endif // __SCHEDULE_CONFIG_H__

