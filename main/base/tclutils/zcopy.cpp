/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  zcopy.cpp
 *  @brief: Provides a Tcl callable package that supports zero copy initialization
 */

/**
 * The problem we're trying to solve here is that servers must have initialized
 * their listen socket to support zero copy transmission prior to accepting
 * a service socket.  This package provides the ability to set/check the SO_ZROCOPY
 * socket option on a channel.
 *   Commands:
 *   -   zcopy::enable chan-name   - Enable zero copy
 *   -   zcopy::check  chan-name   - true if zero copy is set.
 *   -   zcopy::issocket  chan-name - True of the channel is a socket.
 *  
 */
#include <tcl.h>
#include <TCLObjectProcessor.h>
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <system_error>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////
// Static utility functions that are needed across command bounds.
/**
 * isSocket
 * Internal function - check that a file descriptor is a socket.
 * @param fd - file descriptor.
 * @return bool - true if fd is a file desriptor.
 * @throws std::system_error if there's an error in fstat.
 */
static bool
isSocket(int fd)
{
    struct stat statbuf;
    if (fstat(fd, &statbuf)) {
        throw std::system_error(errno, std::generic_category(), "Stat check failed");
    }
    return S_ISSOCK(statbuf.st_mode);
}   

/**
 * channelToFd
 *   Internal function to turn a channel into a file descriptor.
 * @param interp      - interpreter the channel is defined on.
 * @param channelName - name of the channel.
 * @param mode        - Desired channel mode: TCL_READABLE or TCL_WRITABLE
 *                      in theory for sockets this does not matter, but
 *                      Tcl allows a channel to use different file descriptors
 *                      for read/write.
 * @return int        - integer
 * @throw std::invalid_argument - if the channel can't be converted.
 */
static int
channelToFd(CTCLInterpreter& interp, const char* channelName, int mode)
{
    int modes;
    Tcl_Channel chan =
        Tcl_GetChannel(interp.getInterpreter(), channelName, &modes);
    if (!chan) {
        std::string msg("There is no channel named: ");
        msg += channelName;
        throw std::invalid_argument(msg);
    }
    
    ClientData pResult;
    int status = Tcl_GetChannelHandle(chan, mode, &pResult);
    if (status != TCL_OK) {
        std::string msg("Unable to get an file descriptor for : ");
        msg += channelName;
        throw std::invalid_argument(msg);
    }
    return reinterpret_cast<uintptr_t>(pResult);
}


/////////////////////////////////////////////////////////////////////////
// Class declarations.
namespace zcopy {
    /**
     * @class zcopy::enable
     *    Defines and implements the zcopy::enable operation.
     */
    class enable : public CTCLObjectProcessor {
    friend std::string getName(
        CTCLObjectProcessor& cmd, CTCLInterpreter& interp,
        std::vector<CTCLObject>& objv
    );
    public:
        enable(CTCLInterpreter& interp, const char* command);
        virtual ~enable();
        
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
    /**
     *  @class zcopy::check 
     *     Defines and implements the zcopy::check command.
     */
    class check : public CTCLObjectProcessor {
    friend std::string getName(
        CTCLObjectProcessor& cmd, CTCLInterpreter& interp,
        std::vector<CTCLObject>& objv
    );
    public:
        check(CTCLInterpreter& interp, const char* command);
        virtual ~check();
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
    /**
     * @class zcopy::issocket
     *    Define and implement the zcopy::issocket command.
     */
    class issocket : public CTCLObjectProcessor {
    friend std::string getName(
        CTCLObjectProcessor& cmd, CTCLInterpreter& interp,
        std::vector<CTCLObject>& objv
    );
    public:
        issocket(CTCLInterpreter& interp, const char* command);
        virtual ~issocket();
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
    

/////////////////////////////////////////////////////////////////////////
// Implementation of the zcopy::enable class:

/**
 * constructor - let the base class create the command in the interpreter.
 * @param interp - reference to the interpreter on which to create the command.
 * @param command - The command string (zcopy::enabel).
 */
enable::enable(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, kfTRUE)
{}
/**
 * destructor
 *    - provided for destructor chaining.
 */
enable::~enable()
{}

/**
 * operator()
 *    Implement the command -
 *    - Translate the channel name.
 *    - Check that the channel is a socket.
 *    - set the SO_ZEROCOPY socket option.
 * @note that all errors will be signalled via std::exception derived
 *      objects, converted into a result and result in a TCL_ERROR return.
 * @param interp -interpreter that's running the command.
 * @param objv   -COmmand line words.
 * @return  int
 */
int
enable::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireExactly(objv, 2);    
    
        std::string channelName = objv[1];
        
        int fd = channelToFd(interp, channelName.c_str(), TCL_WRITABLE);
        if (!isSocket(fd)) {
            std::string msg("Channel: ");
            msg += channelName;
            msg += "  is not a socket";
            throw std::invalid_argument(msg);
        }
        int one = 1;                   // Since I need to pass a pointer
#ifdef SO_ZEROCOPY	
        if (setsockopt(fd, SOL_SOCKET, SO_ZEROCOPY, &one, sizeof(one))) {
            throw std::system_error(
                errno, std::generic_category(), "Failed to set socket option"
            );
        }
#endif        
    }
    catch (std::string& msg) {   // requireXxx methods throw these.
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;

}
/////////////////////////////////////////////////////////////////////////////
// Implement check

/**
 * constructor
 */
check::check(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, kfTRUE)
{}

/**
 * destructor
 */
check::~check()
{}

/**
 * operator()
 *    - get the channel name.
 *    - Translate it into an fd.
 *    - ensure it's a socket.
 *    - Check the state of SO_SOCKET SO_ZEROCOPY.
 * @param interp - interpreter.
 * @param objv   - Command line words.
 * @return int
 */
int
check::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireExactly(objv, 2);    
    
        std::string channelName = objv[1];
        int fd = channelToFd(interp, channelName.c_str(), TCL_WRITABLE);
        if (!isSocket(fd)) {
            std::string msg("Channel: ");
            msg += channelName;
            msg += (" is not a socket");
            throw std::invalid_argument(msg);
        }
        
        int state(0);
        socklen_t size;
#ifdef SO_ZEROCOPY	
        if (getsockopt(fd, SOL_SOCKET, SO_ZEROCOPY, &state, &size)) {
            throw std::system_error(
                errno, std::generic_category(), "Failed to get socket option"
            );
        }
#else
	state = 0;
#endif	
        if (state) {
            interp.setResult("1");
        } else {
            interp.setResult("0");
        }
    }
    catch (std::string& msg) {   // requireXxx methods throw these.
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;
}

///////////////////////////////////////////////////////////////////////
// Implement the issocket class.

/**
 * constructor:
 */
issocket::issocket(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, kfTRUE)
{}

issocket::~issocket()
{    
}

/**
 * operator()
 *    Execute the command:
 *    -  Get the channel name.
 *    -  Convert it to a file descriptor.
 *    -  Find out if it's a socket.
 * @param interp - interpreter running the command.
 * @param objv   - command line parameter.
 * @return int
 */
int
issocket::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireExactly(objv, 2);    
    
        std::string channelName = objv[1];
        int fd = channelToFd(interp, channelName.c_str(), TCL_WRITABLE);
        if (isSocket(fd)) {
            interp.setResult("1");
        } else {
            interp.setResult("0");
        }
        
    }
    catch (std::string& msg) {   // requireXxx methods throw these.
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;
}



///////////////  end zcopy namespace //////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////////
// Package initialization:

extern "C" {
    int Zcopy_Init(Tcl_Interp* pInterp)
    {
        Tcl_PkgProvide(pInterp, "zcopy", "1.0");
        Tcl_Namespace* pNs;
        CTCLInterpreter& rInterp(*(new CTCLInterpreter(pInterp)));
        if (!(pNs = Tcl_CreateNamespace(pInterp, "::zcopy", nullptr, nullptr))) {
            rInterp.setResult("Failed to create zcopy namespace");
            return TCL_ERROR;
        }
        
        // Create the commands:
        
        new zcopy::enable(rInterp, "zcopy::enable");
        new zcopy::check(rInterp, "zcopy::check");
        new zcopy::issocket(rInterp, "zcopy::issocket");
        
        
        return TCL_OK;
    }
}
