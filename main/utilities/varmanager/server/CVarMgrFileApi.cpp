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
# @file   CVarMgrFileApi.cpp
# @brief  Implement API for file database.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrFileApi.h"
#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <CVariable.h>
#include <CEnumeration.h>
#include <CStateMachine.h>
#include <CSqliteTransaction.h>
/*-----------------------------------------------------------------------------
 *  Implement our transaction as a wrapper around a CSqliteTransaction.
 */

/**
 *  constructor
 *    Wrap an SqliteTransaction.  Our destructor will destroy it:
 *  @param api - references a raw api object:
 */
CVarMgrFileApi::FileTransaction::FileTransaction(CVariableDb& api) :
    m_pTransaction(0)
{
    m_pTransaction = new CSqliteTransaction(api);    
}
/**
 * destructor - destroys the wrapped transaction.
 */
CVarMgrFileApi::FileTransaction::~FileTransaction()
{
    delete m_pTransaction;
}

/**
 * rollback
 *   delegates to the wrapped transaction
 */
void
CVarMgrFileApi::FileTransaction::rollback()
{
    m_pTransaction->rollback();
}
/**
 * scheduleRollback
 *    Delegated to the wrapped transaction.
 */
void
CVarMgrFileApi::FileTransaction::scheduleRollback()
{
    m_pTransaction->scheduleRollback();
}

/**
 * commit
 *    Commit now - delegated to the wrapped transaction
 */
void
CVarMgrFileApi::FileTransaction::commit()
{
    m_pTransaction->commit();
}


/*-----------------------------------------------------------------------------*/

/**
 * constructor
 *    Constructs an instance of the API connected to a specific database file.
 *    -   A data base object is created.
 *    -   A dirtree object is also created in order to maintain a cwd.
 * @param[in] pFilePath - Path to the file being attached.
 * @throw std::runtime_error derived exception, if the constructors for either
 *        object fail.
 */

CVarMgrFileApi::CVarMgrFileApi(const char* pFilePath) :
    m_pDb(0), m_pWd(0)
{
    try {
        m_pDb = new CVariableDb(pFilePath);
        m_pWd = new CVarDirTree(*m_pDb);
    } catch(...) {
        // clean up any created objects.
        
        delete m_pWd;
        delete m_pDb;
        
        
        throw;            // make the caller handle this.
    }
}
/**
 * destructor
 *    Destroy the dynamically allocated objects.
 */
CVarMgrFileApi::~CVarMgrFileApi()
{
    delete m_pWd;
    delete m_pDb;
    
}
/**
 * mkdir
 *   Create a new directory.
 *   @param[in] path - The path to the new directory.
 *   @throw std::runtime_error derived exceptino if the
 *          underlying m_pDb->mkdir call fails.
 *   @note -any missing intermediate diretories will be created as well.
 */
void
CVarMgrFileApi::mkdir(const char* path)
{
    m_pWd->mkdir(path);
}

/**
 * cd
 *    Change the API's default working directory to the path.
 * @param[in] path - Path for the new wd.
 * @throw std::runtime_error derived exception if e.g. the path does not exist.
 */
void
CVarMgrFileApi::cd(const char* path )
{
    m_pWd->cd(path);
}
/**
 * getwd
 *    Return the working directory path.
 *
 *   @return std::string - cwd for m_pWd
 */
std::string CVarMgrFileApi::getwd()
{
    return m_pWd->wdPath();
}
/**
 * rmdir
 *    Remove a directory:
 *  @param[in] rmdir - the directory to remove.
 *  @throw std::runtime_error derived exception if e.g. the path does not exist.
 */
void CVarMgrFileApi::rmdir(const char* path)
{
    m_pWd->rmdir(path);
}
/**
 * declare
 *    Create a new variable.
 *
 *  @param[in] path - Path to the variable... this may be relative to the cd.
 *  @param[in] type - Data type the variable holds (must be defined).
 *  @param[in] initial - Initial value string...if null (default) the variable's
 *                 default value will be used.
 *  @throw  std::runtime_error derived exception on error
 */
void
CVarMgrFileApi::declare(const char* path, const char* type, const char* initial)
{
    CVariable::create(*m_pDb, *m_pWd, path, type, initial);
}
/**
 * set
 *   Provide a new value for a varaible.
 *
 *  @param[in] path - Path to the variable (can be wd relative).
 *  @param[in] value - Proposed new value.
 *  @throw std::runtime_error derived exception on error.
 */
void
CVarMgrFileApi::set(const char* path, const char* value)
{
    CVariable v(*m_pDb, *m_pWd, path);
    v.set(value);
}
/**
 * get
 *    Return the value of a variable.
 *
 *  @param[in] path  - Path of the variable (possibly wd relative).
 *  @return std::string - Value of the variable.
 *  @throw std::runtime_error derived exception on errors.
 */
std::string
CVarMgrFileApi::get(const char* path)
{
    CVariable v(*m_pDb, *m_pWd, path);
    return v.get();
}
/**
 * defineEnum
 *    Create an enumeration data type.
 *
 * @param[in] typeName  - Name of the new type (must be unique).
 * @param[in] values    - values the type can take.
 * @throw std::runtime_error derived exception in the event of an error.
 */
void
CVarMgrFileApi::defineEnum(const char* typeName, CVarMgrApi::EnumValues values)
{
    CEnumeration::create(*m_pDb, typeName, values);
}
/**
 * defineStateMachine
 *    Define a state machine data type
 *
 *  @param[in] typeName - Name of the new data type (must be unique).
 *  @param[in] transitions - State transition map.
 *
 *  @throw std::runtime_error derived excpeption on errors.
 */
void CVarMgrFileApi::defineStateMachine(const char* typeName, CVarMgrApi::StateMap transitions)
{
    if (!validTransitionMap(transitions)) {
        throw CVarMgrApi::CException("Invalid state transition map");
    }
    CStateMachine::create(*m_pDb, typeName, transitions);
}
/**
 * ls
 *   Produce a listing of subdirectories for a path.
 *
 *  @param[in] path - Directory path
 *                  - if not supplied, the wd is listed.
 *                  - if supplied and absolute, the path is listed.
 *                  - if supplied and relative the path computed from the wd
 *                    is listed.
 *  @return std::vector<std::string> sub-directories in the path.
 */
std::vector<std::string>
CVarMgrFileApi::ls(const char* path)
{
    // Make dirtree that depends on the specific path value:
    
    CVarDirTree parent(*m_pDb);
    parent.cd(m_pWd->wdPath().c_str());
    if (path) {
        parent.cd(path);
    }
   
    std::vector<CVarDirTree::DirInfo> dirs = parent.ls();
    std::vector<std::string> result;
    
    for (int i =0; i < dirs.size(); i++) {
        result.push_back(dirs[i].s_name);
    }
    return result;
}
/**
 * lsvar
 *    List information about the variables in a directory path.
 *    @param[in] path - directory path, see ls above for how it's treated.
 *    @return std::vector<CVarMgrApi::VarInfo> - containing info about the
 *            variables in the directory.
 */
std::vector<CVarMgrApi::VarInfo>
CVarMgrFileApi::lsvar(const char* path)
{
    
    std::vector<CVariable::VarInfo> rawResult = CVariable::list(m_pDb, *m_pWd, path);
    std::vector<CVarMgrApi::VarInfo> result;
    for (int i = 0; i < rawResult.size(); i++) {
        VarInfo vInfo = {
            rawResult[i].s_name, rawResult[i].s_type, "0"    
        };
        std::string varp;
        if (path) {
            varp += path;
            if (varp != "/") { 
                varp += "/";
            }
        }   
        varp += vInfo.s_name;
        
        CVariable v(*m_pDb, *m_pWd, varp.c_str());
        vInfo.s_value = v.get();
        
        result.push_back(vInfo);
        
    }
    return result;
}
/**
 * rmvar
 *    Remove an existing variable.
 *
 *   @param path - path to the variable to remove (this can be wd relative).
 *
 *  @throw std::runtime_error derviec exception
 *         if error (e.g. the path does not exist).
 */
void
CVarMgrFileApi::rmvar(const char* path)
{
    CVariable::destroy(*m_pDb, *m_pWd, path);
}
/**
 *  transaction
 *     Factory method to produce a transaction suitable for use with our
 *     interface.  Here's a segment of code that shows typical use of this
 *     method.   Note the variable pApi is a pointer to a CVarMgrFileApi:
 * 
 *  \verbatim
 *     try {
 *        std::unique_ptr<CVarMgrApi::Transaction> t(apApi->transaction());  // begin
 *        try {
 *                // Do stuff in the database.   Can t->rollback(), t->scheduleRollback()
 *                // or explicit commit with t->commit().
 *        } catch (...) {
 *            t->rollback();                // Rollback on error.
 *              ...
 *        }                                // commits if no rollback/scheduleRollback.
 *     }
 *     catch (CVarMgrApi::Unimplemented e) { (API doesn't implement transactions) }
 *
 *  @return CVarMgrFileApi::FileTransaction*
 */
CVarMgrFileApi::Transaction*
CVarMgrFileApi::transaction()
{
    return new FileTransaction(*m_pDb);
}