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




#ifndef DAQ_V12_CTRANSPARENTFILTER_H
#define DAQ_V12_CTRANSPARENTFILTER_H

#include <V12/CFilter.h>
#include <make_unique.h>


namespace DAQ {
namespace V12 {

// forward declaration
class CTransparentFilter;

// Useful typedefs for smart pointers
using CTransparentFilterUPtr = std::unique_ptr<CTransparentFilter>;
using CTransparentFilterPtr  = std::shared_ptr<CTransparentFilter>;


/**! \class CTransparentFilter
  This class has handlers that do nothing more
  than return the pointer passed as an argument. It uses all of the base class 
  handler implementations and only provides the clone method. For the exact details
  of the handler methods, see CFilter.h
*/
class CTransparentFilter : public CFilter
{
  public:
    // Virtual constructor
    virtual CFilterUPtr clone() const { return DAQ::make_unique<CTransparentFilter>(*this);}
  
};


} // end V12
} // end DAQ

#endif
