/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#include <config.h>
using namespace std;

#include "CBufferManager.h"
#include <string>
#include <stdint.h>
#include <Iostream.h>

/*!
 *   Create the buffer manager. It is not the responsibility
 *   of the constructor to schedule the thread.  When 'the time is right'
 *   the start() member must be called to schedule execution.
 * \param size (Defaults to 8192) The number of bytes for each
 *              buffer.
 * \param count (Defaults to 10) The number of buffers to maintain.
 * 
 */
CBufferManager::CBufferManager(size_t size, size_t count) :
	m_bufferSize(size),
	m_nInitialBufferCount(count),
	m_Tid(0),
	m_fRunning(false)
{
	
}
/*!
 *   If the buffer manager is destroyed, it's probably a bad
 * thing...we'll complain via a string exception.
 */
CBufferManager::~CBufferManager()
{
	throw string("Buffer managers in general can't be destroyed!!!")
}
/*!
 * Return the current buffersize.  This can be done in the 
 * executing (client) thread:
 * \return size_t
 * \retval number of bytes in buffers managed by the  manager.
 */
size_t
CBufferManager::getBufferSize() const
{
	return m_bufferSize;
}
/*!
 *   Set the buffersize. This must be done in the context of the
 *   buffer manager thread as the existing buffers in the
 *   buffer queue must be removed and destroyed.
 *   Therefore we send a resize message to the 
 *   manager and continue on our merry parallel way.
 *   To ensure that this is done, some time later it's probably
 *   a good thing to ask about the buffersize.
 * \param newSize  The new buffersize.
 */
size_t
CBufferManager::setBufferSize(size_t newSize)
{
	CommandElement cmd;
	cmd.s_Command = resize;
	cmd.s_Data.u_Size = newSize;
	m_Commands.queue(cmd);
}
/*!
 *   Return the number of buffers being managed by the buffer
 *   manager thread.  This is the initial number of buffers created,
 *   after each buffer is in any way released by the user, a replacement
 *   is created (well actually freed ones that match the current size
 *   are just reinserted in the queue while routed ones must be destroyed
 *   and replaced).
 *  \return size_t
 *  \retval Number of buffers being maintained.
 */
size_t
CBufferManager::getBufferCount() const
{
	return m_nInitialBufferCount;
}
/*!
 *    Sets a new number of buffers managed.
 *    Since this will involve creating/freeing buffers,
 *    this must be done in the thread context of the
 *    buffer manager.
 *   \param newCount New numberof buffers to manage.
 */
void
CBufferManager::setBufferCount(size_t newCount)
{
	CommandElement cmd;
	cmd.s_Command     = changecount;
	cmd.s_Data.u_Size = newCount;
	m_Commands.queue(cmd);
}
/*!
 *     Allocates a buffer by fetching a pointer to the buffer
 *     from the queue.  The queue get operation
 *     will block if needed until a buffer is available, however
 *     best it to have the manager manage enough buffers that 
 *     there are always sufficient buffers available.
 * \return DAQWordBuffer*   
 * \retval  The buffer from the queue.
 */
DAQWordBuffer*
CBufferManager::allocateBuffer()
{
	return m_Buffers.get();
}
/*!
 *    Returning a buffer without actually routing it
 *    requires support from the buffer manager.. this is so that
 *    if the buffer being returned is not the same size as the
 *    current buffersize (it may have been in-flight), the
 *    buffer is destroyed and a new one created.
 *    \param pBuffer  - Pointer to the buffer to be released.
 */
void
CBufferManager::freeBuffer(DAQWordBuffer* pBuffer)
{
	CommandElement cmd;
	cmd.s_Command        = free;
	cmd.s_Data.u_pBuffer = pBuffer;
	m_Commands.queue(cmd);
}
/*!
 *   Route a buffer.  Buffer can only be routed by the thread
 *   that created them so, we interact with the manager thread.
 * \param pBuffer - Pointer to the buffer to route.
 */
void
CBufferManager::route(DAQWordBuffer* pBuffer)
{
	CommandElement cmd;
	cmd.s_Command         = routeBuffer;
	cmd.s_Data.u_pBuffer  = pBuffer;
	m_Commands.queue(cmd);
}
/*!
 *   Start the buffer manager thread.  Getting the buffer
 *  manager started is a 2 stage operation. First construction,
 *  then initiation.
 */
void
CBufferManager::start()
{
	m_Tid = daq_dispatcher.Dispatch(*this);
}

/////////////////////////////////////////////////////////////////
//  Entry  point to the buffer manager.
//  Create the innitial set of buffers and queue them
//  in the buffer queue, mark ourselves running and then
//  loop waiting for, and processing commands from clients.

int
CBufferManager::operator()(int argc, char** argv)
{
	create(m_nInitialBufferCount);
	m_fRunning = true;
	
	
	while (1) {
		CommandElement cmd = m_Commands.get();  // blocks.
		switch (cmd.s_Command) {
		case routeBuffer:
			// The buffer must be routed,
			// destroyed, and a new one created to replace it.
			
			DAQWordBuffer* pBuffer = cmd.s_Data.u_pBuffer;
			pBuffer->Route();
			delete pBuffer;
			create();
			break;
		case free:
			// If the buffer has the right size, just put it in the
			// queue...else destroy and make a new one:
			
			DAQWordBuffer* pBuffer = cmd.s_Data.u_pBuffer;
			if (pBuffer->GetLen() == m_bufferSize/sizeof(uint16_t)) {
				m_Buffers.queue(pBuffer);
			}
			else {
				delete pBuffer;
				create();
			}
			break;
		case resize:
			// Resizing the buffer requires killing off the existing buffer
			// queue and allocating a whole new set of buffers:
			
			m_bufferSize = cmd.s_Data.u_Size;
			clear();
			create(m_nInitialBufferCount);
			break;
			
		case changecount:
			// Change the number of buffers that are pre-allocated.
			// If we're getting fewer buffers (not likely) we 
			// get buffers from the queue and free them.
			// if more (likely) we just add more to the queue.
			
			size_t newBufferCount = cmd.s_Data.u_Size;
			int    difference     = newBufferCount - m_nInitialBufferCount;
			
			if(difference > 0) {
				// Add buffers.
				
				create(difference);
				m_nInitialBufferCount = difference;
			}
			else {
				// Delete buffers.

				difference = - difference;            // # to delete.
				m_nInitialBufferCount = newBufferCount;
				for (int i=0; i < difference; i++) {
					DAQWordBuffer* pBuffer = allocateBuffer();   // We can allocate too!
					delete pBuffer;
				}
			}
			break;
		default:
			// Really bad thing... ignore with a cerr.
		
			cerr << "Buffer manager received an invalid command: "
				 << cmd.s_Command << " Ingoring\n";
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////
/*
 * Clears all the buffers from the buffer queue and deletes them.
 * If there are any bufferes in-flight to other threads, they will typically
 * be reclaimed when routed or freed.
 */
void
CBufferManager::clear()
{
	list<DAQBWordBuffer*> buffers = m_Buffers.getAll();
	list<DAQWordBuffer*>::iterator i = buffers.begin();
	while (i != buffers.end()) {
		delete *i;
		i++;
	}
}
////////////////////////////////////////////////////////////////////////////////////
/*
 * 
 * Creates a set of daqword buffers and inserts them into m_Buffers.
 */
void
CBufferManager::create(size_t n)
{
	for (int i =0; i < n; i++) {
		m_Buffers.queue(new DAQWordBuffer(m_bufferSize));
	}
}
/*
 *  Get an element from the command queue.
 */
