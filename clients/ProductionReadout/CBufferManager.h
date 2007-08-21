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


#ifndef CBUFFERMANAGER_H_
#define CBUFFERMANAGER_H_


using namespace std;

#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#ifndef __CBUFFERQUEUE_H
#include <CBufferQueue.h>

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif


/*!
 *   CBufferManager is a thread that attempts to maintain a set
 *   of spectrodaq buffers available for the readout thread.  It does
 *   this by running as a separate thread, allocating buffers and
 *   placing them in a buffer queue for use by clients.  Since
 *   the allocating thread must route buffers, we provide rendezvous for
 *   routing buffers in the context of the allocating thread.
 *   The assumption is that any buffer allocated by a thread must be
 *   either routed or freed... we provde methods for both which involve
 *   waking up the allocator thread to eventually replace the allocated buffer.
 */ 
class CBufferManager : public DAQThread
{
		// Public data types:
	
	/*! Operations that can be queued to the manager thread: */
		
	typdef enum _CommandCode {   
			route,
			free,
			resize,
			changecount
	
	} CommandCode;
	/*! Command queue objects */
	
	typedef struct _CommandElement {
		CommandCode    s_Command;
		union {
			size_t          u_Size
			DAQWordBuffer*  u_pBuffer;
			
		}              s_Data
	
		
	} CommandElement;
	
	typedef CBufferQueue<DAQWordBuffer>  BufferQueue;
	typedef CBufferQueue<CommandElement> CommandQueue;
	
		// Object member data:
private:
	size_t     m_bufferSize;          //!< Size of buffers created by this manager (bytes)
	size_t     m_nInitialBufferCount; //!< Number of buffers to maintain.
	DAQTrheadId m_Tid;                //!< Manager's thread id.
	bool        m_fRunning;           //!< Manager is running.
	
	BufferQueue  m_Buffers;
	CommandQueue m_Commands;
public:
	// Construtors and other canonicals.
	CBufferManager(size_t size=8192, size_t count=10); 
	virtual ~CBufferManager();
	
	// Copy like operations cannot be supported by thread objects:
	
private:
	CBufferManager(const CBufferManager& rhs);
	CBufferManager& operator=(const CBufferManager& rhs);
	int operator==(const CBufferManager& rhs) const;
	int operator!=(const CBufferManager& rhs) const;
public:
	
	/*  We can set and fetch the buffersize. */
	
	size_t getBufferSize() const;
	void   setBufferSize(size_t newSize);
	
	/*  We can set and fetch the number of buffers we're trying to maintain: */
	
	size_t getBufferCount() const;
	void   setBufferCount(size_t newCount);
	
	/*  We can allocate buffers, free them and route them */
	
	DAQWordBuffer* allocateBuffer();
	void           freeBuffer(DAQWordBuffer* pBuffer);
	void           route(DAQWordBuffer* pBuffer);
	
	// Start the manager:
	
	void start();
	
	// The thread entry is internal.
protected:
	int operator()(int argc, char** argv);
	
	// Utilities:
private:
	void            clear();                  // Empty the buffer queue.
	void            create(size_t n=1);       // Create some buffers.
};

#endif /*CBUFFERMANAGER_H_*/
