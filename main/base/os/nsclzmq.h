/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file nsclzmq.h 
 * @brief  NSCL ZMQ utility operations.
 */


#ifndef NSCLZMQ_H
#define NSCLZMQ_H

#include <zmq.hpp>
#include <list>


/**
 * Pointer like wrapper for a zmq::socket_t
 *
 * Creation Should be crated by the ZmqObjectFactory.
 * Destruction implies the objet will be unregistered from the factory.
 */
class ZmqSocket {
private:
  zmq::socket_t*    m_pSocket;
  static std::list<ZmqSocket*> m_sockets;                // socket registry.
public:
  ZmqSocket(zmq::context_t& context, int type);
  virtual ~ZmqSocket();

  zmq::socket_t& operator*();
  zmq::socket_t* operator->();

  static void shutdown();
  
};


/**
 *
 * @class ZmqObjectFactory
 *
 *   This class is responsible for creating zmq::socket_t objects
 *   It does that in a way that 
 *   - Ensures that a zmq::context_t object is singleton.
 *   - Ensures that at the exit of the program, all sockets are
 *     shutdown properly.
 *   - Ensures that once all sockets are shutdown properly at the end of the
 *      program the singleton zmq::context_t is destroyed.
 */
class ZmqObjectFactory {
private:
  static zmq::context_t*       m_pContext;               // Singleton context.

  ZmqObjectFactory();                                    // Private constructor.
public:

  // Factory methods:
  
  static zmq::context_t*   getContextInstance();           // in case someone wants it.
  static ZmqSocket*        createSocket(int type);         // Create a socket wrapper.


  static void shutdown();                                  // Shutdown the system.
};

#endif
