
#include <bheader.h>
#include <cstdint>

using namespace DAQ::Buffer;
using namespace DAQ::V8;

ByteBuffer& operator<<(ByteBuffer& buffer, const bftime& time)
{
  buffer << time.month;
  buffer << time.day;
  buffer << time.year;
  buffer << time.hours;
  buffer << time.min;
  buffer << time.sec;
  buffer << time.tenths;
  return buffer;
}

ByteBuffer& operator<<(ByteBuffer& buffer, const bheader& header)
{
  buffer << header.nwds;
  // the next is an enum and must be forced to 16-bit wide
  buffer << std::uint16_t(header.type);
  buffer << header.cks;
  buffer << header.run;
  buffer << header.seq;
  buffer << header.nevt;
  buffer << header.nlam;
  buffer << header.cpu;
  buffer << header.nbit;
  // the next is an enum and must be forced to 16-bit wide
  buffer << std::uint16_t(header.buffmt);
  buffer << header.ssignature;
  buffer << header.lsignature;
  buffer << header.unused[0];
  buffer << header.unused[1];

  return buffer;
}
