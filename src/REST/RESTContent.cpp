#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <list>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "REST.hpp"

RESTContentNode::RESTContentNode()
{

}

RESTContentNode::~RESTContentNode()
{

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
RESTContentNode::setField( std::string name, std::string value )
{
    if( fieldValues.count( name ) > 0 )
    {
        fieldValues[name] = value;
        return;
    }

    fieldValues.insert( std::pair< std::string, std::string >( name, value ) );
}

bool 
RESTContentNode::clearField( std::string name )
{
    fieldValues.erase( name );
}

bool 
RESTContentNode::getField( std::string name, std::string &value )
{
    value.clear();

    if( fieldValues.count( name ) > 0 )
    {
       value = fieldValues[ name ];
       return false;
    }

    return true;
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
RESTContentHelperXML::parseRepSimple( RESTRepresentation *repPtr )
{
    unsigned long dataLength;
    std::string contentType;
    unsigned char *dataBuf;

    xmlDocPtr   doc;
    xmlNode    *rootElem;

    std::cout << "RESTContentHelperXML::parseRepSimple -- start" << std::endl;

    if( repPtr->hasSimpleContent() == false )
    {
        return true;
    }

    dataBuf = repPtr->getSimpleContentPtr( contentType, dataLength );

    if( contentType != "application/xml" )
    {
        return true;
    }

    // Run the simple content through the xml parser
    doc = xmlParseMemory( (const char *)dataBuf, dataLength );
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
RESTContentHelperXML::parseRepEncoded( std::string key, RESTRepresentation *repPtr )
{

}

RESTContentHelperJSON::RESTContentHelperJSON()
{

}

RESTContentHelperJSON::~RESTContentHelperJSON()
{

}

