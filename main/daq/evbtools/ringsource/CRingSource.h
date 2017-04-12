/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file CRingSource.h
 * @brief defines ring specific event builder data source class.
 */
#ifndef DAQ_CRINGSOURCE_H
#define DAQ_CRINGSOURCE_H

#include <CRingItemToFragmentTransform.h>

#include <CEVBClientApp.h>
#include <EVBFramework.h>

#include <CDataSource.h>

#include <string>
#include <vector>
#include <cstdint>

// Forward definitions:

struct gengetopt_args_info;
struct timespec;

namespace DAQ {

namespace V12 {
    class CRawRingItem;
}

/**
 * Provides experiment specific code for the Ring Buffer experiment specific
 * data source.   This takes data from the ring named --ring (TCP/IP if needed)
 * and invokes a user written timestamp extractor in the library defined by
 * --timestampextractor.
 *
 *  The timestamp extractor is event format specific and must be supplied by 
 *  the user.  It must have "C" linkage and have a single of the signature:
 * \verbatim
 *    uint64_t timestamp(pPhysicsEventItem item);
 * \endverbatim
 * 
 * The assumption is that only responses to physics triggers actually have timestamps.
 * all other ring item types either have no timestamp (scaler items e.g.) or are barrier
 * fragments (e.g. BEGIN_RUN.
 *
 */
class CRingSource : public CEVBClientApp 
{
  // attributes:

private:
  struct gengetopt_args_info* m_pArgs;
  CDataSourcePtr              m_pBuffer;
  std::vector<uint32_t>       m_allowedSourceIds;
  bool                        m_fOneshot;
  unsigned                    m_nEndRuns;
  unsigned                    m_nEndsSeen;
  unsigned                    m_nTimeout;
  unsigned                    m_nTimeWaited;

  int                         m_nTimeOffset;
  CEVBFragmentList            m_frags;
  CRingItemToFragmentTransform m_wrapper;
  
  // Canonicals:

public:
  // Constructor for testing purposes
  CRingSource(CDataSourcePtr pBuffer,
              const std::vector<uint32_t>& allowedIds);

  // Constructor for production
  CRingSource(int argc, char** argv);
  virtual ~CRingSource();

private:
  CRingSource(const CRingSource&);
  CRingSource& operator=(const CRingSource&);
  int operator==(const CRingSource&) const;
  int operator!=(const CRingSource&) const;

public:
  virtual void initialize();
  virtual bool dataReady(int ms);
  virtual void getEvents();
  virtual void shutdown();

  const CEVBFragmentList& getFragmentList() const { return m_frags; }

  void setOneshot(bool val) { m_fOneshot = val; }
  void setNumberOfSources(unsigned nsources) { m_nEndRuns = nsources; }
  bool oneshotComplete();
  void setAllowedSourceIds(const std::vector<uint32_t>& ids);

  void validateItem(const V12::CRingItem& item);

public:
  uint64_t timedifMs(struct timespec& later, struct timespec& earlier);
  void transformAvailableData();
};

} // end DAQ

#endif
