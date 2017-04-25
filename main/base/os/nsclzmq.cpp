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
 * @file nsclzmq.cpp
 * @brief implementation of NSCL ZMQ utility operations.
 */

#include <nsclzmq.h>
#include <stdexcept>

/*------------------------------------------------------------------------*/
/**
 * Implementation of the ZmqSocket wrapper class - a pointer-like
 * object to a zmq::socket_t.
 * This class allows for zmq::socket_t* to be automatically cleaned up
 * at end of execution.
 */

std::list<ZmqSocket*> ZmqSocket::m_sockets;

/**
 * Construtor:
 *   Construct the underlying zmq::socket_t and add it to the managed list.
 *   
 * @param[in] context - zmq::context_t singleton reference.
 * @param[in] type    - socket type (e.g. ZMQ_REQ).
 *
 */
ZmqSocket::ZmqSocket(zmq::context_t& context, int type)
{
  m_pSocket = new zmq::socket_t(context, type);
  m_sockets.push_back(this);
}
/**
 * destructor
 *   Remove ourselves from the sockets lit.
 *   Set the linger for the socket we contain to 1 second.
 *   Destroy the socket we contain (which will shut it down).
 */
ZmqSocket::~ZmqSocket()
{

  // Unregister us:
  
  auto p  = std::find(m_sockets.begin(), m_sockets.end(), this);
  if (p == m_sockets.end()) {
    /// This is really bad:

    throw std::logic_error("Could not find a ZmqSocket in the registration list at destruct time");
  }
  m_sockets.erase(p);

  // Ensure we have a small linger time:

  int lingerPeriod(1000);                    // milliseconds:
  m_pSocket->setsockopt(ZMQ_LINGER, &lingerPeriod, sizeof(int));

  delete m_pSocket;                            // Kill off the socket and closes it.
}

/**
 * operator* 
 *  Return a reference to the socket we manage allows e.g. (*me).recv()
 *
 * @return zmq::socket_t&
 */
zmq::socket_t&
ZmqSocket::operator*()
{
  return *m_pSocket;
}
/**
 * operator->
 *   Return  pointer to the socket we manage allows e.g. me->recv();
 *
 * @return  zmq::socket_t*
 */
zmq::socket_t*
ZmqSocket::operator->()
{
  return m_pSocket;
}


/**
 * shutdown
 *   System shutdown 
 *   Deletes all of the sockets in m_sockets.
 *   note that socket destruction invalidates iterators.
 */
void
ZmqSocket::shutdown()
{
  while(!m_sockets.empty()) {
    delete m_sockets.front();            // Removes front from list.
  }
}



/*--------------------------------------------------------------------------------------*/

/**
 *  Implementation of ZmqObjectFactory
 */


zmq::context_t* ZmqObjectFactory::m_pContext(0);

/**
 *   getContextInstance.
 * get zmq::context_t singleton:
 *
 * @return zmq::context_t*
 */
zmq::context_t*
ZmqObjectFactory::getContextInstance()
{
  if (!m_pContext) {
    m_pContext = new zmq::context_t(5);
  }
  return m_pContext;
}
/**
 * createSocket
 *   Create a new socket object:
 *
 * @param type - type of socket (e.g. ZMQ_PUB.
 * @return ZmqSocket* - pointer like object to the new managed zmq::socket_t.
 */
ZmqSocket*
ZmqObjectFactory::createSocket(int type)
{
  return new ZmqSocket(*getContextInstance(), type);
}

/**
 * shutdown
 *   Shutdown the system.
 *  -  Shutdown all sockets.
 *  -  Shutdown the context.
 */

void
ZmqObjectFactory::shutdown()
{
  ZmqSocket::shutdown();
  zmq::context_t* pContext = m_pContext;
  m_pContext = 0;
  delete pContext;

}

/**
 * @class ZmqCleanup
 *   The only purpose of this class is to be statically instantiated and then to have 
 *   a destructor that shutsdown the ZmqObjectFactory.
 */
class ZmqCleanup
{
public:
   ~ZmqCleanup() {ZmqObjectFactory::shutdown();}
};

/**
 *  This object will be created the first time code in this module is
 *  Invoked.  If constructed, program exit will invoke the ZmqCleanup
 *  destructor.  That in turn will close all zmq sockets and kill off]
 *  the zmq context.
 */

static ZmqCleanup cleanup;
