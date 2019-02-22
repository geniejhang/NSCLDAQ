/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CRingItemProcessor.h
 *  @brief: Process ring items into output lists of ring items.
 */
#ifndef CRINGITEMPROCESSOR_H
#define CRINGITEMPROCESSOR_H
#include "MessageTypes.h"
#include <list>

class CRingItem;

/**
 * @interface CRingItemProcessor
 *    Accepts a single ring item and produces a list of output ring items.
 *    The ring items are produced in message form.  The message type indicates
 *    what to do but the message bodies are each pointers to a ring item.
 */
class CRingItemProcessor
{
public:
    CRingItemProcessor();
    virtual ~CRingItemProcessor();
    virtual MessageType::Message operator()(MessageType::Message& msg);
    virtual std::list<CRingItem*> process(CRingItem& item) = 0;
    
};

#endif