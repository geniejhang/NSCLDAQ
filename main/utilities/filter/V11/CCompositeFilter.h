/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/




#ifndef DAQ_V11_CCOMPOSITEFILTER_H
#define DAQ_V11_CCOMPOSITEFILTER_H

#include <V11/CFilter.h>

#include <vector> 
#include <memory>

namespace DAQ {
namespace V11 {

// Forward declaration
class CCompositeFilter;

// Useful typedefs
using CCompositeFilterUPtr = std::unique_ptr<CCompositeFilter>;
using CCompositeFilterPtr  = std::shared_ptr<CCompositeFilter>;

/*!
 * \brief The CCompositeFilter class
 *
 * The CCompositeFilter class acts as a composite entity. Multiple
 * filter objects can be registered or added to an object of this
 * class, so that when a method of the object is called, it will
 * call the same method for all of the objecs that have been
 * registered to it.
 */
class CCompositeFilter : public CFilter
{
  public:
    // Basic typedefs for use with the class
    typedef std::vector<CFilterPtr> FilterContainer;
    typedef FilterContainer::iterator iterator;
    typedef FilterContainer::const_iterator const_iterator;

  public:
    FilterContainer m_filter; //!< The list of filters

  public:
    // Canonical methods
    CCompositeFilter();
    CCompositeFilter(const CCompositeFilter&);
    CCompositeFilter& operator=(const CCompositeFilter&);

    // Filter registration
    void registerFilter(CFilterPtr filter);
    FilterContainer& getFilters();

    // Virtual copy constructor 
    CCompositeFilter* clone() const;

    virtual ~CCompositeFilter();

    // Handlers
    virtual CRingItem* handleRingItem(CRingItem* item);
    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem* item);
    virtual CRingItem* handleScalerItem(CRingScalerItem* item);
    virtual CRingItem* handleTextItem(CRingTextItem* item);
    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* item);
    virtual CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem* item);
    virtual CRingItem* handleFragmentItem(CRingFragmentItem* item);
    virtual CRingItem* handleAbnormalEndItem(CAbnormalEndItem *pItem);
    virtual CRingItem* handleDataFormatItem(CDataFormatItem *pItem);
    virtual CRingItem* handleGlomParameters(CGlomParameters *pItem);

    // Startup
    virtual void initialize();
    // Exit 
    virtual void finalize();

    // iterator interface
    iterator begin() { return m_filter.begin(); }
    const_iterator begin() const { return m_filter.begin(); }

    iterator end() { return m_filter.end(); }
    const_iterator end() const { return m_filter.end(); }

    void clear() { m_filter.clear() ;}
    size_t size() const { return m_filter.size();}


};



} // end V11
} // end DAQ

#endif
