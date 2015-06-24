

#include "ByteBuffer.h"

using namespace DAQ::Buffer;

static ByteBuffer& loadBuffer(ByteBuffer& buffer, const char* beg, const char* end)
{
  // make sure we only allocate once the memory we need
  buffer.reserve(buffer.size() + (end - beg));

  // copy the data using a back_inserter (calls push_back when assigning),
  // so that the buffer is properly sized when the copy is completed.
  std::copy(beg, end, std::back_inserter(buffer));

  return buffer;
}


ByteBuffer& operator<<(ByteBuffer& buffer, std::uint8_t value)
{
  buffer.push_back(value);
  return buffer;
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::int8_t value)
{
  buffer.push_back(value);
  return buffer;
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::uint16_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::int16_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::uint32_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::int32_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::uint64_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

ByteBuffer& operator<<(ByteBuffer& buffer, std::int64_t value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  return loadBuffer(buffer, beg, end);
}

