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




#ifndef DAQ_V12_CCOMPOSITEFILTER_H
#define DAQ_V12_CCOMPOSITEFILTER_H

#include <V12/CFilter.h>

#include <vector> 
#include <memory>

namespace DAQ {
namespace V12 {

// forward declaration
class CCompositeFilter;

// Useful typedefs
using CCompositeFilterUPtr = std::unique_ptr<CCompositeFilter>;
using CCompositeFilterPtr  = std::shared_ptr<CCompositeFilter>;



/*!
 * \brief The CCompositeFilter class
 *
 * The CCompositeFilter class maintains a set
 * of filters. When a method of this class is called, the same method
 * for each of the filters in the set is called as well. The only exception
 * to this is if there a multiple filters that need their handler to be called
 * and an earlier filter returns nullptr (or equivalent) from its handler. In
 * that case, the processing is aborted and the subsequent filters' handlers
 * will not be called.
 *
 * The order in which the filters is called is the same as the order
 * they were registered.
 *
 */
class CCompositeFilter : public CFilter
{
  public:
    // Basic typedefs for use with the class
    typedef std::vector<CFilterPtr> FilterContainer; // CFilterPtr is a std::shared_ptr
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

    // Virtual copy constructor 
    CFilterUPtr clone() const;

    virtual ~CCompositeFilter();

    // Handlers
    virtual CRingItemPtr handleRingItem(CRingItemPtr item);
    virtual CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr item);
    virtual CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr item);
    virtual CRingTextItemPtr handleTextItem(CRingTextItemPtr item);
    virtual CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr item);
    virtual CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr item);
    virtual CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem);
    virtual CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem);
    virtual CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem);
    virtual CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem);

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



} // end V12
} // end DAQ

#endif
