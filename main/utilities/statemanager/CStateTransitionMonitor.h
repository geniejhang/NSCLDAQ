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
# @file   CStateTranitionMonitor.h
# @brief  Monitor state transitions across the global and local state machines
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATETRANSITIONMONITOR_H
#define CSTATETRANSITIONMONITOR_H

#include <vector>
#include <string>


#include <CBufferQueue.h>
#include <CGaurdedObject.h>
#include <CSynchronizedThread.h>
#include <stdexcept>


class CVarMgrApi;
class CVarMgrSubscriptions;

/**
 * @class CStateTransitionMonitor
 *
 *    This class allows a program to monitor state transitions in the global state
 *    as well as the individual states of programs.  It follows the monitor thread
 *    message queue pattern of many of this sort of class:  A monitor thread
 *    does the actual subscription and zmq processing decodning the notifications
 *    into messages that are put into a buffer queue.  The main thread can then
 *    block with timeout on the message queue getting the next change.  This model
 *    allows monitoring to be placed into other event loops (e.g. Tcl or Qt) as well
 *    as into pretty simple C/C++ programs.
 *    
 */
class CStateTransitionMonitor : public CGaurdedObject
{
    // Exported data types:
    
public:
    // We provide the following types of notifications:
    
    typedef enum _NotificationType {
        GlobalStateChange,                             // Change to global state variable.
        ProgramStateChange,                            // Change to an individual program's state.
        ProgramJoins,                                  // Program joins the system.
        ProgramLeaves,                                 // Program Leaves the system.
        VarChanged,                                    // A variable changed.
    } NotificationType;
    
    // Here's the message that's queued up to the main thread:
    
    typedef struct _Notification {
        NotificationType    s_type;
        
        // The fields below mostly mean what they say the mean but for some
        // Values of s_type these are repurposed:
        // s_state is unused for {ProgramJoins, ProgramLeaves}.
        // s_state is a variable path for VarChanged
        // s_program is the variable new value for VarChanged.
        
        std::string         s_state;
        std::string         s_program;                // Used for all but GlobalState changes.
    } Notification, *pNotification;
        
    // Private data types

private:
    typedef CBufferQueue<Notification>  NotificationQ;
    class   MonitorThread;
    
    // private data:
private:
    CVarMgrApi*           m_pRequestApi;
    CVarMgrSubscriptions* m_pSubscriptions;
    NotificationQ         m_notifications;
    std::string           m_programParentPath;
    MonitorThread*        m_pMonitor;
    
    // Canonicals
    
public:
    CStateTransitionMonitor(const char* reqURI, const char* subURI);
    virtual ~CStateTransitionMonitor();
    
    // operations available to clients:
    
    std::string programParentDir() const {return m_programParentPath; }
    CVarMgrApi* getApi(){ return m_pRequestApi; }
    
    std::vector<std::string> activePrograms();
    std::vector<std::string> allPrograms();
    bool                     isStandalone(const char* programName);
    bool                     isEnabled(const char* programName);
    int                      transitionTimeout();
    void                     setTransitionTimeout(int secs);
    
    std::vector<Notification>  getNotifications(
        int maxNotifixcations = -1, int timeout = -1
    );
    void updateProgramParentPath(const char* path);
    
    // Communication methods that are not intended for use by external clients:
    
public:
    void postNotification(Notification msg);

    // Errors throw this:
public:    
    class CException : public std::runtime_error {
    public:
        CException(std::string what) noexcept : runtime_error(what) {}
    };
    
    // The thread:
    
private:
    class MonitorThread : public CSynchronizedThread
    {
        
    private:
        std::string              m_parentDir;
        bool                     m_exiting;
        CVarMgrSubscriptions*    m_pApi;
        CStateTransitionMonitor* m_pParent;
        
    public:
        MonitorThread(
            CVarMgrSubscriptions* psubAPI, CStateTransitionMonitor* parent
        );
        ~MonitorThread();
        void init();
        void operator()();
        void scheduleExit() {m_exiting = true;}
        
    private:
        std::string programFromVarPath(std::string varpath);
    };
    
    // Utilities:
private:
    void createReqAPI(const char* uri);
    void createSubAPI(const char* uri);
    void locateParentPath();
    void releaseResources();
    bool isActive(std::string name);
    bool getBool(std::string program, std::string name);
    std::string getVar(const char* program, const char* name);
    std::string varPath(const char* program, const char* name);
    
    
    void startMonitorThread();
};
#endif