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


#ifndef BUFFERTRANSLATOR_H
#define BUFFERTRANSLATOR_H

#include <stdint.h>
#include <iostream>
#include <buffer.h>
#include <histotypes.h>

/*-----------------------------------------------------------------------------
  Definition of class CBufferTranslator
-----------------------------------------------------------------------------*/

class CBufferTranslator
{
protected: 
  Address_t m_pBuffer;  /*! A pointer to the buffer this holds */
  
public:
  
  UChar_t GetByte( UInt_t );
  Short_t GetWord( UInt_t );
  Long_t GetLongword( UInt_t );
  Address_t getBuffer() {
    return m_pBuffer;
  }
  void newBuffer(Address_t pBuffer) {
    m_pBuffer = pBuffer;
  }

  virtual void GetBlock( const Address_t, Int_t, Int_t ) = 0;
  virtual Long_t TranslateLong(ULong_t value) {
    Address_t buffer = m_pBuffer;

    m_pBuffer = &value;
    Long_t     answer = GetLongword(0);

    m_pBuffer = buffer;

    return answer;
  }
  virtual uint64_t getQuad(uint64_t value) = 0;
};

/*-----------------------------------------------------------------------------
  Definition of class SwappingCBufferTranslator
-----------------------------------------------------------------------------*/

class CSwappingBufferTranslator: public CBufferTranslator
{
  
 public:

  // Default Constructor
  CSwappingBufferTranslator( Address_t pB = 0 ) {m_pBuffer = pB;}

  // Accessor functions
  virtual void GetBlock( const Address_t, int, int );
  virtual uint64_t getQuad(uint64_t value);
};

/*-----------------------------------------------------------------------------
  Definition of class NonSwappingCBufferTranslator
-----------------------------------------------------------------------------*/

class CNonSwappingBufferTranslator: public CBufferTranslator
{

 public:

  // Default constrcutor
  CNonSwappingBufferTranslator( Address_t pB = 0 ) {m_pBuffer = pB;}

  // Accessor functions
  virtual void GetBlock( const Address_t, int, int );
  virtual uint64_t getQuad(uint64_t value);
};

/*-----------------------------------------------------------------------------
  Definition of factory class BufferFactory
-----------------------------------------------------------------------------*/

class CBufferFactory
{
 public:
  enum Endian {little, big};
  static CBufferTranslator* 
    CreateBuffer(Address_t pBuffer, Int_t Signature32);
};

//  MyEndianess returns the endianess of the running system
CBufferFactory::Endian MyEndianess();

#endif
