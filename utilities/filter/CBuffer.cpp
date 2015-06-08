
#include <algorithm>
#include "CBuffer.h"

//
//
CBuffer::CBuffer(size_t n) : m_begin(m_buffer),
  m_cursor(m_begin),
  m_end(m_begin+8192)
{
  if (n>8192) {
    m_begin = new char[n];
    m_cursor = m_begin;
    m_end = m_begin + n;
  }
}

//
//
CBuffer::CBuffer(const char* begin, const char* end) 
: CBuffer(end - begin) 
{
  std::copy(begin, end, m_begin);
  m_cursor = m_begin + (end-begin);
}

//
//
CBuffer::CBuffer(const CBuffer& rhs) : m_begin(m_buffer),
  m_cursor(m_begin),
  m_end(m_begin+8192)
{
  if (!rhs.usingSBO()) {
    size_t n = rhs.m_end - rhs.m_begin;
    m_begin  = new char[n];
    m_cursor = m_begin;
    m_end = m_begin + n;
  }

  std::copy(rhs.begin(), rhs.end(), m_begin);
  m_cursor = m_begin + (rhs.end() - rhs.begin());
}

//
//
CBuffer::~CBuffer() {
  if (m_begin!=m_buffer) {
    delete [] m_begin;
  }
}

//
//
void CBuffer::reserve(size_t nbytes) {
  if (nbytes > capacity()) {
    // allocate new space. Note that if we have to do this,
    // we cannot be using the small buffer optimization 
    // anymore
    auto newBegin = new char[nbytes];
    
    // copy the old content over ... this invalidate all
    // previous iterators
    std::copy(m_begin, m_cursor, newBegin);
    
    // update the cursor and end
    m_cursor = newBegin + (m_cursor-m_begin);
    m_end = newBegin + nbytes;

    // free old memory
    if (m_begin!=m_buffer) {
      delete [] m_begin;
    }
    // update our start
    m_begin = newBegin;
  }
}

void CBuffer::resizeWithoutInit(size_t nbytes) 
{
  // this really doesn't do anything if capacity is large enought already
  reserve(nbytes);

  // move cursor forward
  m_cursor = m_begin + nbytes;
}

