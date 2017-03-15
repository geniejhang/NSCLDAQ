/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CEndRunInfoFactory.cpp
# @brief  Implements the end run info factory.
# @author <fox@nscl.msu.edu>
*/

#include "CEndRunInfoFactory.h"
#include "CEndRunInfo12.h"
#include "CEndRunInfo11.h"
#include "CEndRunInfo10.h"
#include <V10/DataFormat.h>
#include <V11/DataFormat.h>

#include <V12/DataFormat.h>
#include <V12/CRingItemParser.h>

#include <array>
#include <stdexcept>
#include <exception>
#include <io.h>
#include <iostream>

using namespace DAQ;




/**
 * create
 *    This overload creates a new end run info class from an open file:
 *    -   Seek the file to the beginning.
 *    -   Read the first ring item.
 *    -   If the first ring item is a ring format item, return an 11 end run object.
 *    -   If the first ring item is not a ring format item, return a 10.x end run object.
 *    -   If the first ring item is a ring format item but not for 11, throw std::domain_error.
 *
 *   @param fd - file descriptor open on the file.
 *   @throw std::domain_error - if it's clear we can't build the correct object.
 *   @note - as a side effect, the file is rewound.
 */

CEndRunInfo*
CEndRunInfoFactory::create(int fd)
{
    lseek(fd, 0, SEEK_SET);               // Rewind the file.
    
    // Read the header first to see if it's a format item:
    // all ring item version have a size and type to start off
    // with
    std::array<char,8> hdrBuffer;
    int status = io::readData(fd, hdrBuffer.data(), hdrBuffer.size());

    uint32_t size, type;
    bool needsSwap;
    V12::Parser::parseSizeAndType(hdrBuffer.begin(), hdrBuffer.end(),
                                  size, type, needsSwap);

    lseek(fd, 0, SEEK_SET);   // rewind again.

    // we have an empty file... give it to a V10 end run info and let it do its thing.

    if (status == 0 || size < hdrBuffer.size()) {
        return create(nscldaq10, fd);
    }

    if (type == V11::RING_FORMAT || type == V12::RING_FORMAT) {
        if (size == 4*sizeof(uint32_t)) {
            // we have a version 11 data format item
            return create(nscldaq11, fd);
        } else if (size == 6*sizeof(uint32_t)) {
            // we have a version 12 data format item
            return create(nscldaq12, fd);
        } else {
            throw std::domain_error("Looks like this file format is newer than I can handle");
        }
    } else {
        if (type == V10::BEGIN_RUN || type == V10::END_RUN ||
                type == V10::PAUSE_RUN || type == V10::RESUME_RUN) {
            return create(nscldaq10, fd);
        } else {
            throw std::domain_error("Looks like this file format is newer than I can handle");
        }
    }
}
/**
 * create
 *     This overload creates an end run info object of the specified type.
 *     Note that the item is dynamically pooofed into being so the caller
 *     is responsible for delete-ing it when done.
 *
 *    @param version - version of the end run info to create.
 *    @param fd   - File descriptor on file on which the ring item will be made.
 *    @return CEndRunInfo* pointer to dynamically created end run info object
 *                         of the right type
 *    @throw std::domain_error if type is not valid.
 */
CEndRunInfo*
CEndRunInfoFactory::create(CEndRunInfoFactory::DAQVersion version, int fd)
{
    switch (version) {
        case nscldaq11:
            return new CEndRunInfo11(fd);
        case nscldaq10:
            return new CEndRunInfo10(fd);
    case nscldaq12:
        return new CEndRunInfo12(fd);
    default:
            throw std::domain_error("Invalid daq version to create");
    }
}
