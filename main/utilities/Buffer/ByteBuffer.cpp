#include "ByteBuffer.h"

namespace std {
  
  std::ostream& operator<<(ostream& s, const DAQ::Buffer::ByteBuffer& d) {
    s << "{";
    for (auto e: d) {
      s << e << ' ';
    }
    s << "}";
    return s;
  }
  
}
