// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CEventAccumulator.h"            // White box testing.
#undef private

#include <fragment.h>
#include <DataFormat.h>
#include <CRingScalerItem.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <time.h>

static std::string FilenameTemplate="evactestXXXXXX";
static uint32_t headerSize(sizeof(RingItemHeader) + sizeof(BodyHeader));

class evaccTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(evaccTest);
  CPPUNIT_TEST(construct);
  
  CPPUNIT_TEST(allocinfo_1);
  CPPUNIT_TEST(allocinfo_2);
  CPPUNIT_TEST(freeinfo_1);
  
  CPPUNIT_TEST(sizeiov_1);
  
  CPPUNIT_TEST(freespace_1);
  CPPUNIT_TEST(freespace_2);
  
  CPPUNIT_TEST(itemtype_1);
  
  CPPUNIT_TEST(reservesize_1);
  
  CPPUNIT_TEST(appendf_1);
  CPPUNIT_TEST(appendf_2);
  
  CPPUNIT_TEST(addfrag_1);
  CPPUNIT_TEST(finish_1);    // Need this to test other branches of addFragment
  CPPUNIT_TEST(addfrag_2);   // fragment with same type appends.
  CPPUNIT_TEST(addfrag_3);   // Fragment with different type finishes prior.
  CPPUNIT_TEST(addfrag_4);   // events need flushing to make space.
  CPPUNIT_TEST(addfrag_5);   // Adding a fragment to an event would overflow.
  CPPUNIT_TEST(addfrag_6);   // Fragment bigger than buffer by itself.
  CPPUNIT_TEST(addfrag_7);   // Fragment overflows maxfrags/event.
  
  CPPUNIT_TEST(flush_1);    // Event flushing. tests.
  CPPUNIT_TEST(flush_2);
  CPPUNIT_TEST(flush_3);
  CPPUNIT_TEST(flush_4);
  CPPUNIT_TEST(flush_5);
  
  CPPUNIT_TEST(oob_1);       // out of band fragment tests.
  CPPUNIT_TEST(oob_2);
  CPPUNIT_TEST(oob_3);
  CPPUNIT_TEST_SUITE_END();
  


private:
  std::string        m_filename;
  int                m_fd;
  CEventAccumulator* m_pTestObj;
public:
  void setUp() {
    char fnameTemplate[FilenameTemplate.size() +1];
    strcpy(fnameTemplate, FilenameTemplate.c_str());
    m_fd = mkstemp(fnameTemplate);
    m_filename = fnameTemplate;
    
    m_pTestObj =
      new CEventAccumulator(m_fd, 1, 1024, 10, CEventAccumulator::last);
    
  }
  void tearDown() {
    delete m_pTestObj;    // This first as we may flush.
    close(m_fd);
    unlink(m_filename.c_str());
  }
protected:
  void construct();
  
  void allocinfo_1();
  void allocinfo_2();
  void freeinfo_1();
  
  void sizeiov_1();
  
  void freespace_1();
  void freespace_2();
  
  void itemtype_1();
  
  void reservesize_1();
  
  void appendf_1();
  void appendf_2();
  
  void addfrag_1();
  void finish_1();
  void addfrag_2();
  void addfrag_3();
  void addfrag_4();
  void addfrag_5();
  void addfrag_6();            // Fragment bigger than buffer.
  void addfrag_7();
  
  void flush_1();
  void flush_2();
  void flush_3();
  void flush_4();
  void flush_5();
  
  void oob_1();
  void oob_2();
  void oob_3();
};  

CPPUNIT_TEST_SUITE_REGISTRATION(evaccTest);

void evaccTest::construct() {
  EQ(m_fd, m_pTestObj->m_nFd);
  EQ(time_t(1), m_pTestObj->m_maxFlushTime);
  
  time_t now = time_t(nullptr);
  ASSERT((now - m_pTestObj->m_lastFlushTime) <= 1);
  EQ(CEventAccumulator::last, m_pTestObj->m_tsPolicy);
  ASSERT(m_pTestObj->m_pBuffer);
  EQ(size_t(1024), m_pTestObj->m_nBufferSize);
  EQ(size_t(0), m_pTestObj->m_nBytesInBuffer);
  
  EQ(size_t(0), m_pTestObj->m_fragsInBuffer.size());
  EQ(m_pTestObj->m_nMaxFrags, m_pTestObj->m_freeFrags.size());
  ASSERT(!m_pTestObj->m_pCurrentEvent);
  
  ASSERT(!m_pTestObj->m_pIoVectors);
  EQ(size_t(0), m_pTestObj->m_nMaxIoVecs);
  EQ(size_t(0), m_pTestObj->m_nIoVecs);
}


void evaccTest::allocinfo_1()
{
  // Make us a fragment:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  CEventAccumulator::pEventInformation pInfo;
  CPPUNIT_ASSERT_NO_THROW(
    pInfo =
      m_pTestObj->allocEventInfo(reinterpret_cast<EVB::pFlatFragment>(pHdr), 1)
  );
  
  // Now check the contents:
  
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pBodyStart);
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pInsertionPoint);
  
  CEventAccumulator::EventAccumulation& Ac(pInfo->s_eventInfo);
  EQ(size_t(0), Ac.s_nBytes);
  EQ(size_t(0), Ac.s_nFragments);
  EQ(uint64_t(0), Ac.s_TimestampTotal);
  
  RingItemHeader& Ih(pInfo->s_eventHeader.s_itemHeader);
  BodyHeader&     Bh(pInfo->s_eventHeader.s_bodyHeader);
  
  EQ(uint32_t(headerSize), Ih.s_size);
  EQ(PHYSICS_EVENT, Ih.s_type);
  
  EQ(NULL_TIMESTAMP, Bh.s_timestamp);
  EQ(uint32_t(1), Bh.s_sourceId);
  EQ(uint32_t(0), Bh.s_barrier);
  EQ(uint32_t(sizeof(BodyHeader)), Bh.s_size);
}

void evaccTest::allocinfo_2() // If ts policy is first, init with ts.
{
  m_pTestObj->m_tsPolicy  = CEventAccumulator::first;
  
    // Make us a fragment:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  CEventAccumulator::pEventInformation pInfo;
  CPPUNIT_ASSERT_NO_THROW(
    pInfo =
      m_pTestObj->allocEventInfo(reinterpret_cast<EVB::pFlatFragment>(pHdr), 1)
  );
  
  // For now assume all the rest is ok:
  BodyHeader&     Bh(pInfo->s_eventHeader.s_bodyHeader);
  EQ(pHdr->s_timestamp, Bh.s_timestamp);
}

void evaccTest::freeinfo_1()
{
  size_t nFree = m_pTestObj->m_freeFrags.size();
  nFree++;                              // expected value.
  
  CEventAccumulator::pEventInformation pI = new CEventAccumulator::EventInformation;
  m_pTestObj->freeEventInfo(pI);
  EQ(nFree, m_pTestObj->m_freeFrags.size());
}
void evaccTest::sizeiov_1()
{
  CPPUNIT_ASSERT_NO_THROW(
    m_pTestObj->sizeIoVecs(100)
  );
  ASSERT(m_pTestObj->m_pIoVectors);
  
  EQ(size_t(100), m_pTestObj->m_nMaxIoVecs);
}
void evaccTest::freespace_1()  // initially the whole buffer is free:
{
  EQ(m_pTestObj->m_nBufferSize, m_pTestObj->freeSpace());
}
void evaccTest::freespace_2()
{
  // Indicate some is used:
  
  m_pTestObj->m_nBytesInBuffer = 100;
  EQ(m_pTestObj->m_nBufferSize - 100, m_pTestObj->freeSpace());
}
void evaccTest::itemtype_1()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
    
  EQ(PHYSICS_EVENT, m_pTestObj->itemType(reinterpret_cast<EVB::pFlatFragment>(pHdr)));
}

void evaccTest::reservesize_1()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  
  EQ(sizeof(uint32_t), m_pTestObj->m_nBytesInBuffer);
  
  // Now the info block:
  
  uint8_t* pBeg = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(sizeof(uint32_t), size_t(pNext - pBeg));
  EQ(sizeof(uint32_t), pInfo->s_eventInfo.s_nBytes);
  EQ(size_t(0), pInfo->s_eventInfo.s_nFragments);
  
  // The actual size field has sizeof(uint32_t) as well:
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(uint32_t)), *pSize);
}
void evaccTest::appendf_1()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  m_pTestObj->appendFragment(pFrag);
  
  // let's see how pInfo was updated.  The data in the buffer is the
  // ring item body and body header.... preceded by an updated event size
  
  // Event size in buffer; and ring item copied to buffer.
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)),
    *pSize
  );
  EQ(0, memcmp(pFrag, pSize+1, *pSize - sizeof(uint32_t)));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t) + headerSize),
    sz
  );
  
  // Body header timestamp should match the fragment's since policy defaults
  // was set to be last:
  
  EQ(pBh->s_timestamp, pInfo->s_eventHeader.s_bodyHeader.s_timestamp);
}
void evaccTest::appendf_2()
{
  m_pTestObj->m_tsPolicy=CEventAccumulator::average;
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  m_pTestObj->appendFragment(pFrag);

  // The info's timestamp sum should be reflected:
  
  EQ(uint64_t(0x12345678), pInfo->s_eventInfo.s_TimestampTotal);
  
  // If we throw the item at it again:
  
  m_pTestObj->appendFragment(pFrag);
  EQ(2*uint64_t(0x12345678), pInfo->s_eventInfo.s_TimestampTotal);
}
void evaccTest::addfrag_1()
{
  // New fragment -- check
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  // There'd better be a current event:
  
  ASSERT(m_pTestObj->m_pCurrentEvent);
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  // The current event content should just be like in appendf_1():
  
   // let's see how pInfo was updated.  The data in the buffer is the
  // ring item body and body header.... preceded by an updated event size
  
  // Event size in buffer; and ring item copied to buffer.
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(EVB::FragmentHeader)+sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pFrag, pSize+1, sizeof(EVB::FragmentHeader) + pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)+headerSize),
    sz
  );
}
// we need to see an event finished properly before we can
// check the other branches of addFragment so:

void evaccTest::finish_1()
{
  // New fragment -- check
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  m_pTestObj->finishEvent();
  
  // There should be 1 frags in buffer - and it should match addfrag_1
  
  EQ(size_t(1), m_pTestObj->m_fragsInBuffer.size());
  auto pInfo = m_pTestObj->m_fragsInBuffer.front();
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(EVB::FragmentHeader)+sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pFrag, pSize+1, sizeof(EVB::FragmentHeader) + pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)+headerSize),
    sz
  );
}
void evaccTest::addfrag_2()    // append fragment current:
{
  // New fragment -- check
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  pHdr->s_timestamp+= 0x100;     // 0x12345778
  m_pTestObj->addFragment(pFrag, 2);
  
  EQ(size_t(0), m_pTestObj->m_fragsInBuffer.size());
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  // The event has two frags equally sized, identical other than for the
  // timestamps.
  
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  uint32_t size =
    2*(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader)) + sizeof(uint32_t);
  EQ(size , *pSize);
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(size_t(size), a.s_nBytes);
  EQ(size_t(2), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(size_t(size), size_t(pNext - pBase));
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(size + headerSize, sz);

  // There will be two fragments in the buffer:
  
  // Fragment 1:
  
  pHdr->s_timestamp = 0x12345678;           // original value
  uint8_t* pFrag1 = static_cast<uint8_t*>(m_pTestObj->m_pBuffer) + sizeof(uint32_t);
  ASSERT(memcmp(pFrag, pFrag1, sizeof(EVB::FragmentHeader) + pItem->s_size) == 0);
  
  // Fragment 2:
  
  pHdr->s_timestamp += 0x100;
  auto pFrag2      = pFrag1 +  pItem->s_size + sizeof(EVB::FragmentHeader);
  ASSERT(memcmp(pFrag, pFrag2, sizeof(EVB::FragmentHeader) + pItem->s_size)== 0);
}
void evaccTest::addfrag_3()   // Adding a fragment of a different type ends event
{
  // New fragment -- check
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  pItem->s_type = END_RUN;
  m_pTestObj->addFragment(pFrag, 2);
  
  // There should be a fragment in the buffer and a current fragment
  // that is our stub of an end run item:
  
  EQ(size_t(1), m_pTestObj->m_fragsInBuffer.size());
  ASSERT(m_pTestObj->m_pCurrentEvent);
  auto pInfo = m_pTestObj->m_pCurrentEvent;
 
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  uint32_t size =
    (sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader)) + sizeof(uint32_t);
  EQ(size , *pSize);
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(size_t(size), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  uint32_t* pFrag1 = pSize+1;
  ASSERT(memcmp(pFrag, pFrag1, sizeof(EVB::FragmentHeader) + pItem->s_size) ==0);
}
void evaccTest::addfrag_4()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  m_pTestObj->finishEvent();
  
  // Fake like threre's no space for the next fragment:
  
  m_pTestObj->m_nBufferSize = m_pTestObj->m_nBytesInBuffer + 10;
  
  m_pTestObj->addFragment(pFrag, 2);
  
  // This fragment should be at the start of the buffer;
  // as slide should have happened.
  
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pBodyStart);
  
  // Check body end:
  
  size_t size =
    static_cast<uint8_t*>(pInfo->s_pInsertionPoint)  -
    static_cast<uint8_t*>(pInfo->s_pBodyStart);
  EQ(sizeof(EVB::FragmentHeader) + pItem->s_size, size - sizeof(uint32_t));
  
  // The data that needed sliding was the size uint32_t:
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(sizeof(EVB::FragmentHeader) + pItem->s_size, *pSize - sizeof(uint32_t));
  
  
}
// If we attempt to add a fragment to the current event that would cause it to
// overflow, we terminate the event, and start a new one with the new fragment.


void evaccTest::addfrag_5()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  // Make it so the next addfrag will overflow:
  
  m_pTestObj->m_nBufferSize = m_pTestObj->m_nBytesInBuffer + 10;
  
  m_pTestObj->addFragment(pFrag, 2);
  
  // this should be the only fragment in the event, as this caused a finish
  // followed by a flush:
  
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  EQ(size_t(1), pInfo->s_eventInfo.s_nFragments);  // no over flow ths is 2.
  
  
}
void evaccTest::addfrag_6()
{
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->m_nBufferSize =10;
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->addFragment(pFrag, 2),
    std::range_error
  );
}
void evaccTest::addfrag_7()
{
  m_pTestObj->m_nMaxFrags = 2;
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);
  m_pTestObj->addFragment(pFrag, 2);   // forced an end of event
  
  ASSERT(!m_pTestObj->m_pCurrentEvent);  //finished implicitly.
  
  // there's one event in the buffer and it has our fragments:
  
}
void evaccTest::flush_1()
{
  // immediate flush results in no data in file:
  
  m_pTestObj->flushEvents();
  
  struct stat info;
  stat(m_filename.c_str(), &info);
  EQ(off_t(0), info.st_size);
}
void evaccTest::flush_2()
{
  // flush does not finish a partial event:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);
  
  m_pTestObj->flushEvents();

  struct stat info;
  stat(m_filename.c_str(), &info);
  EQ(off_t(0), info.st_size);  
}
void evaccTest::flush_3()
{
  // Flush an event with one fragment:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);
  m_pTestObj->finishEvent();
  m_pTestObj->flushEvents();
  
  struct stat info;
  stat(m_filename.c_str(), &info);
  ASSERT(info.st_size > 0);
  
  // Contents should be the size uint32_t, and the flattened fragment.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t readBuffer[1024];
  

  ssize_t nRead = read(fd, readBuffer, sizeof(readBuffer));
  EQ(ssize_t(
    sizeof(RingItemHeader) + sizeof(BodyHeader)  +  // Full ring item header
    sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size    // size of the 1 fragment.
  ), nRead);
  
  // First shoulid be  ring item header for the entire event:
  
  pRingItemHeader pRHdr = reinterpret_cast<pRingItemHeader>(readBuffer);
  EQ(PHYSICS_EVENT, pRHdr->s_type);
  EQ(
    uint32_t(sizeof(RingItemHeader) + sizeof(BodyHeader)  +  // Full ring item header
    sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size ), pRHdr->s_size);
  // Next the uint32_t that is the size of the remaining event:
  
  pRHdr++;                 // Points to BodyHeader....
  pBodyHeader pReadBhdr = reinterpret_cast<pBodyHeader>(pRHdr);
  pReadBhdr++;
  
  uint32_t* payloadSize = reinterpret_cast<uint32_t*>(pReadBhdr);
  EQ(
    uint32_t(sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size ),
    *payloadSize
  );
  // After that is the first fragment:
  
  payloadSize++;
  EVB::pFlatFragment pReadFrag =
    reinterpret_cast<EVB::pFlatFragment>(payloadSize);
  ASSERT(memcmp(pFrag, pReadFrag, sizeof(EVB::FragmentHeader) + pHdr->s_size) == 0);
}
void evaccTest::flush_4()
{
    // One event, a couple of fragments.

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);
  pHdr->s_sourceId = 1;           // Different source id.
  pHdr->s_timestamp = 0x12345679; // Slightly different timestamp.
  m_pTestObj->addFragment(pFrag, 2);
  
  m_pTestObj->finishEvent();
  m_pTestObj->flushEvents();
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  
  // Suck in the entire file into a single buffer:
  
  uint8_t readBuffer[2048];
  ssize_t nRead = read(fd, readBuffer, sizeof(readBuffer));
  
  // Should be one item with two identically sized fragments:
  
  ASSERT(nRead < sizeof(readBuffer));
  
  uint8_t* p = readBuffer;
  pRingItemHeader pRH = reinterpret_cast<pRingItemHeader>(p);
  EQ(PHYSICS_EVENT, pRH->s_type);
  EQ(uint32_t(
    headerSize + sizeof(uint32_t) +
    2*(sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader))
  ), pRH->s_size);
  // Following the ring item header is a body header with the latest
  // timestamp:
  
  pBodyHeader pRBH = reinterpret_cast<pBodyHeader>(pRH+1);
  EQ(uint64_t(0x12345679), pRBH->s_timestamp);
  EQ(uint32_t(2), pRBH->s_sourceId);
  
  // Next is the uint32_t size of the fragment body:
  
  uint32_t* pPayloadSize = reinterpret_cast<uint32_t*>(pRBH+1);
  EQ(uint32_t(
    sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader))
  ), *pPayloadSize);
  // Now the first ring item:
  
  EVB::pFragmentHeader pRFH =
    reinterpret_cast<EVB::pFragmentHeader>(pPayloadSize+1);
  EQ(uint64_t(0x12345678), pRFH->s_timestamp);
  EQ(uint32_t(5), pRFH->s_sourceId);
  EQ(uint32_t(sizeof(RingItemHeader) + sizeof(BodyHeader)), pRFH->s_size);
  
  // Now compare the ring item
  
  pHdr->s_timestamp = 0x12345678; // (put things back the way they were.)
  pHdr->s_sourceId  = 5;
  ASSERT(memcmp(pRFH+1, pItem, pItem->s_size) == 0);
  
  // On to the next fragment:
  
  pRH = reinterpret_cast<pRingItemHeader>(pRFH+1);
  p   = reinterpret_cast<uint8_t*>(pRH);
  p  += pRH->s_size;
  
  // p points to the next fragment header:
  
  pRFH= reinterpret_cast<EVB::pFragmentHeader>(p);
  EQ(uint64_t(0x12345679), pRFH->s_timestamp);
  EQ(uint32_t(1), pRFH->s_sourceId);
  
}

void evaccTest::flush_5()
{
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);   // Event 1.
  m_pTestObj->finishEvent();
  
  pHdr->s_timestamp++;
  pBh->s_timestamp++;
  m_pTestObj->addFragment(pFrag, 2);   // Event 3.
  m_pTestObj->finishEvent();
  
  pHdr->s_timestamp++;
  pBh->s_timestamp++;
  m_pTestObj->addFragment(pFrag, 2);   // Event 3.
  m_pTestObj->finishEvent();          
  
  m_pTestObj->flushEvents();
  
  // Now read it all in in one gulp:
  
  uint8_t rdBuffer[8192];
  int fd = open(m_filename.c_str(), O_RDONLY);
  ssize_t nRead = read(fd, rdBuffer, sizeof(buffer));
  ASSERT(nRead < sizeof(rdBuffer));   // Make sure we got it all.
  
  // Each event consists of a ring header a body header, a uint32_t,
  // a fragment header, a ring  item header, and a body header:
  
  ssize_t expectedSize = 3*(
    sizeof(RingItemHeader)  + sizeof(BodyHeader)
    + sizeof(uint32_t) + sizeof(EVB::FragmentHeader) +
    + sizeof(RingItemHeader) + sizeof(BodyHeader));
  EQ(expectedSize, nRead);
  
  
  uint64_t expectedTs = 0x12345678;
  uint8_t* p = reinterpret_cast<uint8_t*>(rdBuffer);
  
  for (int i =0; i < 3; i++) {   // Loop over events.
    std::stringstream strMsg;
    strMsg << "Event: " << i;
    std::string msg = strMsg.str();
    
    // Ring item header for the event:
    
    pRingItemHeader pRHdr = reinterpret_cast<pRingItemHeader>(p);
    p += pRHdr->s_size;                 // Setup for next event.
    
    EQMSG(msg, PHYSICS_EVENT, pRHdr->s_type);
    EQMSG(msg, uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
      sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader)
    ), pRHdr->s_size);
    
    // Body header for the event as a whole:
    
    pBodyHeader pRBhdr = reinterpret_cast<pBodyHeader>(pRHdr +1);
    EQMSG(msg, expectedTs, pRBhdr->s_timestamp);
    EQMSG(msg, uint32_t(2), pRBhdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pRBhdr->s_barrier);
    
    // Size field:
    
    uint32_t* pFragSize = reinterpret_cast<uint32_t*>(pRBhdr+1);
    EQMSG(msg, uint32_t(
      sizeof(uint32_t) +
      sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader)      
    ) , *pFragSize);
    
    // Fragment header for the event:
    
    EVB::pFragmentHeader pFHdr =
      reinterpret_cast<EVB::pFragmentHeader>(pFragSize+1);
    EQMSG(msg, expectedTs, pFHdr->s_timestamp);
    EQMSG(msg, uint32_t(5), pFHdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pFHdr->s_barrier);
    EQMSG(msg, uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader)  
    ), pFHdr->s_size);
    
    // Ring item header for the event.
 
    pRHdr = reinterpret_cast<pRingItemHeader>(pFHdr+1);
    EQMSG(msg,uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader)
    ), pRHdr->s_size);
    EQMSG(msg, PHYSICS_EVENT, pRHdr->s_type);
    
    // body header for the event.
    
    pRBhdr = reinterpret_cast<pBodyHeader>(pRHdr+1);
    EQMSG(msg, expectedTs, pRBhdr->s_timestamp);
    EQMSG(msg, uint32_t(5), pRBhdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pRBhdr->s_barrier);
    
    // next event has the next timestamp..
    
    expectedTs++;
  }
}
void evaccTest::oob_1()  
{
  // oob fragment when nothing's buffered gives the oob fragment.
  
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  EVB::pFlatFragment pFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pFrag, 1);
  
  // The file should have the fragment's ring item;.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t rdBuffer[2048];
  ssize_t rdSize = read(fd, rdBuffer, sizeof(rdBuffer));
  ASSERT(rdSize < sizeof(rdBuffer));
  EQ(ssize_t(pOriginalItem->s_header.s_size), rdSize);
  ASSERT(memcmp(pOriginalItem, rdBuffer, rdSize) == 0);
}
void evaccTest::oob_2()
{
  // oob item when there's a partial event -- gives only the oob
  // item and the partial event remains untouched.
  
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);   // The in progress fragment.
  
  
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  pFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pFrag, 1);
  
  // we should find the scaler item in the file:  

  // The file should have the fragment's ring item;.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t rdBuffer[2048];
  ssize_t rdSize = read(fd, rdBuffer, sizeof(rdBuffer));
  ASSERT(rdSize < sizeof(rdBuffer));
  EQ(ssize_t(pOriginalItem->s_header.s_size), rdSize);
  ASSERT(memcmp(pOriginalItem, rdBuffer, rdSize) == 0);
  
  // There should be a currente event with one fragment:
  
  ASSERT(m_pTestObj->m_pCurrentEvent);
  EQ(size_t(1), m_pTestObj->m_pCurrentEvent->s_eventInfo.s_nFragments);
}
void evaccTest::oob_3()
{
  // any buffered event gets flushed befgore 
  // the OOB event.
  
  // oob item when there's a partial event -- gives only the oob
  // item and the partial event remains untouched.
  
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  m_pTestObj->addFragment(pFrag, 2);   // The in progress fragment.
  m_pTestObj->finishEvent();           // Fully buffered event now.
  
  // Now put in the oob item -- that should flush both events:
  
  uint8_t oobBuffer[1024];
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(oobBuffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  EVB::pFlatFragment pOOBFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pOOBFrag, 1);
  
  // the file should have both events.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  
  uint8_t rdbuffer[2048];
  ssize_t n = read(fd, rdbuffer, sizeof(rdbuffer));
  
  // First we'll see a PHYSI`CS ring item -- we'll assume it's
  // right if so.
  
  pRingItemHeader pReadItem = reinterpret_cast<pRingItemHeader>(rdbuffer);
  EQ(PHYSICS_EVENT, pReadItem->s_type);
  
  // Next should be a block of data that's identical to the ring scaler item:
  
  uint8_t* p = reinterpret_cast<uint8_t*>(pReadItem);
  p += pReadItem->s_size;
  pReadItem = reinterpret_cast<pRingItemHeader>(p);
  
  EQ(pOriginalItem->s_header.s_size, pReadItem->s_size);
  ASSERT(memcmp(pReadItem, pOriginalItem, pReadItem->s_size) == 0);
}