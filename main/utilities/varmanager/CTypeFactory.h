/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTypeFactory.h
# @brief  Define extensible factory for data types.
# @author <fox@nscl.msu.edu>
*/
#ifndef _CTYPEFACTORY_H
#define _CTYPEFACTORY_H

#include <CExtensibleFactory.h>
#include <list>

class CDataType;
typedef  CCreator<CDataType> CDataTypeCreator;
class CSqlite;
class CVariableDb;
class CUnknownTypeHandler;

/**
 * @class CTypeFactory
 *   Can return data types for specific variable types.  Data type objects
 *   are then capable of performing basic type checking on inputs, as well
 *   as some simple database services.
 *
 */
class CTypeFactory : public CExtensibleFactory<CDataType>
{

    // Object data:
private:    
    CSqlite&                         m_db;
    std::list<CUnknownTypeHandler*>  m_typeUnknownHandlers;
    
    // Static methods:
public:
    static void createSchema(CSqlite& db);
    
    // Canonicals:
    
    CTypeFactory(CSqlite& db);
    virtual ~CTypeFactory();
    
    // Overrides of virtual methods:
    
    CDataType* create(std::string type);        // Override to allow catchall.
    
    // Object methods:
    
    void addUnknownTypeHandler(CUnknownTypeHandler* pHandler);
    
    // Private utilities:
private:
    void registerDefaultCreators();
    void registerDefaultUnknownHandlers();
    
    
    
};

#endif
