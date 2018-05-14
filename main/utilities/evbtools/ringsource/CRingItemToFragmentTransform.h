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
 * @file CRingItemToFragmentTransform.h
 * @brief defines ring specific event builder data source class.
 */
#ifndef CRINGITEMTOFRAGMENTTRANSFORM_H
#define CRINGITEMTOFRAGMENTTRANSFORM_H

#include <config.h>

#include <CEVBClientApp.h>
#include <EVBFramework.h>
#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <ByteBuffer.h>

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

// Forward definitions:



namespace DAQ {


class CRingItemToFragmentTransform 
{
  // Prototype for the timestamp getter:

private:
  std::vector<std::uint32_t> m_allowedSourceIds;

  // Canonicals:

public:
  CRingItemToFragmentTransform();
  virtual ~CRingItemToFragmentTransform();

  // Main entry point
  ClientEventFragment operator()(const V12::CRawRingItem& pItem);

  // Getters and setters
  void setAllowedSourceIds(const std::vector<uint32_t>& ids)
  { m_allowedSourceIds = ids; }
  std::vector<uint32_t>& getAllowedSourceIds() { return m_allowedSourceIds; }

private:
  bool formatPhysicsEvent(const V12::CRawRingItem& p, ClientEventFragment& frag);
  void validateSourceId(uint32_t sourceId);
  bool isValidSourceId(uint32_t sourceId);
};

} // end DAQ

#endif
