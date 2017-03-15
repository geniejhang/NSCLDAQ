/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#ifndef DAQ_CDATASINK_H
#define DAQ_CDATASINK_H

#include <stdlib.h>
#include <vector>
#include <utility>
#include <memory>

namespace DAQ {

class CDataSink;

typedef std::unique_ptr<CDataSink> CDataSinkUPtr;
typedef std::shared_ptr<CDataSink> CDataSinkPtr;


/*! \brief Interface for CDataSinks
*
* This is a pure virtual base class that establishes an
* expected interface for all data sinks.
*/
class CDataSink
{
    
public:
    
    // The virtual destructor
    virtual ~CDataSink();


  /*!
     * \brief Write a block of data to the sink
     *
     * \param pData   - pointer to start of contiguous data to write
     * \param nBytes  - number of bytes to write
     */
    virtual void put(const void* pData, size_t nBytes) = 0;

    virtual void putv(const std::vector<std::pair<const void*, size_t>>& buffers) = 0;
};


} // end DAQ

#endif
