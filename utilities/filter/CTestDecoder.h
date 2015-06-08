

#ifndef CTESTDECODER_H
#define CTESTDECODER_H

#include <string>
#include <CBufferDecoder.h>

class CTestDecoder : public CBufferDecoder 
{
  public:
    virtual const Address_t getBody() { return (void*)(0x12345678); }
    virtual UInt_t getBodySize() { return 100; }
    virtual UInt_t getRun() { return 1; }
    virtual UInt_t getEntityCount() { return 1; }
    virtual UInt_t getSequenceNo() { return 1; }
    virtual UInt_t getLamCount() { return 1; }
    virtual UInt_t getPatternCount() { return 0; }
    virtual UInt_t getBufferType() { return 2; }
    virtual void getByteOrder(Short_t& Signature16, Int_t& Signature32) {
      Signature16=0x1234;
      Signature32=0x12345678;
    }
    virtual std::string getTitle() { return std::string("test"); }
};
#endif
