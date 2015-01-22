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

#ifndef __READER_H
#define __READER_H

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif

// Forward classes.

class ReadoutStateMachine;
class CTrigger;
class CBusy;


class CReader
{
  // private data:
private:
  mutable 
    ReadoutStateMachine&   m_rManager; //!< State machine (some services)
  mutable 
    DAQWordBuffer*         m_pBuffer;  //!< Current buffer.
  mutable 
    DAQWordBufferPtr       m_BufferPtr; //!< Cursor into current buffer.
  unsigned int             m_nEvents;  //!< Current Event count.
  unsigned int             m_nWords;   //!< Current word count.
  unsigned int             m_nBufferSize; //!< System buffersize (High w.mrk).

  mutable CTrigger*        m_pTrigger; //!< Trigger manager.
  mutable CBusy*	   m_pBusy;    //!< Dead-time module.
  // Constructors and other canonical functions.

public:
  CReader(ReadoutStateMachine& rManager);
  virtual ~CReader() {}
private:
  CReader(const CReader& rRhs);	//!< Copy constructor illegal.
  CReader& operator=(const CReader& rRhs); //!< Assignment illegal.
  int     operator==(const CReader& rRhs) const; //!<< Comparison senseless.
  int     operator!=(const CReader& rRhs) const; //!<< Comparison senseless.
public:

  //  Selectors:

public:
  ReadoutStateMachine& getManager() const {
    return m_rManager;
  }
  DAQWordBuffer* getBuffer() const {
    return m_pBuffer;
  }
  DAQWordBufferPtr getBufferPointer() const {
    return m_BufferPtr;
  }
  unsigned int getEventCount() const {
    return m_nEvents;
  }
  unsigned int getWordCount() const {
    return m_nWords;
  }
  unsigned int getBufferSize() const {
    return m_nBufferSize;
  }
  CTrigger* getTrigger() const {
    return m_pTrigger;
  }
  CBusy* getBusy() const {
    return m_pBusy;
  }

  // Simple mutators:

protected:
  void setBuffer(DAQWordBuffer* pBuffer) {
    m_pBuffer    = pBuffer;
  }
  void setBufferPointer(DAQWordBufferPtr& rBufferPtr) {
    m_BufferPtr = rBufferPtr;
  }
  void setEventCount(unsigned int nEvents) {
    m_nEvents = nEvents;
  }
  void setWordCount(unsigned int nWords) {
    m_nWords = nWords;
  }
  void setBufferSize(unsigned int nBufferSize) {
    m_nBufferSize;
  }
public:
  void setTrigger(CTrigger* pTrigger) {
    m_pTrigger = pTrigger;
  }
  void setBusy(CBusy* pBusy) {
    m_pBusy = pBusy;
  }

  // Invariant functions.

public:
  void Enable();
  void Disable();
  void ReadSomeEvents(unsigned int nPasses);
  void FlushBuffer();

  // Utility functions.

private:
  void OverFlow(DAQWordBufferPtr& rLastEventPtr);

};

#endif

