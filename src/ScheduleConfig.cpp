#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <list>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>

#include "REST/REST.hpp"
#include "ScheduleConfig.hpp"

ScheduleConfig::ScheduleConfig()
{
    writer = NULL;
}

ScheduleConfig::~ScheduleConfig()
{

}

bool
ScheduleConfig::getAttribute( void *docPtr, void *nodePtr, std::string attrName, std::string &result )
{
    xmlChar *attrValue;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Start with a clear result
    result.clear();

    // Get the type attribute
    attrValue = xmlGetProp( elem, (xmlChar *)attrName.c_str() );

    if( attrValue == NULL )
    {
        return true;
    }    

    result = (char*)attrValue;

    xmlFree( attrValue );

    return false;
}

bool
ScheduleConfig::getChildContent( void *docPtr, void *nodePtr, std::string childName, std::string &result )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlNode *childElem;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Start with a clear result
    result.clear();

    // Find the address element
    childElem = NULL;
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( (curElem->type == XML_ELEMENT_NODE) && (strcmp( (char *)curElem->name, childName.c_str() ) == 0) ) 
        {
            childElem = curElem;
            break;
        }
    }
 
    if(childElem == NULL )
    {
        return true;
    }

    // Get the type attribute
    contentValue = xmlNodeGetContent( childElem );

    result = (char*)contentValue;

    xmlFree( contentValue );

    return false;
}

void
ScheduleConfig::addFieldValues( void *docPtr, void *nodePtr, RESTContentTemplate *cnPtr )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Find the address element
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) == 0 ) ) 
        {
            // Get the node contents
            contentValue = xmlNodeGetContent( curElem );
              
            // Add this name value pair
            //cnPtr->setField( (const char *)curElem->name, (const char *)contentValue );

            // Cleanup temporary copy
            xmlFree( contentValue );
        }
    }
}

void *
ScheduleConfig::findMatchingElement( void *docPtr, void *nodePtr, std::string elemName )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Find the address element
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) == 0 ) ) 
        {
            if( elemName == (const char *)curElem->name )
            {
                return curElem;
            }
        }
    }

    return NULL;
}

typedef struct SCParseChildRecord
{
    xmlNode             *childElem;
    RESTContentTemplate *cnPtr;
}SCPC_T;

std::string 
ScheduleConfig::getTypeField( void *docPtr, void *nodePtr )
{
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;
    std::string result;

    getChildContent( docPtr, nodePtr, "type", result );

    return result;
}

std::string 
ScheduleConfig::getIDField( void *docPtr, void *nodePtr )
{
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;
    std::string result;

    getChildContent( docPtr, nodePtr, "id", result );

    return result;
}

void 
ScheduleConfig::parseTree( void *docPtr, void *nodePtr, RESTContentIDStack &idStack, RESTContentTemplate *cnPtr, bool create )
{
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;

    xmlChar *contentValue;
    xmlNode *curElem;
    //RESTContentNode curCN; 
    SCPC_T childRec;
    std::vector< SCPC_T > childList;
    std::string objID;
    std::string typeStr;

    // Find a node at the current level that matches the template node name
    curElem = (xmlNode *) nodePtr;

    // Done?
    if( curElem == NULL )
    {
        return;
    }

    std::cout << "parseTree - node: " << curElem->name << "  expected: " << cnPtr->getName() << std::endl;

    // If the current element doesn't match the expected name then we are done.
    if( cnPtr->getName() != (char *)curElem->name )
    {
        std::cout << "mismatch, done parsing." << std::endl;
        return;
    }

    // If this is a list element then handle it differently
    if( cnPtr->getType() == RCNT_ARRAY )
    {
        std::cout << "parseTree - array: " << curElem->name << std::endl;
    
        // Add any child elements and recurse on those
        for( curElem = elem->children; curElem; curElem = curElem->next ) 
        {
            if( curElem->type == XML_ELEMENT_NODE ) 
            {
                 
                if( cnPtr->checkForChildMatch( (const char*) curElem->name, &childRec.cnPtr ) )
                {
                    childRec.childElem = curElem;
                    childList.push_back( childRec );
                    continue;
                }
            }
        }

        // Process recursively through all of the identified child elements.
        for( std::vector<SCPC_T>::iterator it = childList.begin() ; it != childList.end(); ++it)
        {
            cnPtr->startChild( objID );

            parseTree( doc, it->childElem, idStack, it->cnPtr, create );

            cnPtr->endChild( objID );
        }  

        return;
    }

    objID   = getIDField( docPtr, curElem );
    typeStr = getTypeField( docPtr, curElem );

    if( create == false )
    {
        if( objID.empty() )
        {
            // The non create case must have object IDs specified.
            std::cout << "Object id not specified: " << objID << std::endl;
            return;
        }

        idStack.pushID( objID );

        if( cnPtr->lookupObj( idStack, cnPtr, typeStr ) == false )
        {
            // Failed to lookup the object so signal error.
            std::cout << "Object doesn't exist and create not allowed: " << objID << std::endl;
            return;
        }
    }
    else
    {
        // Call the create object callback, if objID is empty then a new ID will be created.
        if( cnPtr->createObj( idStack, cnPtr, typeStr, objID ) == false )
        {
            // Failed to create the new object so signal error.
            std::cout << "Object creation failed: " << objID << std::endl;
            return;
        }
    }

    // Add any child elements and recurse on those
    // Find the address element
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( curElem->type == XML_ELEMENT_NODE ) 
        {
            // Check if this child element matches a 
            // child from the template.  If so store it
            // for later processing
            // If not then check if it matches a field.
            // Otherwise check if it should be processed
            // as a tag.
            // If it's content is empty then ignore it.
            if( cnPtr->checkForFieldMatch( (const char*) curElem->name ) )
            {
                // Get the node contents
                contentValue = xmlNodeGetContent( curElem );
                 
                cnPtr->updateField( objID, (const char*) curElem->name, (const char*) contentValue );
  
                // Cleanup temporary copy
                xmlFree( contentValue ); 

                continue;
            }
            else if( cnPtr->checkForRefMatch( (const char*) curElem->name ) )
            {
                // Get the node contents
                contentValue = xmlNodeGetContent( curElem );
                 
                cnPtr->updateRef( objID, (const char*) curElem->name, (const char*) contentValue );
  
                // Cleanup temporary copy
                xmlFree( contentValue ); 

                continue;
            }
            else if( cnPtr->checkForChildMatch( (const char*) curElem->name, &childRec.cnPtr ) )
            {
                childRec.childElem = curElem;
                childList.push_back( childRec );
                continue;
            }
            else if( cnPtr->checkForTagMatch( (const char*) curElem->name ) )
            {
                // Get the node contents
                contentValue = xmlNodeGetContent( curElem );
                 
                cnPtr->updateTag( objID, (const char*) curElem->name, (const char*) contentValue );
  
                // Cleanup temporary copy
                xmlFree( contentValue ); 

                continue;
            }
            else
            {

            }

        }
    }

    // Indicate that the objects fields have all been read in. 
    cnPtr->fieldsValid( objID );

    // Process recursively through all of the identified child elements.
    for( std::vector<SCPC_T>::iterator it = childList.begin() ; it != childList.end(); ++it)
    {
        cnPtr->startChild( objID );

        parseTree( doc, it->childElem, idStack, it->cnPtr, create );

        cnPtr->endChild( objID );
    }    

    // Indicate that the objects processing is complete for this object and 
    // all of it's children.
    cnPtr->endObject( objID );

}

bool
ScheduleConfig::findFieldValue( std::string fieldName, RESTContentNode *cnPtr, void *docPtr, void *nodePtr )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Find the address element
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) == 0 ) ) 
        {
            if( fieldName == (const char *)curElem->name )
            {
                // Get the node contents
                contentValue = xmlNodeGetContent( curElem );
              
                // Add this name value pair
                cnPtr->setField( (const char *)curElem->name, (const char *)contentValue );

                // Cleanup temporary copy
                xmlFree( contentValue );

                return true;
            }
        }
    }

    return false;
}

bool
ScheduleConfig::generateChildContent(  RESTContentNode *childCN )
{

    std::cout << "ScheduleConfig::generateChildContent - start" << std::endl;

    if( childCN->isID() )
    {
        std::cout << "ScheduleConfig::generateChildContent - id" << std::endl;
        generateIDContent( childCN );
    }
    else if( childCN->isObj() )
    {
        std::cout << "ScheduleConfig::generateChildContent - object" << std::endl;
        generateObjectContent( childCN );
    }
    else if( childCN->isArray() )
    {
        std::cout << "ScheduleConfig::generateChildContent - array" << std::endl;
        generateArrayContent( childCN );
    }

    return false;
}

bool
ScheduleConfig::generateArrayContent( RESTContentNode *arrCN )
{
    int rc;

    // Start an element  
    rc = xmlTextWriterStartElement( writer, BAD_CAST arrCN->getName().c_str() );
    if( rc < 0 ) 
        return true;

    for( int indx = 0; indx < arrCN->getChildCount(); indx++ )
    {
        RESTContentNode *childCN = (RESTContentNode *)arrCN->getChildByIndex( indx );
        
        generateChildContent( childCN );
    }

    // Close the element  
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool
ScheduleConfig::generateObjectContent( RESTContentNode *objCN )
{
    std::string rspData;
    int rc;

    // Start an element named zone-rule. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST objCN->getName().c_str() );
    if( rc < 0 ) 
        return true;

    // Go throught the remaining fields.
    std::vector< RESTContentField* > fieldList = objCN->getFieldList();

    for( std::vector< RESTContentField* >::const_iterator it = fieldList.begin(); it != fieldList.end(); ++it )
    {
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST (*it)->getName().c_str() , "%s", (*it)->getValue().c_str() );
        if( rc < 0 ) 
            return true;

    }

#if 0
    std::vector< RESTContentReference* > refList = objCN->getRefList();

    for( std::vector< RESTContentReference* >::const_iterator it = refList.begin(); it != refList.end(); ++it )
    {
        rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST (*it)->getName().c_str() , "%s", (*it)->getValue().c_str() );
        if( rc < 0 ) 
            return true;

    }


    for( int findx = 0; findx < objCN->getFieldCount(); findx++ )
    {
        RESTContentField *field;

        field = objCN->getFieldByIndex( findx );
        
        rspData = "<" + field->getName() + ">" + field->getValue() + "</" + field->getName() + ">"; 
        repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 
    }
#endif

    for( int indx = 0; indx < objCN->getChildCount(); indx++ )
    {
        RESTContentNode *childCN = (RESTContentNode *)objCN->getChildByIndex( indx );
        
        generateChildContent( childCN );
    }

    // Close the element named zone-rule. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;
}

bool
ScheduleConfig::generateIDContent( RESTContentNode *idCN )
{
    int rc;

    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "id" , "%s", idCN->getID().c_str() );
    if( rc < 0 ) 
        return true;

    return false;
}

bool
ScheduleConfig::generateEdgeContent( RESTContentEdge *edgeCE )
{
    int rc;

    // Start an element named edge. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rest-cm-edge" );
    if( rc < 0 ) 
        return true;

    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "src" , "%s", edgeCE->getSrcID().c_str() );
    if( rc < 0 ) 
        return true;

    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "dst" , "%s", edgeCE->getDstID().c_str() );
    if( rc < 0 ) 
        return true;

    rc = xmlTextWriterWriteFormatElement( writer, BAD_CAST "type" , "%s", edgeCE->getEdgeType().c_str() );
    if( rc < 0 ) 
        return true;

    // Close the element named edge. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    return false;

}

#define MY_ENCODING "ISO-8859-1"

bool 
ScheduleConfig::writeConfig( std::string filePath, RESTContentManager *objMgr )
{
    //RESTContentNode *rootCN = cnObj;
    int rc;

    std::cout << "ScheduleConfig::writeConfig" << std::endl;

    // Create a new XmlWriter for filename, with no compression. 
    writer = xmlNewTextWriterFilename( filePath.c_str(), 0 );
    if( writer == NULL ) 
    {
        printf("testXmlwriterFilename: Error creating the xml writer\n");
        return true;
    }

    // Start the document with the xml default for the version,
    // encoding ISO 8859-1 and the default for the standalone
    // declaration. 
    rc = xmlTextWriterStartDocument( writer, NULL, MY_ENCODING, NULL );
    if( rc < 0 ) 
    {
        printf( "testXmlwriterFilename: Error at xmlTextWriterStartDocument\n" );
        return true;
    }

    // Start an element named edge. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rcm-config" );
    if( rc < 0 ) 
        return true;

    // Start an element named edge. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rcm-object-list" );
    if( rc < 0 ) 
        return true;

    // Generate Records for all of the objects
    std::vector< RESTContentNode > objList;

    objMgr->getObjectRepresentationList( objList );

    for( std::vector< RESTContentNode >::iterator it = objList.begin(); it != objList.end(); ++it )
    {
        generateObjectContent( &(*it) );
    }

    // Close the object list. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    // Start an element named edge. 
    rc = xmlTextWriterStartElement( writer, BAD_CAST "rcm-edge-list" );
    if( rc < 0 ) 
        return true;

    // Generate Records for all of the edges
    std::vector< RESTContentEdge > edgeList;

    objMgr->getEdgeRepresentationList( edgeList );

    for( std::vector< RESTContentEdge >::iterator it = edgeList.begin(); it != edgeList.end(); ++it )
    {
        generateEdgeContent( &(*it) );
    }

    // Close the edge list element. 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;

    // Close the root element 
    rc = xmlTextWriterEndElement( writer );
    if( rc < 0 ) 
        return true;


    // Close out the document
    rc = xmlTextWriterEndDocument( writer );
    if( rc < 0 ) 
    {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
        return true;
    }

    // Write the file
    xmlFreeTextWriter( writer );
    writer = NULL;

    return false;
}

void
ScheduleConfig::readObject( xmlDocPtr doc, xmlNodePtr objNode, RESTContentManager *objMgr )
{
    RESTContentNode      nodeCN;
    RESTContentTemplate *templateNode;
    std::string          objID;
    unsigned int         objType;

    if( strcmp( (const char *)objNode->name, "rcm-root" ) == 0 )
    {
        std::cout << "Root Object Special Handling" << std::endl;
        return;    
    }

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Determine the type value based on enclosing type string
        objType = objMgr->getTypeFromObjectElementName( (const char *) objNode->name );

        // Generate a template for acceptable data
        templateNode = objMgr->getContentTemplateForType( objType );

        if( templateNode->isObj() == false )
        {
            return;
        }
    
        // Make sure the root level object matches
        if( templateNode->getName() != (const char *) objNode->name )
        {
            std::cout << "Object name did not match.  Expecting " + templateNode->getName() + " found " + (const char *) objNode->name << std::endl;
            //throw new RESTContentException( 1000, "Object name did not match.  Expecting " + templateCN->getName() + " found " + (const char *)rootElem->name );
            return;
        }

        // Set the name in the generated object
        nodeCN.setName( templateNode->getName() );

        // Handle the id attribute special for the loading case, normally it is ignored for the REST side
        // since it instead is encoded in the URL.
        objID = getIDField( doc, objNode );
        if( objID.empty() )
        {
            std::cout << "Manadatory ID field not found." << std::endl;
            return;
        }

        // Set the id into the object
        nodeCN.setID( objID );

        std::cout << "readObject - objID: " << objID << std::endl;

        // Go through the fields in the template and try to match those.
        std::vector< RESTContentFieldDef* > fieldList = templateNode->getFieldList();

        for( std::vector< RESTContentFieldDef* >::const_iterator it = fieldList.begin(); it != fieldList.end(); ++it )
        {
            std::cout << "Field Name: " << (*it)->getName() << std::endl;
            if( findFieldValue( (*it)->getName(), &nodeCN, doc, objNode ) == false )
            {
                std::cout << "Field Not Found!" << std::endl;
            }
            else
            {
                std::string tmpStr;
                nodeCN.getField( (*it)->getName(), tmpStr );
                std::cout << "Field Found: " << tmpStr << std::endl;
            }
        }

        // Add the object to the object Manager
        objMgr->addObj( objType, objID );
        objMgr->updateObj( objID, &nodeCN );
    }
    catch( RCMException& me )
    {
        return;
    }
    catch(...)
    {
        return;
    }
}

void
ScheduleConfig::readEdge( xmlDocPtr doc, xmlNodePtr edgeNode, RESTContentManager *objMgr )
{
    std::string  listType;
    std::string  parentID;
    std::string  objID;
    xmlChar     *contentValue;
    xmlNode     *curElem;

    // All of the routines below will throw a SMException if they encounter an error
    // during processing.
    try
    {
        // Find the address element
        for( curElem = edgeNode->children; curElem; curElem = curElem->next ) 
        {
            if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) == 0 ) ) 
            {
                if( strcmp( (const char *)curElem->name, "src" ) == 0 )
                {
                    contentValue = xmlNodeGetContent( curElem );
                    parentID = (const char *) contentValue;
                    xmlFree( contentValue );
                }
                else if( strcmp( (const char *)curElem->name, "dst" ) == 0 )
                {
                    contentValue = xmlNodeGetContent( curElem );
                    objID = (const char *) contentValue;
                    xmlFree( contentValue );
                }
                else if( strcmp( (const char *)curElem->name, "type" ) == 0 )
                {
                    contentValue = xmlNodeGetContent( curElem );
                    listType = (const char *) contentValue;
                    xmlFree( contentValue );
                }
            }
        }

        objMgr->addRelationship( listType, parentID, objID );
   
    }
    catch( RCMException& me )
    {
        return;
    }
    catch(...)
    {
        return;
    }
}

bool 
ScheduleConfig::readConfig( std::string filePath, RESTContentManager *objMgr )
{
    xmlDocPtr           doc;
    xmlNode            *rootElem;
    xmlNode            *curElem;
    xmlXPathContextPtr  xpathCtx; 
    xmlXPathObjectPtr   xpathObj; 

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
	    return true;
    }

    std::cout << "ScheduleConfig::readConfig" << std::endl;

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Create xpath evaluation context 
    xpathCtx = xmlXPathNewContext( doc );
    if( xpathCtx == NULL ) 
    {
        fprintf( stderr,"Error: unable to create new XPath context\n" );
        return true;
    }

    // Process all of the object definitions
    // Find the nodes from the object list.
    std::string nodeXPath = "/rcm-config/rcm-object-list/*";

    xpathObj = xmlXPathEvalExpression( (const xmlChar *) nodeXPath.c_str(), xpathCtx );
    if( xpathObj == NULL ) 
    {
        fprintf( stderr,"Error: unable to evaluate xpath expression \"%s\"\n", nodeXPath.c_str() );
        return true;
    }
    
    if( xpathObj->nodesetval != NULL )
    {    
        int nodeCnt = xpathObj->nodesetval->nodeNr;

        if( nodeCnt != 0 )
        {
            std::cout << "Result (%d nodes): " << nodeCnt <<std::endl;
            for( unsigned int nodeIndx = 0; nodeIndx < nodeCnt; ++nodeIndx ) 
            {  
                curElem = xpathObj->nodesetval->nodeTab[ nodeIndx ];	

                if( curElem->type == XML_ELEMENT_NODE ) 
                {
                    std::cout << "object Elem: " << curElem->name <<std::endl;
                    readObject( doc, curElem, objMgr );
	            } 
            }
        }
    }

    // Free things.
    xmlXPathFreeObject(xpathObj);

    // Process all of the edge definitions
    // Find the edges from the edge list.
    std::string edgeXPath = "/rcm-config/rcm-edge-list/*";

    xpathObj = xmlXPathEvalExpression( (const xmlChar *) edgeXPath.c_str(), xpathCtx );
    if( xpathObj == NULL ) 
    {
        fprintf( stderr,"Error: unable to evaluate xpath expression \"%s\"\n", edgeXPath.c_str() );
        return true;
    }
    
    if( xpathObj->nodesetval != NULL )
    {    
        int nodeCnt = xpathObj->nodesetval->nodeNr;

        if( nodeCnt != 0 )
        {
            std::cout << "Result (%d edges): " << nodeCnt <<std::endl;
            for( unsigned int nodeIndx = 0; nodeIndx < nodeCnt; ++nodeIndx ) 
            {  
                curElem = xpathObj->nodesetval->nodeTab[ nodeIndx ];	

                if( curElem->type == XML_ELEMENT_NODE ) 
                {
                    std::cout << "edge Elem: " << curElem->name <<std::endl;
                    readEdge( doc, curElem, objMgr );	         
	            } 
            }
        }
    }

    // Free things.
    xmlXPathFreeObject(xpathObj);

    // Go through the config data.
    //parseTree( doc, rootElem, idStack, cnObj, true );

    // Free things
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc);

    return false;
}


