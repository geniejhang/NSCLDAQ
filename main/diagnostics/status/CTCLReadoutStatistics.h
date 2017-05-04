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
# @file   CTCLReadoutStatistics.h
# @brief  TCL Encapsulation of CStatusDefinitions::ReadoutStatistics
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLREADOUTSTATISTICS_H
#define CTCLREADOUTSTATISTICS_H

#include <TCLObjectProcessor.h>
#include <zmq.hpp>
#include "CStatusMessage.h"
#include <map>
#include <nsclzmq.h>

class CTCLInterpreter;
class CTCLObject;

/**
 *  @class CTCLReadoutStatistics
 *    Encapsulates the CStatusDefinitions::ReadoutStatistics class as follows:
 *    This outer class CTCLReadoutStatistics provides a new command that
 *    can instantiate and destroy wrapped instances of
 *    CStatusDefinitions::ReadoutStatistics.  Our inner class TCLReadoutStatistics
 *    implements the Tcl wrapper around that class.
 *    @note - Sockets are wrapped in auto shutdown pointer like objects
 *            produced by ZmqObjectFactory.  This also ensures a single ZMQ
 *            context for the whole app.
 */
class CTCLReadoutStatistics : public CTCLObjectProcessor
{
    // Data types:
private:
    class TCLReadoutStatistics;
    typedef std::map<std::string, TCLReadoutStatistics*>
        Registry;
    
    
    // Member data:
private:
    static unsigned m_instanceCounter;             // Used to create unique cmds.
    Registry        m_registry;
    bool            m_testing;                  // For testing - affects socket type.
    // Canonicals:
public:    
    CTCLReadoutStatistics(CTCLInterpreter& interp, const char* cmd = "ReadoutStatistics");
    virtual ~CTCLReadoutStatistics();
    
    // Methods:
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void enableTesting() {m_testing = true; }
    void disableTesting() {m_testing = false;}
    
    // Subcommand executors:
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Class to encapsulate instances:
private:
    class TCLReadoutStatistics : public CTCLObjectProcessor {
    private:
        CStatusDefinitions::ReadoutStatistics* m_pObject;
        ZmqSocket&                             m_pSocket;
    public:
        TCLReadoutStatistics(
            CTCLInterpreter& interp, const char* cmd,
            CStatusDefinitions::ReadoutStatistics* object, ZmqSocket& sock
        );
        virtual ~TCLReadoutStatistics();
        
    public:
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void beginRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void emitStatistics(
            CTCLInterpreter& interp, std::vector<CTCLObject>& objv
        );
    };
};

#endif
