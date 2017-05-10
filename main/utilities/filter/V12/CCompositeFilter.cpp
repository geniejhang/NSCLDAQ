/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include <V12/CCompositeFilter.h>
#include <make_unique.h>

namespace DAQ {
namespace V12 {

/**! The default constructor
  Does nothing but initialize an empty vector of filters
*/ 
CCompositeFilter::CCompositeFilter()
  : m_filter()
{}

/**! The copy constructor
* Performs a deep copy of the filters. Clones of each of the filters
* in the target composite are made.
*
* \param rhs the filter to copy
*/
CCompositeFilter::CCompositeFilter(const CCompositeFilter& rhs)
  : CFilter(rhs), m_filter()
{
  const_iterator it = rhs.begin();
  const_iterator itend = rhs.end();

  while (it!=itend) {
    m_filter.emplace_back((*it)->clone());
    ++it;
  } 
}

/*! Assignment

  Performs a deep copy of the target. The target's state is copied 
  entirely prior to assignment. Following this operation, this will not own pointers 
  to the same objects as were pointed to by the target's pointers. Instead, 
  the various pointers will refer to copies of the objects pointed to by the 
  target's pointers.

  \param rhs the object to copy 
  \return a reference to this after the copy operation
*/
CCompositeFilter& CCompositeFilter::operator=(const CCompositeFilter& rhs)
{
  if (this != &rhs) {
    const_iterator it = rhs.begin();
    const_iterator itend = rhs.end();

    // First fill a vector with all the newly
    // copied filters. If this fails, then we know the 
    // original filters were not modified.
    FilterContainer copy;
    while (it!=itend) {
      copy.emplace_back((*it)->clone());
      ++it;
    } 

    m_filter.clear();
    m_filter = copy;
  }

  return *this;
}

/*! Destructor */
CCompositeFilter::~CCompositeFilter()
{
}

/*! Virtual copy constructor
  Returns a dynamically allocated object whose state is a copy of this.
  Ownership of the returned object belongs to the caller. The semantics 
  of the copy operation for this class is described in the
  CCompositeFilter::CCompositeFilter(const CCompositeFilter&) method.

  \return a pointer to the new dynamically allocated object
*/
CFilterUPtr CCompositeFilter::clone() const
{
  return DAQ::make_unique<CCompositeFilter>(*this);
}

/*! Append a filter to the container
*
   The composite filter takes shared ownership of the filter.
   Filters are appended to the back of the container every
   time this method is called. As a result, the order of registration
   is preserved at execution time.

  \param filter     a shared pointer to the filter
*/
void CCompositeFilter::registerFilter(CFilterPtr filter)
{
  m_filter.push_back(filter);
}

/**! Handle a generic ring item
    The handler iterates through the set of
    registered filters and calls their respective handleRingItem(CRingItemPtr)
    methods. It is possible for a filter
    to return a pointer to the same object as was passed it and this is handled properly.

    If a filter returns nullptr or a shared ptr managing no memory,
    handlers for subsequent filters will not be called
    and processing will terminate.

    
  \param item   the ring item to process
  \return a pointer to the output of the last filter registered to the composite
*/  
CRingItemPtr CCompositeFilter::handleRingItem(CRingItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Loop through the filters while the newly returned object isn't null
  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handleRingItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle state change items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleStateChangeItem(CRingStateChangeItemPtr) is
   called.
*/
CRingStateChangeItemPtr
CCompositeFilter::handleStateChangeItem(CRingStateChangeItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handleStateChangeItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle scaler items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleScalerItem(CScalerItemPtr) is
   called.
*/
CRingScalerItemPtr CCompositeFilter::handleScalerItem(CRingScalerItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handleScalerItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle text items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleTextItem(CTextItemPtr) is
   called.
*/
CRingTextItemPtr CCompositeFilter::handleTextItem(CRingTextItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handleTextItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle physics event items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handlePhysicsEventItem(const CPhysicsEventItemPtr) is
   called.
*/
CPhysicsEventItemPtr CCompositeFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handlePhysicsEventItem(pItem);
  
    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle physics event items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr) is
   called.
*/
CRingPhysicsEventCountItemPtr CCompositeFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
  iterator it = begin(); 
  iterator itend = end(); 

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item 
    pItem = (*it)->handlePhysicsEventCountItem(pItem);
  
    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle abnormal end items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleAbnormalEndItem(CAbnormalEndItemPtr) is
   called.
*/
CAbnormalEndItemPtr CCompositeFilter::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
  iterator it = begin();
  iterator itend = end();

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered ite
      pItem = (*it)->handleAbnormalEndItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}


/**! Handle evb glom parameters items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleGlomParameters(CGlomParametersPtr) is
   called.
*/
CGlomParametersPtr CCompositeFilter::handleGlomParameters(CGlomParametersPtr pItem)
{
  iterator it = begin();
  iterator itend = end();

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item
    pItem = (*it)->handleGlomParameters(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}



/**! Handle composite items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleCompositeItem(CCompositeRingItemPtr) is
   called.
*/
CCompositeRingItemPtr CCompositeFilter::handleCompositeItem(CCompositeRingItemPtr pItem)
{
  iterator it = begin();
  iterator itend = end();

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item
    pItem = (*it)->handleCompositeItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}

/**! Handle data format items

   See handleRingItem(CRingItemPtr) documentation above. The exact same thing
   is done except that handleDataFormatItem(CDataFormatItemPtr) is
   called.
*/
CDataFormatItemPtr CCompositeFilter::handleDataFormatItem(CDataFormatItemPtr pItem)
{
  iterator it = begin();
  iterator itend = end();

  while (it!=itend && pItem) {

    // pass the first item to the filter and get the filtered item
    pItem = (*it)->handleDataFormatItem(pItem);

    // increment to the next filter
    ++it;
  }
  return pItem;
}



/**! Initialization hook to run before any data is processed
*/
void CCompositeFilter::initialize()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    (*it)->initialize();
    ++it;
  }
}

/**! Finalization hook to run after all data is processed
*/
void CCompositeFilter::finalize()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    (*it)->finalize();
    ++it;
  }
}

} // end V12
} // end DAQ

