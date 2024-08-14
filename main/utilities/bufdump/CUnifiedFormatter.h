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
#ifndef UNIFIEDFORMATTER_H
#define UNIFIEDFORMATTER_H
/**
 * @file CUnifiedFormater.h
 * @brief Provides a mechanism to format ring items->strings using unifiedfmt
 * 
 * @note In order to prevent conflicts with the CRingItems in NSCLDAQ, we'll need to 
 * isolate this in a library and build that library against the unified format--and hope
 * the symbols are then weak enough to avoid collisions with CRingItem in NSCLDAQ and
 * CRingItem in UnifiedFormat.
 * 
 *  
*/

#include <string>



class RingItemFactoryBase;

/**
 * This class hides the use of the unified formatter in a separate library.
 * The idea is that the bufdumpmain can instantiate it and pass it ring item
 * data which will then be formatted via the appropriate underlying factory.
 * 
 */
class CUnifiedFormatter {              // Final
private:
    RingItemFactoryBase* m_pBase;
public:
    CUnifiedFormatter(int version);
    ~CUnifiedFormatter();

    std::string operator()(const void* pItem);    // Avoid use of CRingItem to void conflicts.

};

#endif