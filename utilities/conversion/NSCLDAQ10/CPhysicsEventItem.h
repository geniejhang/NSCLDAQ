#ifndef CPHYSICSEVENTITEM_H
#define CPHYSICSEVENTITEM_H
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

#include <CRingItem.h>		/* Base class */

#include <unistd.h>

#include <stdint.h>

#include <string>

namespace NSCLDAQ10
{

struct _RingItem;

/**
 *  This class is a wrapper for physics events.
 *  It's mainly provided so that textual dumps
 *  can be performed as typeName and toString
 *  are the only substantive methods...everything
 *  else just delegates to the base class.
 */

class CPhysicsEventItem : public CRingItem
{
public:
  CPhysicsEventItem(uint16_t type, size_t maxBody=8192);
  CPhysicsEventItem(const CPhysicsEventItem& rhs);
  virtual ~CPhysicsEventItem();

  CPhysicsEventItem& operator=(const CPhysicsEventItem& rhs);
  int operator==(const CPhysicsEventItem& rhs) const;
  int operator!=(const CPhysicsEventItem& rhs) const;

  // Virtual methods that all ring items must provide:

  virtual std::string typeName() const;	// Textual type of item.
  virtual std::string toString() const; // Provide string dump of the item.

 
  
};

} // end of namespace

#endif
