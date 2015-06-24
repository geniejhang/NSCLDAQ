/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/
/******************************************************************************
  Interface for class CBufferTranslator
******************************************************************************/


#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <algorithm>
#include <iostream>
using namespace std;


namespace DAQ 
{
  namespace BO // byte order package
  {

    template<class T> void swapBytes(T& podObj) {

      char* pItem = reinterpret_cast<char*>(&podObj);
      std::reverse(pItem, pItem+sizeof(podObj));

    }

    // BufferTranslator that works on any generic buffer type
    class CByteSwapper
    {
    private:
      bool         m_needsSwap;

    public:
      CByteSwapper(bool needsSwap);

      bool isSwappingBytes() const { return m_needsSwap; }
      void setSwapBytes(bool swap) { m_needsSwap = swap; }


      // convert raw bytes to a properly byte ordered value
      template<typename T, typename ByteIter> T copyAs(ByteIter pos) const
      {
        T type;
        if (! m_needsSwap) {
            std::copy(pos, pos+sizeof(type), reinterpret_cast<char*>(&type));
          } else {
            std::reverse_copy(pos, pos+sizeof(type), reinterpret_cast<char*>(&type));
          }

        return type;
      }


    };

  } // end of BO
} // end of DAQ
#endif
