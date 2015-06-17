
#include <CTestSourceSink.h>

#include <iterator>
#include <algorithm>
#include <stdexcept>

using namespace std;

CTestSourceSink::CTestSourceSink()
  : m_buffer()
{}

CTestSourceSink::CTestSourceSink(size_t buffer_size)
  : m_buffer()
{
  m_buffer.reserve(buffer_size);
}

CTestSourceSink::~CTestSourceSink() {}

void CTestSourceSink::putItem(const CRingItem& item)
{}

void
CTestSourceSink::put(const void* pData, size_t nBytes)
{
  auto pBegin = reinterpret_cast<const char*>(pData);
  auto pEnd   = pBegin + nBytes;

  copy(pBegin, pEnd, back_inserter(m_buffer));
}

void CTestSourceSink::read(char* pData, size_t nBytes)
{
  if (m_buffer.size() < nBytes) {
    throw std::runtime_error("TestSourceSink::get() does not have requested bytes stored");
  }

  auto itBegin = m_buffer.begin();
  auto itEnd   = itBegin + nBytes;

  auto itOut = reinterpret_cast<uint8_t*>(pData);

  // copy the bytes requested
  copy(itBegin, itEnd, itOut);

  // erase the bytes sent out
  m_buffer.erase(itBegin, itEnd);
}
