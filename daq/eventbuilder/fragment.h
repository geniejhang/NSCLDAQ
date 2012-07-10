/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __FRAGMENT_H
#define __FRAGMENT_H

#ifndef __CRTL_STDINT_H
#include <stdint.h>
#ifndef __CRTL_STDINT_H
#define __CRTL_STDINT_H
#endif
#endif

#ifndef __CRTL_TYPE_H
#include <sys/types.h>
#ifndef __CRTL_TYPE_H
#define __CRTL_TYPE_H
#endif
#endif

/**
 * The conditional directives in this file are for two reasons:
 * - In C++ sources, the headers will qualify the type and function
 *   definitions with the ::EVB:: namepsace.
 * - The implementations of the support functions are in C
 *
 * All of this is to support C programmers as well as C++.
 */

#ifdef __cplusplus
namespace EVB {
#endif
  /**
   *  The typedef below defines a fragment header.
   */ 
  
  typedef struct _FragmentHeader {
    uint64_t       s_timestamp;	//< Fragment time relative to globally synchronized clock.
    uint32_t       s_sourceId ;	//< Unique source identifier.
    uint32_t       s_size;	// Bytes in fragment payload.
  } FragmentHeader, *pFragmentHeader;
  
  /**
   * Within the event builder fragments and payloads get bundled
   * together into something that looks like:
   */
  typedef struct _Fragment {
    FragmentHeader   s_header;
    void*           s_pBody;
  } Fragment, *pFragment;


  /**
   * Linked list of fragments:
   */
  typedef struct _FragmentChain {
    struct _FragmentChain*    s_pNext;
    pFragment         s_pFragment;
  } FragmentChain, *pFragmentChain;

  /**
   * Below are convenience functions for fragments:
   */

#ifdef __cplusplus
  extern "C" {
#endif
  void freeFragment(pFragment p);
  pFragment allocateFragment(pFragmentHeader pHeader);
  pFragment newFragment(uint64_t timestamp, uint32_t sourceId, uint32_t size);

  size_t fragmentChainLength(pFragmentChain p);
#ifdef __cplusplus
  }
#endif
  
#ifdef __cplusplus
}
#endif
  
   

#endif
