
#include <bheader.h>
#include <cstdint>
#include <ctime>

using namespace DAQ::Buffer;
using namespace DAQ::V8;

namespace DAQ {
  namespace V8 {

bftime to_bftime(const std::time_t &time)
{
  // start of the epoch
  bftime btime = {1, 1, 1970, 0, 0, 0, 0};

  std::tm* pTime = std::localtime(&time);

  if (pTime != nullptr) {
    btime.month = pTime->tm_mon;
    btime.day = pTime->tm_mday;
    btime.year = pTime->tm_year+1900;
    btime.hours = pTime->tm_hour;
    btime.min = pTime->tm_min;
    btime.sec = pTime->tm_sec;
    btime.tenths = 0;
  }

  return btime;
}


bheader::bheader()
  : bheader(16, VOID, 0, 0, 0, 0, 0, 0, 0, StandardVsn, BOM16, BOM32, 0, 0)
{}

bheader::bheader(std::uint16_t nwds_, std::uint16_t type_, std::uint16_t cks_,
                 std::uint16_t run_, std::uint32_t seq_, std::uint16_t nevt_,
                 std::uint16_t nlam_, std::uint16_t cpu_, std::uint16_t nbit_,
                 std::uint16_t buffmt_, std::uint16_t ssignature_,
                 std::uint32_t lsignature_, std::uint16_t unused0, std::uint16_t unused1)
  : nwds(nwds_),
    type(type_),
    cks(cks_),
    run(run_),
    seq(seq_),
    nevt(nevt_),
    nlam(nlam_),
    cpu(cpu_),
    nbit(nbit_),
    buffmt(buffmt_),
    ssignature(ssignature_),
    lsignature(lsignature_)
{
  unused[0] = unused0;
  unused[1] = unused1;
}

  } // end V8
} // end DAQ


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
