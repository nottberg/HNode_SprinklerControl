#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include "REST/REST.hpp"
#include "ScheduleManager.hpp"
#include "ScheduleResource.hpp"



ScheduleZoneGroupListResource::ScheduleZoneGroupListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/zone-groups", mgr, SCH_ROTID_ZONEGROUP, "zone-group-list", "zg", "zone-groups", "root" )
{

}

ScheduleZoneGroupListResource::~ScheduleZoneGroupListResource()
{

}

#if 0
ScheduleZoneGroupListResource::ScheduleZoneGroupListResource()
{
    setURLPattern( "/schedule/zone-groups", (REST_RMETHOD_T)(REST_RMETHOD_GET|REST_RMETHOD_POST) );
}

void
ScheduleZoneGroupListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneGroupListResource::restGet( RESTRequest *request )
{
    std::string        rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneGroupListResource::restGet" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateZoneGroupListContent( objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );

    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }    

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
ScheduleZoneGroupListResource::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    ScheduleZoneGroup *ruleObj;

    std::cout << "ScheduleZoneGroupListResource::restPost" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = ScheduleZoneGroup::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        schManager->addNewZoneGroup( helper->getRootNode(), &ruleObj );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( ruleObj->getID() );
}
#endif

ScheduleZoneGroupResource::ScheduleZoneGroupResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/zone-groups/{zonegroupid}", mgr, SCH_ROTID_ZONEGROUP, "zonegroupid" )
{
    //setURLPattern( "/schedule/zone-groups/{zonegroupid}", (REST_RMETHOD_T)( REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE ) );
}

ScheduleZoneGroupResource::~ScheduleZoneGroupResource()
{

}

#if 0
void
ScheduleZoneGroupResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneGroupResource::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string objID;

    if( request->getURIParameter( "zonegroupid", objID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateZoneGroupContent( objID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleZoneGroupResource::restPut( RESTRequest *request )
{
    std::string zgID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneGroupResource::restPut" << std::endl;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        // Generate a template for acceptable data
        templateNode = ScheduleZoneGroup::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        schManager->updateZoneGroup( zgID, helper->getRootNode() );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleZoneGroupResource::restDelete( RESTRequest *request )
{
    std::string zgID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneGroupResource::restDelete" << std::endl;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    try
    {
        // Attempt the delete operation.
        schManager->deleteZoneGroup( zgID );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}
#endif

ScheduleZoneRuleListResource::ScheduleZoneRuleListResource()
{
    setURLPattern( "/schedule/zone-groups/{zonegroupid}/members", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
}

ScheduleZoneRuleListResource::~ScheduleZoneRuleListResource()
{

}

void
ScheduleZoneRuleListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneRuleListResource::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string        objID;
    ScheduleZoneGroup *zgObj;

    if( request->getURIParameter( "zonegroupid", objID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateZoneGroupRuleListContent( objID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleZoneRuleListResource::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    std::string       objID;
    ScheduleZoneRule  *ruleObj;

    std::cout << "ScheduleoneRuleListResource::restPost" << std::endl;

    if( request->getURIParameter( "zonegroupid", objID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = ScheduleZoneRule::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        schManager->addNewZoneRule( objID, helper->getRootNode(), &ruleObj );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( ruleObj->getID() );
}


ScheduleZoneRuleResource::ScheduleZoneRuleResource()
{
    setURLPattern( "/schedule/zone-groups/{zonegroupid}/members/{zoneid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE) );
}

ScheduleZoneRuleResource::~ScheduleZoneRuleResource()
{

}

void
ScheduleZoneRuleResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleZoneRuleResource::restGet( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateZoneGroupRuleContent( zgID, zoneID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleZoneRuleResource::restPut( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneRuleResource::restPut" << std::endl;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        // Generate a template for acceptable data
        templateNode = ScheduleZoneRule::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        schManager->updateZoneRule( zgID, zoneID, helper->getRootNode() );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleZoneRuleResource::restDelete( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneRuleResource::restDelete" << std::endl;

    if( request->getURIParameter( "zonegroupid", zgID ) )
    {
        printf("Failed to look up zonegroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "zoneid", zoneID ) )
    {
        printf("Failed to look up zoneid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL ZoneID: %s\n", zoneID.c_str() );

    try
    {
        // Attempt the delete operation.
        schManager->deleteZoneRule( zgID, zoneID );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

ScheduleTriggerGroupListResource::ScheduleTriggerGroupListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/trigger-groups", mgr, SCH_ROTID_TRIGGERGROUP, "trigger-group-list", "tg", "trigger-groups", "root" )
{

}

ScheduleTriggerGroupListResource::~ScheduleTriggerGroupListResource()
{

}

#if 0
ScheduleTriggerGroupListResource::ScheduleTriggerGroupListResource()
{
    setURLPattern( "/schedule/trigger-groups", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
}

ScheduleTriggerGroupListResource::~ScheduleTriggerGroupListResource()
{

}

void
ScheduleTriggerGroupListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleTriggerGroupListResource::restGet( RESTRequest *request )
{
    std::string        rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleTriggerGroupListResource::restGet" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateTriggerGroupListContent( objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );

    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }    

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

void 
ScheduleTriggerGroupListResource::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    ScheduleTriggerGroup *ruleObj;

    std::cout << "ScheduleTriggerGroupListResource::restPost" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = ScheduleTriggerGroup::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        schManager->addNewTriggerGroup( helper->getRootNode(), &ruleObj );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( ruleObj->getID() );
}
#endif

ScheduleTriggerGroupResource::ScheduleTriggerGroupResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/trigger-groups/{triggergroupid}", mgr, SCH_ROTID_TRIGGERGROUP, "triggergroupid" )
{
    // setURLPattern( "/schedule/trigger-groups/{triggergroupid}", (REST_RMETHOD_T)(REST_RMETHOD_GET|REST_RMETHOD_PUT|REST_RMETHOD_DELETE) );
}

ScheduleTriggerGroupResource::~ScheduleTriggerGroupResource()
{

}

#if 0
ScheduleTriggerGroupResource::ScheduleTriggerGroupResource()
{
    setURLPattern( "/schedule/trigger-groups/{triggergroupid}", (REST_RMETHOD_T)(REST_RMETHOD_GET|REST_RMETHOD_PUT|REST_RMETHOD_DELETE) );
}

ScheduleTriggerGroupResource::~ScheduleTriggerGroupResource()
{

}

void
ScheduleTriggerGroupResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleTriggerGroupResource::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string objID;

    if( request->getURIParameter( "triggergroupid", objID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateTriggerGroupContent( objID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleTriggerGroupResource::restPut( RESTRequest *request )
{
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    std::string objID;

    if( request->getURIParameter( "triggergroupid", objID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        // Generate a template for acceptable data
        templateNode = ScheduleTriggerGroup::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        schManager->updateTriggerGroup( objID, helper->getRootNode() );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleTriggerGroupResource::restDelete( RESTRequest *request )
{
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string objID;

    if( request->getURIParameter( "triggergroupid", objID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", objID.c_str() );

    try
    {
        // Attempt the delete operation.
        schManager->deleteTriggerGroup( objID );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

#endif


ScheduleTriggerRuleListResource::ScheduleTriggerRuleListResource()
{
    setURLPattern( "/schedule/trigger-groups/{triggergroupid}/members", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
}

ScheduleTriggerRuleListResource::~ScheduleTriggerRuleListResource()
{

}

void
ScheduleTriggerRuleListResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleTriggerRuleListResource::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    std::string        objID;
    ScheduleZoneGroup *zgObj;

    if( request->getURIParameter( "triggergroupid", objID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateTriggerGroupRuleListContent( objID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleTriggerRuleListResource::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    std::string       objID;
    ScheduleTriggerRule  *ruleObj;

    std::cout << "ScheduleoneRuleListResource::restPost" << std::endl;

    if( request->getURIParameter( "triggergroupid", objID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = ScheduleTriggerRule::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        schManager->addNewTriggerRule( objID, helper->getRootNode(), &ruleObj );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( ruleObj->getID() );
}


ScheduleTriggerRuleResource::ScheduleTriggerRuleResource()
{
    setURLPattern( "/schedule/trigger-groups/{triggergroupid}/members/{triggerid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE) );
}

ScheduleTriggerRuleResource::~ScheduleTriggerRuleResource()
{

}

void
ScheduleTriggerRuleResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleTriggerRuleResource::restGet( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    if( request->getURIParameter( "triggergroupid", zgID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "triggerid", zoneID ) )
    {
        printf("Failed to look up triggerid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerID: %s\n", zoneID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {

        if( schManager == NULL )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
            return;
        }

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Generate the list    
        schManager->generateTriggerGroupRuleContent( zgID, zoneID, objNode );        

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleTriggerRuleResource::restPut( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;

    std::cout << "ScheduleTriggerRuleResource::restPut" << std::endl;

    if( request->getURIParameter( "triggergroupid", zgID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "triggerid", zoneID ) )
    {
        printf("Failed to look up triggerid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerID: %s\n", zoneID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        // Generate a template for acceptable data
        templateNode = ScheduleTriggerRule::generateContentTemplate();

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        schManager->updateTriggerRule( zgID, zoneID, helper->getRootNode() );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();

}

void 
ScheduleTriggerRuleResource::restDelete( RESTRequest *request )
{
    std::string zgID;
    std::string zoneID;
    std::string rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;

    std::cout << "ScheduleZoneRuleResource::restDelete" << std::endl;

    if( request->getURIParameter( "triggergroupid", zgID ) )
    {
        printf("Failed to look up triggergroupid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerGroupID: %s\n", zgID.c_str() );

    if( request->getURIParameter( "triggerid", zoneID ) )
    {
        printf("Failed to look up triggerid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL TriggerID: %s\n", zoneID.c_str() );

    try
    {
        // Attempt the delete operation.
        schManager->deleteTriggerRule( zgID, zoneID );
    }
    catch( SMException& sme )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, sme.getErrorCode(), sme.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Success
    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}


#if 0
RESTResourceRESTContentList::RESTResourceRESTContentList( std::string pattern, RESTContentManager &mgr, unsigned int type, std::string list, std::string prefix, std::string relationshipName, std::string relationshipRoot )
: contentMgr( mgr )
{
    setURLPattern( pattern, (REST_RMETHOD_T)( REST_RMETHOD_GET | REST_RMETHOD_POST ) );
    listName  = "event-rule-list";
    objPrefix = "er";
    objType   = SCH_ROTID_EVENTRULE;
    relName   = "event-rules";
    relRoot   = "root";
}
#endif

ScheduleRuleListResource::ScheduleRuleListResource( ScheduleManager &mgr )
:RESTResourceRESTContentList( "/schedule/rules", mgr, SCH_ROTID_EVENTRULE, "event-rule-list", "er", "event-rules", "root" )
{

}

ScheduleRuleListResource::~ScheduleRuleListResource()
{

}

ScheduleRuleResource::ScheduleRuleResource( ScheduleManager &mgr )
:RESTResourceRESTContentObject( "/schedule/rules/{ruleid}", mgr, SCH_ROTID_EVENTRULE, "ruleid" )
{

}

ScheduleRuleResource::~ScheduleRuleResource()
{

}

ScheduleCalendarEventResource::ScheduleCalendarEventResource()
{
    setURLPattern( "/calendar/events", (REST_RMETHOD_T)(REST_RMETHOD_GET) );
}

ScheduleCalendarEventResource::~ScheduleCalendarEventResource()
{

}

void
ScheduleCalendarEventResource::setScheduleManager( ScheduleManager *schMgr )
{
    schManager = schMgr;
}

void 
ScheduleCalendarEventResource::restGet( RESTRequest *request )
{
    std::string rspData;

    std::cout << "ScheduleCalendarResource::restGet" << std::endl;

    rspData = "<hnode-schedule-event-list>";

    printf( "ScheduleCalendarResource: schManager - 0x%lx\n", (long unsigned int) schManager );

    ScheduleDateTime startTime;
    startTime.getCurrentTime();
    startTime.subDays( 15 );

    ScheduleDateTime endTime;
    endTime.getCurrentTime();
    endTime.addDays( 15 );

    if( schManager )
    {
        ScheduleEventList *eventList;

        eventList = schManager->getEventsForPeriod( startTime, endTime );

        if( eventList != NULL )
        {
            printf( "EventList count: %d\n", eventList->getEventCount() );

            for( int indx = 0; indx < eventList->getEventCount(); indx++ )
            {
                ScheduleEvent *event = eventList->getEvent( indx );

                rspData += "<schedule-event>";

                rspData += "<id>";
                rspData += event->getId();
                rspData += "</id>";
       
                rspData += "<title>";
                rspData += event->getTitle();
                rspData += "</title>";

                ScheduleDateTime eventTime;

                event->getStartTime( eventTime );
                rspData += "<start-time>";
                rspData += eventTime.getExtendedISOString();
                rspData += "</start-time>";

                event->getEndTime( eventTime );
                rspData += "<end-time>";
                rspData += eventTime.getExtendedISOString();
                rspData += "</end-time>";

                rspData += "</schedule-event>";
            }

            schManager->freeScheduleEventList( eventList );
        }
    }

    rspData += "</hnode-schedule-event-list>"; 

    RESTRepresentation *rspRep = request->getOutboundRepresentation();
    rspRep->setSimpleContent( "application/xml", (unsigned char*) rspData.c_str(), rspData.size() );

    request->setResponseCode( REST_HTTP_RCODE_OK );
    request->sendResponse();
}

