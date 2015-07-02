
#ifndef CBUFFER_H
#define CBUFFER_H

#include <vector>
#include <array>
#include <cstring>
#include <string>
#include <iterator>

namespace DAQ
{
  namespace Buffer
  {

    using ByteBuffer = std::vector<unsigned char>;

  }  // end of Buffer
} // end of DAQ


/*!
 *  \brief Data insertion using a vector
 */
template<class T>
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const std::vector<T>& data) {

  std::size_t nBytes = data.size() * sizeof(T);

  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(data.data());
  const char* end = beg + nBytes;

  // make sure we only allocate once the memory we need
  buffer.reserve(buffer.size() + nBytes);

  // copy the data using a back_inserter (calls push_back when assigning),
  // so that the buffer is properly sized when the copy is completed.
  std::copy(beg, end, std::back_inserter(buffer));

  return buffer;
}

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::uint8_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::int8_t value);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::uint16_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::int16_t value);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::uint32_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::int32_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::uint64_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    std::int64_t value);
extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::Buffer::ByteBuffer& rhs);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const std::string& string);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const char* string);
#endif
