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


#include <CTemplateFilter.h>
#include <make_unique.h>

#include <cstdint>
#include <iostream>
#include <ByteBuffer.h>

using namespace DAQ::V12;

/*! Virtual copy constructor
 *
 * DO NOT FORGET THIS!
 *
 */
CFilterUPtr CTemplateFilter::clone() const {
    return DAQ::make_unique<CTemplateFilter>(*this);
}


/*! A sample filter for handling physics events

        This filter will be called for every physics event item. It will produce
        a ring item double the size of the original item with the first half being
        the original data and the second half being the data in reversed order.
        This filter is unlikely to have any real use but is defined to be
        illustrative of how to manipulate the data of a ring item.

        @param pItem a pointer to the raw physics event item to process
        @return the resulting ring item from this filter. This can be the same item
                pointed to by pItem or a newly allocated one. Can be any derived
                type of ring item.
    */
DAQ::V12::CPhysicsEventItemPtr
CTemplateFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    using DAQ::Buffer::ByteBuffer;

    // Create a copy of the original item to manipulate. This is unnecessary
    // but allows one to safely abort filtering and return the original ring
    // item.
    CPhysicsEventItemPtr pFiltItem(new CPhysicsEventItem(*pItem));

    ByteBuffer& oldBody = pItem->getBody();
    ByteBuffer& newBody = pFiltItem->getBody();

    newBody.insert(newBody.end(), oldBody.rbegin(), oldBody.rend());

    return pFiltItem;
}

