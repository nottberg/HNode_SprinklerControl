#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <list>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

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
ScheduleConfig::findFieldValue( std::string fieldName, RESTContentTemplate *cnPtr, void *docPtr, void *nodePtr )
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
                //cnPtr->setField( (const char *)curElem->name, (const char *)contentValue );

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

#define MY_ENCODING "ISO-8859-1"

bool 
ScheduleConfig::writeConfig( std::string filePath, RESTContentNode *cnObj )
{
    RESTContentNode *rootCN = cnObj;
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

    generateChildContent( rootCN );

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

bool 
ScheduleConfig::readConfig( std::string filePath, RESTContentTemplate *cnObj )
{
    xmlDocPtr   doc;
    xmlNode    *rootElem;
    xmlNode    *ruleListElem;
    xmlNode    *zgListElem;
    xmlNode    *tgListElem;
    xmlNode    *curElem;

    RESTContentIDStack idStack;

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
	    return true;
    }

    std::cout << "ScheduleConfig::readConfig" << std::endl;

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Go through the config data.
    parseTree( doc, rootElem, idStack, cnObj, true );

    // Free the config document
    xmlFreeDoc(doc);

    return false;
}


