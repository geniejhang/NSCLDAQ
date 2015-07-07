#ifndef BUFFERIOV8_H
#define BUFFERIOV8_H

#include <iosfwd>

namespace DAQ {
  namespace V8 {
    class CRawBuffer;
  }
}

class CDataSource;
class CDataSink;

extern std::istream& operator>>(std::istream& stream, DAQ::V8::CRawBuffer& buffer);
extern CDataSource& operator>>(CDataSource& stream, DAQ::V8::CRawBuffer& buffer);


extern std::ostream& operator<<(std::ostream& stream, const DAQ::V8::CRawBuffer& buffer);
extern CDataSink& operator<<(CDataSink& stream, const DAQ::V8::CRawBuffer& buffer);



#endif // BUFFERV8IO_H
