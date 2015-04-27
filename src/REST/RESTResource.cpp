#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <stdio.h>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "REST.hpp"

       
RESTResourcePE::RESTResourcePE()
{
    matchType = REST_RPE_MATCH_NOTSET;
}

RESTResourcePE::~RESTResourcePE()
{

}

void
RESTResourcePE::setMatch( REST_RPE_MATCH_T mType, std::string elemStr )
{
    matchType  = mType;
    patternStr = elemStr;
}

REST_RPE_MATCH_T
RESTResourcePE::getMatchType()
{
    return matchType;
}

std::string
RESTResourcePE::getMatchElement()
{
    return patternStr;
}

bool
RESTResourcePE::isMatch( std::string testElem )
{
    if( (matchType == REST_RPE_MATCH_STRING) && (testElem == patternStr) )
    {
        // This element matches
        return true;
    }
    
    // Not a match
    return false;
}

RESTResource::RESTResource()
{

}

RESTResource::~RESTResource()
{

}

void 
RESTResource::setURLPattern( std::string pattern, REST_RMETHOD_T methodFlags )
{
    RESTResourcePE elem;

    urlPattern = pattern;
    methods = methodFlags;

    boost::char_separator<char> sep("/");
    boost::tokenizer< boost::char_separator<char> > tokens(urlPattern, sep);
    unsigned int index = 0;
    BOOST_FOREACH (const std::string& t, tokens) 
    {
        boost::regex  e("\\{([A-Za-z0-9-]+)\\}");
        boost::smatch what;

        if( regex_match(t, what, e) )
        {
            std::cout << "Adding URL parameter: " << what[1] << std::endl; 
            elem.setMatch( REST_RPE_MATCH_PARAM, what[1] );
            patternElements.push_back( elem );
            index += 1;
        }
        else
        {
            std::cout << "Adding pattern: " << t << "." << index << std::endl;
            elem.setMatch( REST_RPE_MATCH_STRING, t );
            patternElements.push_back( elem );
            index += 1;
        }
    }

}

bool 
RESTResource::linkRequest( RESTRequest *request )
{
    RESTResourcePE *elem;

    std::cout << "Process Request Start: " << request->getURL() << std::endl;

    // Check that the requested operation is supported by this resource.
    if( (request->getMethod() & methods) == 0 )
    {
        std::cout << "Process Request Result -- method not supported" << std::endl;
        return false;
    }

    // Walk through the request URL to make sure its a match.
    boost::char_separator<char> sep("/");
    boost::tokenizer< boost::char_separator<char> > tokens(request->getURL(), sep);
    unsigned int index = 0;
    BOOST_FOREACH (const std::string& t, tokens) 
    {
        std::cout << "Process Request:" << t << "." << index << std::endl;
        std::cout << "Process Request - index: " << index << "." << patternElements.size() << std::endl;

        // If the request has more elements than the 
        // pattern then this is not a match
        if( index >= patternElements.size() )
        {
            std::cout << "Process Request Result -- request > pattern" << std::endl;
            return false;
        }

        // Get the current pattern element
        elem = &patternElements[ index ];

        // Determine what to do with this
        // element
        switch( elem->getMatchType() )
        {
            case REST_RPE_MATCH_STRING:
                if( elem->isMatch( t ) == false )
                {
                    std::cout << "Process Request Result -- mismatched element" << std::endl;
                    std::cout << "Mismatch element: " << elem->getMatchElement() << "." << index <<std::endl;
                    return false;
                }
            break;

            case REST_RPE_MATCH_PARAM:
                request->setURIParameter( elem->getMatchElement(), t );
            break;
        }

        // Examine the next element
        index += 1;
    }

    // If everything matched but the request
    // and pattern didn't have the same number
    // of elements, then it is not a match.
    if( index != patternElements.size() )
    {
        std::cout << "Process Request Result -- request < pattern" << std::endl;
        return false;
    }

    // Setup the link for future request handling.
    request->setLink( this );

    // Request handled.
    return true;
}

bool 
RESTResource::executeRequest( RESTRequest *request )
{
    // Match process was successful,
    // execute the request.
    switch( request->getMethod() )
    {
        case REST_RMETHOD_GET:
            std::cout << "Process Request: Get" << std::endl;
            restGet( request );
        break;

        case REST_RMETHOD_PUT:
            std::cout << "Process Request: Put" << std::endl;
            restPut( request );
        break;

        case REST_RMETHOD_POST:
            std::cout << "Process Request: Post" << std::endl;
            restPost( request );
        break;

        case REST_RMETHOD_DELETE:
            std::cout << "Process Request: Delete" << std::endl;
            restDelete( request );
        break;
    }

    // Request handled.
    return true;
}

void 
RESTResource::restGet( RESTRequest *request )
{
    std::cout << "RESTResource::restGet" << std::endl;
    request->setResponseCode( REST_HTTP_RCODE_NOT_IMPLEMENTED );
    request->sendResponse();
}

void 
RESTResource::restPut( RESTRequest *request )
{
    std::cout << "RESTResource::restPut" << std::endl;
    request->setResponseCode( REST_HTTP_RCODE_NOT_IMPLEMENTED );
    request->sendResponse();
}

void 
RESTResource::restPost( RESTRequest *request )
{
    std::cout << "RESTResource::restPost" << std::endl;
    request->setResponseCode( REST_HTTP_RCODE_NOT_IMPLEMENTED );
    request->sendResponse();
}

void 
RESTResource::restDelete( RESTRequest *request )
{
    std::cout << "RESTResource::restDelete" << std::endl;
    request->setResponseCode( REST_HTTP_RCODE_NOT_IMPLEMENTED );
    request->sendResponse();
}

RESTResourceRESTContentList::RESTResourceRESTContentList( std::string pattern, RESTContentManager &mgr, unsigned int type, std::string list, std::string prefix, std::string relationshipName, std::string relationshipRoot )
: contentMgr( mgr )
{
    setURLPattern( pattern, (REST_RMETHOD_T)( REST_RMETHOD_GET | REST_RMETHOD_POST ) );

    objType   = type;
    listName  = list;
    objPrefix = prefix;
    relName   = relationshipName;
    relRoot   = relationshipRoot;
}

RESTResourceRESTContentList::~RESTResourceRESTContentList()
{

}

void
RESTResourceRESTContentList::restGet( RESTRequest *request )
{
    std::string        rspData;
    RESTContentHelper *helper;
    RESTContentNode   *objNode;
    RESTContentNode *curNode;

    std::cout << "RESTResourceRESTContentList::restGet" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        objNode = helper->getRootNode();

        // Create the root object
        objNode->setAsArray( listName );

        // Enumerate the zone-group list
        std::vector< std::string > idList;

        contentMgr.getIDVectorByType( objType, idList );

        for( std::vector< std::string >::iterator it = idList.begin() ; it != idList.end(); ++it)
        {
            std::cout << "RESTResourceRESTContentList::restGet - " << *it << std::endl;

            curNode = new RESTContentNode();
            curNode->setAsID( *it );
            curNode->setID( *it );
            objNode->addChild( curNode );
        }

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );

    }
    catch( RCMException& me )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, me.getErrorCode(), me.getErrorMsg() ); 
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
RESTResourceRESTContentList::restPost( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentTemplate  *templateNode;
    RESTContentNode   *objNode;
    std::string        objID;

    std::cout << "RESTResourceRESTContentList::restPost" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Generate a template for acceptable data
        templateNode = contentMgr.getContentTemplateForType( objType );

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() ); 

        // Create the new object
        contentMgr.createObj( objType, objPrefix, objID );
        contentMgr.updateObj( objID, helper->getRootNode() );
        contentMgr.addRelationship( relName, relRoot, objID );
    }
    catch( RCMException& me )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, me.getErrorCode(), me.getErrorMsg() ); 
        return;
    }
    catch(...)
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 10000, "Internal Controller Error" ); 
        return;
    }

    // Build a response, including the new rule id
    request->sendResourceCreatedResponse( objID );
}


RESTResourceRESTContentObject::RESTResourceRESTContentObject(  std::string pattern, RESTContentManager &mgr, unsigned int type, std::string idParam )
: contentMgr( mgr )
{
    setURLPattern( pattern, (REST_RMETHOD_T)( REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE ) );
 
    objType  = type;
    objUrlID = idParam; 
}

RESTResourceRESTContentObject::~RESTResourceRESTContentObject()
{

}

void
RESTResourceRESTContentObject::restGet( RESTRequest *request )
{
    RESTContentHelper *helper;
    RESTContentNode   *outNode;
    RESTContentNode   *objNode;
    std::string        objID;

    if( request->getURIParameter( objUrlID, objID ) )
    {
        printf("Failed to look up ruleid parameter\n");
        request->setResponseCode( REST_HTTP_RCODE_BAD_REQUEST );
        request->sendResponse();
        return;
    }

    printf( "URL objID: %s\n", objID.c_str() );

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Parse the content
        helper = RESTContentHelperFactory::getResponseSimpleContentHelper( request->getInboundRepresentation() ); 

        // Get a pointer to the root node
        outNode = helper->getRootNode();

        // Lookup the rule object
        objNode = contentMgr.getObjectByID( objID );

        // Generate the list
        objNode->setContentNodeFromFields( outNode );           

        // Make sure we have the expected object
        helper->generateContentRepresentation( request->getOutboundRepresentation() );
    
    }
    catch( RCMException& me )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, me.getErrorCode(), me.getErrorMsg() ); 
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
RESTResourceRESTContentObject::restPut( RESTRequest *request )
{
    std::string          objID;
    RESTContentHelper    *helper;
    RESTContentTemplate  *templateNode;
//    RESTContentNode      *objNode;

    std::cout << "RESTResourceRESTContentObject::restPut" << std::endl;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try{

        if( request->getURIParameter( objUrlID, objID ) )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 0, "A valid id must be provided as part of the URL." ); 
            return;
        }

        printf( "RESTResourceRESTContentObject - PUT: %s\n", objID.c_str() );

        // Generate a template for acceptable data
        templateNode = contentMgr.getContentTemplateForType( objType );

        // Allocate the appropriate type of helper to parse the content
        helper = RESTContentHelperFactory::getRequestSimpleContentHelper( request->getInboundRepresentation() );

        // Parse the content based on the template ( throws an exception for missing content )
        helper->parseWithTemplate( templateNode, request->getInboundRepresentation() );

        // Create the new object
        contentMgr.updateObj( objID, helper->getRootNode() );
    }
    catch( RCMException& me )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, me.getErrorCode(), me.getErrorMsg() ); 
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
RESTResourceRESTContentObject::restDelete( RESTRequest *request )
{
    std::string objID;

    try
    {

        // extract the ruleid parameter
        if( request->getURIParameter( objUrlID, objID ) )
        {
            request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, 0, "A valid id must be provided as part of the URL." );
            return;
        }

        printf( "RESTResourceRESTContentObject - DELETE: %s\n", objID.c_str() );

        // Attempt the delete operation.
        contentMgr.deleteObjByID( objID );

    }
    catch( RCMException& me )
    {
        request->sendErrorResponse( REST_HTTP_RCODE_BAD_REQUEST, me.getErrorCode(), me.getErrorMsg() ); 
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


