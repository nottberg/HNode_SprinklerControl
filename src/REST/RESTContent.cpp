#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <list>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "REST.hpp"

RESTContentField::RESTContentField()
{

}

RESTContentField::~RESTContentField()
{

}

void
RESTContentField::setName( std::string nameStr )
{
    name = nameStr;
}

std::string
RESTContentField::getName()
{
    return name;
}

void
RESTContentField::setValue( std::string valueStr )
{
    value = valueStr;
}

std::string
RESTContentField::getValue()
{
    return value;
}

RESTContentNode::RESTContentNode()
{
    type = RCNT_NOTSET;
}

RESTContentNode::~RESTContentNode()
{

}

void 
RESTContentNode::setAsArray( std::string nameStr )
{
    type = RCNT_ARRAY;
    setName( nameStr );
}

void 
RESTContentNode::setAsID( std::string idValue )
{
    type = RCNT_ID;
    setID( idValue );
}

void 
RESTContentNode::setAsObject( std::string nameStr )
{
    type = RCNT_OBJECT;
    setName( nameStr );
}

RCN_TYPE 
RESTContentNode::getType()
{
    return type;
}

bool 
RESTContentNode::isArray()
{
    if( type == RCNT_ARRAY )
        return true;

    return false;
}

bool 
RESTContentNode::isID()
{
    if( type == RCNT_ID )
        return true;

    return false;
}

bool 
RESTContentNode::isObj()
{
    if( type == RCNT_OBJECT )
        return true;

    return false;
}

void 
RESTContentNode::setID( std::string idValue )
{
    setField( "id", idValue );
}

std::string 
RESTContentNode::getID()
{
    std::string returnStr;

    getField( "id", returnStr );

    return returnStr;
}

void 
RESTContentNode::setName( std::string nameValue )
{
    name = nameValue;
}

bool 
RESTContentNode::hasName()
{
    if( name.empty() )
        return false;

    return true;
}

std::string 
RESTContentNode::getName()
{
    return name;
}

void 
RESTContentNode::defineField( std::string name, bool required )
{
    RESTContentField field;

    if( fieldValues.count( name ) > 0 )
    {
        fieldValues[name] = field;
        return;
    }

    RESTContentField newField;
    newField.setName( name );
    //newField.setValue( value );    
    fieldValues.insert( std::pair< std::string, RESTContentField >( name, newField ) );
}

void 
RESTContentNode::setField( std::string name, std::string value )
{
    if( fieldValues.count( name ) > 0 )
    {
        fieldValues[name].setValue( value );
        return;
    }

    RESTContentField newField;
    newField.setName( name );
    newField.setValue( value );
    fieldValues.insert( std::pair< std::string, RESTContentField >( name, newField ) );
}

bool 
RESTContentNode::clearField( std::string name )
{
    fieldValues.erase( name );
}

bool 
RESTContentNode::getField( std::string name, std::string &value )
{
    if( fieldValues.count( name ) > 0 )
    {
       value = fieldValues[ name ].getValue();
       return true;
    }

    return false;
}

std::vector< RESTContentField* >
RESTContentNode::getFieldList()
{
    std::vector< RESTContentField* > resultList;

    for( std::map< std::string, RESTContentField >::const_iterator it = fieldValues.begin(); it != fieldValues.end(); ++it )
    {
        resultList.push_back( (RESTContentField *) &(it->second) );      
    }

    return resultList;
}

void 
RESTContentNode::setParent( RESTContentNode *parentPtr )
{
    parent = parentPtr;
}

RESTContentNode *
RESTContentNode::getParent()
{
    return parent;
}

void 
RESTContentNode::addChild( RESTContentNode *childPtr )
{
    childPtr->setParent( this );
    children.push_back( childPtr );
}

unsigned long 
RESTContentNode::getChildCount()
{
    return children.size();
}

RESTContentNode *
RESTContentNode::getChildByIndex( unsigned long index )
{
    if( index >= children.size() )
        return NULL;

    return children[ index ];
}

bool 
RESTContentNode::getRequiredFields( std::map< std::string, std::string > &fields )
{

}

bool 
RESTContentNode::getOptionalFields( std::map< std::string, std::string > &fields )
{

}

bool 
RESTContentNode::getUpdateFields( std::map< std::string, std::string > &fields )
{

}

RESTContentHelper::RESTContentHelper()
{
    // Initialize root node
    rootNode = NULL;
}

RESTContentHelper::~RESTContentHelper()
{
    // Cleanup
    clearRootNode();
}

void 
RESTContentHelper::clearRootNode()
{
    // If we have an object then delete it (and its children)
    if( rootNode )
    {
        delete rootNode;
    }

    // Object is no longer
    rootNode = NULL;
}

RESTContentNode *
RESTContentHelper::getRootNode()
{
    // Check if we already have a root node object
    if( rootNode == NULL )
    {
        rootNode = new RESTContentNode();
    }

    // Return object pointer.
    return rootNode;
}

RESTContentNode *
RESTContentHelper::detachRootNode()
{
    RESTContentNode *returnPtr;

    // Check if we already have a root node object
    if( rootNode == NULL )
    {
        returnPtr = new RESTContentNode();
    }   
    else
    {
        returnPtr = rootNode;
    }

    // Forget about the datastructure
    rootNode = NULL;

    // Return the detached datastructure
    return returnPtr;
}

void 
RESTContentHelper::freeDetachedRootNode( RESTContentNode *objPtr )
{
    delete objPtr;
}

bool 
RESTContentHelper::hasRootObject( std::string objectName, RESTContentNode **objPtr )
{
    // Clear the object pointer
    *objPtr = NULL;

    if( rootNode == NULL )
        return false;

    if( rootNode->getName() != objectName )
        return false;

    *objPtr = rootNode;

    return true;
}

RESTContentNode *
RESTContentHelper::getObject( std::string objectName )
{
    // FIX ME -- implement search
    return rootNode;
}

RESTContentHelperXML::RESTContentHelperXML()
{

}

RESTContentHelperXML::~RESTContentHelperXML()
{

}

bool
RESTContentHelperXML::getAttribute( void *docPtr, void *nodePtr, std::string attrName, std::string &result )
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
RESTContentHelperXML::getChildContent( void *docPtr, void *nodePtr, std::string childName, std::string &result )
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
RESTContentHelperXML::addFieldValues( void *docPtr, void *nodePtr, RESTContentNode *cnPtr )
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
            cnPtr->setField( (const char *)curElem->name, (const char *)contentValue );

            // Cleanup temporary copy
            xmlFree( contentValue );
        }
    }
}

void 
RESTContentHelperXML::parseTree( void *docPtr, void *nodePtr, RESTContentNode *cnPtr )
{
    xmlChar *contentValue;
    xmlNode *curElem;
    RESTContentNode *curCNPtr;
    xmlDocPtr doc = (xmlDocPtr) docPtr;
    xmlNode *elem = (xmlNode *) nodePtr;

    // Give it the element name
    cnPtr->setName( (const char *) elem->name );

    // Add any terminal children that represent name value pairs
    addFieldValues( doc, elem, cnPtr );

    // Add any child elements and recurse on those
    // Find the address element
    for( curElem = elem->children; curElem; curElem = curElem->next ) 
    {
        if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) != 0 ) )  
        {
            // A new object for the child
            curCNPtr = new RESTContentNode();

            // Setup the relationships
            curCNPtr->setParent( cnPtr );
            cnPtr->addChild( curCNPtr );

            // Recurse to build the tree
            parseTree( docPtr, nodePtr, curCNPtr );
        }
    }    
}

bool 
RESTContentHelperXML::supportsContentDecode( std::string contentType )
{
    if( contentType != "application/xml" )
    {
        return true;
    }

    return false;
}

bool 
RESTContentHelperXML::supportsContentCreate( std::string contentType )
{
    if( contentType != "application/xml" )
    {
        return true;
    }

    return false;
}

bool 
RESTContentHelperXML::parseRawData( RESTRepresentation *repPtr )
{
    xmlDocPtr      doc;
    xmlNode       *rootElem;
    unsigned char *dataPtr;
    unsigned long dataLength;
    std::string   contentType;
   
    std::cout << "RESTContentHelperXML::parseRepSimple -- start" << std::endl;

    // Find the databuffer to parse
    dataPtr = repPtr->getSimpleContentPtr( contentType, dataLength );

    // Run the simple content through the xml parser
    doc = xmlParseMemory( (const char *)dataPtr, dataLength );
    if (doc == NULL) 
    {
        return true;
    }

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    std::cout << "Root Elem Name: " << rootElem->name << std::endl;

    // Clean up any old root element
    clearRootNode(); 

    // Create the root element
    RESTContentNode *rootCN = getRootNode();  

    // Parse all of the object out of the tree
    parseTree( doc, rootElem, rootCN );

    return false;
}

bool
RESTContentHelperXML::findFieldValue( std::string fieldName, RESTContentNode *cnPtr, void *docPtr, void *nodePtr )
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
RESTContentHelperXML::parseWithTemplate( RESTContentNode *templateCN, RESTRepresentation *repPtr )
{
    xmlDocPtr      doc;
    xmlNode       *rootElem;
    unsigned char *dataPtr;
    unsigned long dataLength;
    std::string   contentType;
   
    std::cout << "RESTContentHelperXML::parseWithTemplate -- start" << std::endl;

    // Find the databuffer to parse
    dataPtr = repPtr->getSimpleContentPtr( contentType, dataLength );

    // Run the simple content through the xml parser
    doc = xmlParseMemory( (const char *)dataPtr, dataLength );
    if (doc == NULL) 
    {
        return true;
    }

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    std::cout << "Root Elem Name: " << rootElem->name << std::endl;

    // Clean up any old root element
    clearRootNode(); 

    // Create the root element
    RESTContentNode *rootCN = getRootNode();

    if( templateCN->isID() )
    {
        std::cout << "RESTContentHelperXML::parseWithTemplate - id" << std::endl;
        //generateIDContent( childCN, repPtr );
    }
    else if( templateCN->isObj() )
    {
        std::cout << "RESTContentHelperXML::parseWithTemplate - object" << std::endl;

        // Make sure the root level object matches
        if( templateCN->getName() != (const char *)rootElem->name )
        {
            std::cout << "Object name did not match.  Expecting " + templateCN->getName() + " found " + (const char *)rootElem->name << std::endl;
            throw new RESTContentException( 1000, "Object name did not match.  Expecting " + templateCN->getName() + " found " + (const char *)rootElem->name );
        }

        // Set the name in the generated object
        rootCN->setName( templateCN->getName() );

        // Go through the fields in the template and try to match those.
        std::vector< RESTContentField* > fieldList = templateCN->getFieldList();

        for( std::vector< RESTContentField* >::const_iterator it = fieldList.begin(); it != fieldList.end(); ++it )
        {
            std::cout << "Field Name: " << (*it)->getName() << std::endl;
            if( findFieldValue( (*it)->getName(), rootCN, doc, rootElem ) == false )
            {
                std::cout << "Field Not Found!" << std::endl;
            }
            else
            {
                std::string tmpStr;
                rootCN->getField( (*it)->getName(), tmpStr );
                std::cout << "Field Found: " << tmpStr << std::endl;
            }
        }
    }
    else if( templateCN->isArray() )
    {
        std::cout << "RESTContentHelperXML::parseWithTemplate - array" << std::endl;
        //generateArrayContent( childCN, repPtr );
    }

    // Parse all of the object out of the tree
    //parseTree( doc, rootElem, rootCN );

    return false;
}

bool
RESTContentHelperXML::generateChildContent(  RESTContentNode *childCN, RESTRepresentation *repPtr )
{

    std::cout << "RESTContentHelperXML::generateChildContent - start" << std::endl;

    if( childCN->isID() )
    {
        std::cout << "RESTContentHelperXML::generateChildContent - id" << std::endl;
        generateIDContent( childCN, repPtr );
    }
    else if( childCN->isObj() )
    {
        std::cout << "RESTContentHelperXML::generateChildContent - object" << std::endl;
        generateObjectContent( childCN, repPtr );
    }
    else if( childCN->isArray() )
    {
        std::cout << "RESTContentHelperXML::generateChildContent - array" << std::endl;
        generateArrayContent( childCN, repPtr );
    }

    return false;
}

bool
RESTContentHelperXML::generateArrayContent( RESTContentNode *arrCN, RESTRepresentation *repPtr )
{
    std::string rspData;

    rspData = "<" + arrCN->getName() + ">";
    repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 

    for( int indx = 0; indx < arrCN->getChildCount(); indx++ )
    {
        RESTContentNode *childCN = arrCN->getChildByIndex( indx );
        
        generateChildContent( childCN, repPtr );
    }

    rspData = "</" + arrCN->getName() + ">";
    repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 

}

bool
RESTContentHelperXML::generateObjectContent( RESTContentNode *objCN, RESTRepresentation *repPtr )
{
    std::string rspData;

    rspData = "<" + objCN->getName() + ">";
    repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 

    std::vector< RESTContentField* > fieldList = objCN->getFieldList();

    for( std::vector< RESTContentField* >::const_iterator it = fieldList.begin(); it != fieldList.end(); ++it )
    {
        rspData = "<" + (*it)->getName() + ">" + (*it)->getValue() + "</" + (*it)->getName() + ">"; 
        repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 
    }

#if 0
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
        RESTContentNode *childCN = objCN->getChildByIndex( indx );
        
        generateChildContent( childCN, repPtr );
    }

    rspData = "</" + objCN->getName() + ">";
    repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 
}

bool
RESTContentHelperXML::generateIDContent( RESTContentNode *idCN, RESTRepresentation *repPtr )
{
    std::string rspData;

    rspData = "<id>";
    rspData += idCN->getID();
    rspData += "</id>";    

    repPtr->appendSimpleContent( (unsigned char*) rspData.c_str(), rspData.size() ); 

    return false;
}

bool 
RESTContentHelperXML::generateContentRepresentation( RESTRepresentation *repPtr )
{
    RESTContentNode *rootCN = getRootNode();
    std::string rspData;

    std::cout << "RESTContentHelperXML::generateContentRepresentation" << std::endl;

    repPtr->setSimpleContent( "application/xml" );

    generateChildContent( rootCN, repPtr );

    return false;
}


RESTContentHelperJSON::RESTContentHelperJSON()
{

}

RESTContentHelperJSON::~RESTContentHelperJSON()
{

}

#if 0
RESTContentHelper *
RESTContentHelperFactory::decodeSimpleContent( RESTRepresentation *repPtr )
{
    RESTContentHelper *chPtr;
    unsigned char     *dataPtr;
    std::string        contentType;
    unsigned long      dataLength;

    // Makes sure we recognize the content
    if( repPtr->hasSimpleContent() == false )
    {
        return NULL;
    }

    dataPtr = repPtr->getSimpleContentPtr( contentType, dataLength );

    // Determine which type of decoder we will use
    if( contentType == "application/xml" )
    {
        chPtr = new RESTContentHelperXML();
        chPtr->parseRawData( dataPtr, dataLength );
        return chPtr;
    }

    return NULL;
}

RESTContentHelper *
RESTContentHelperFactory::decodeSimpleContent( RESTContentNode *templateCN, RESTRepresentation *repPtr )
{
    RESTContentHelper *chPtr;
    unsigned char     *dataPtr;
    std::string        contentType;
    unsigned long      dataLength;

    // Makes sure we recognize the content
    if( repPtr->hasSimpleContent() == false )
    {
        return NULL;
    }

    dataPtr = repPtr->getSimpleContentPtr( contentType, dataLength );

    // Determine which type of decoder we will use
    if( contentType == "application/xml" )
    {
        chPtr = new RESTContentHelperXML();
        chPtr->parseRawData( dataPtr, dataLength );
        return chPtr;
    }

    return NULL;
}
#endif

RESTContentNode *
RESTContentHelperFactory::newContentNode()
{
    return new RESTContentNode();
}

void 
RESTContentHelperFactory::freeContentNode( RESTContentNode *nodePtr )
{
    delete nodePtr;
}

RESTContentHelper *
RESTContentHelperFactory::getRequestSimpleContentHelper( RESTRepresentation *repPtr )
{
    // Default to XML
    return new RESTContentHelperXML();
}

RESTContentHelper *
RESTContentHelperFactory::getResponseSimpleContentHelper( RESTRepresentation *repPtr )
{
    // Default to XML
    return new RESTContentHelperXML();
}

void 
RESTContentHelperFactory::freeContentHelper( RESTContentHelper *chPtr )
{
    delete chPtr;
}

