// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include "CEndRunInfoFactory.h"
#include <io.h>

#include <V10/CRingStateChangeItem.h>
#include <V10/DataFormat.h>

#include <V11/DataFormat.h>
#include <V11/CDataFormatItem.h>

#include <V12/CDataFormatItem.h>
#include <V12/CRawRingItem.h>
#include <V12/DataFormat.h>


#include <CFileDataSink.h>
#include <RingIOV10.h>
#include <RingIOV11.h>
#include <RingIOV12.h>

#include "CEndRunInfo10.h"
#include "CEndRunInfo11.h"
#include "CEndRunInfo12.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
#include <stdexcept>
#include <time.h>
#include <string.h>

using namespace DAQ;

class ErInfoFactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ErInfoFactoryTests);
  CPPUNIT_TEST(explicit10);
  CPPUNIT_TEST(explicit11);
  CPPUNIT_TEST(explicit12);
  CPPUNIT_TEST(explicitbad);
  CPPUNIT_TEST(fromfile11);
  CPPUNIT_TEST(fromfile10);
  CPPUNIT_TEST(fromfile12);
  CPPUNIT_TEST(fromfileunrecog);
  CPPUNIT_TEST(fromfileempty);
  CPPUNIT_TEST_SUITE_END();


private:
    int m_fd;
public:
  void setUp() {
      // open a unique file atomically
      char tmplate[] = "testrunXXXXXX";
      m_fd = mkstemp(tmplate);
      unlink(tmplate);
  }
  void tearDown() {
      close(m_fd);
  }
protected:
  void explicit10();
  void explicit11();
  void explicit12();
  void explicitbad();
  
  void fromfile11();
  void fromfile10();
  void fromfile12();
  void fromfileunrecog();
  void fromfileempty();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ErInfoFactoryTests);

// Explicitly create a version 10 endinfo:

void ErInfoFactoryTests::explicit10() {
  //  We'll use the /dev/null file:
  
  CFileDataSink sink(m_fd);
  writeItem(sink, V10::CRingStateChangeItem(V10::BEGIN_RUN));

  // rewind to the beginning of the file for the next read
  lseek(m_fd, 0, SEEK_SET);

  CEndRunInfo* pEr;
  CPPUNIT_ASSERT_NO_THROW(
    pEr = CEndRunInfoFactory::create(CEndRunInfoFactory::nscldaq10, m_fd)
  );
  
  ASSERT(pEr);
  CEndRunInfo10* pE10 = dynamic_cast<CEndRunInfo10*>(pEr);
  ASSERT(pE10);                      // null if not castable.
  
  delete pEr;

                
}
// Explicitly create a version 11 endinfo

void ErInfoFactoryTests::explicit11()
{
  int nullFd = open("/dev/null", O_RDONLY);
  
  CEndRunInfo* pEr;
  CPPUNIT_ASSERT_NO_THROW(
    pEr = CEndRunInfoFactory::create(CEndRunInfoFactory::nscldaq11, nullFd)
  );
  
  ASSERT(pEr);
  CEndRunInfo11* pE11 = dynamic_cast<CEndRunInfo11*>(pEr);
  ASSERT(pE11);
  
  close(m_fd);
  delete pE11;
}

// Explicitly create a version 12 endinfo

void ErInfoFactoryTests::explicit12()
{
  int nullFd = open("/dev/null", O_RDONLY);
  
  std::unique_ptr<CEndRunInfo> pEr;
  CPPUNIT_ASSERT_NO_THROW(
    pEr.reset(CEndRunInfoFactory::create(CEndRunInfoFactory::nscldaq12, nullFd))
  );
  
  ASSERT(pEr);
  CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CEndRunInfo12&>(*pEr));
  
  close(nullFd);
}
// bad nscldaq on expliciti create:

void ErInfoFactoryTests::explicitbad()
{
  CPPUNIT_ASSERT_THROW(
    CEndRunInfoFactory::create(
      static_cast<CEndRunInfoFactory::DAQVersion>(234), 0),
    std::domain_error
  );
}
// given a  file that leads with an 11.x format create and  11.0
// endrun info object.

void ErInfoFactoryTests::fromfile11()
{
  // Write an 11.x ring format item:
  
  V11::CDataFormatItem item;
  CFileDataSink sink(m_fd);
  writeItem(sink, item);

  // Should not need to rewind.
  
  CEndRunInfo* pObj = CEndRunInfoFactory::create(m_fd);
  close(m_fd);
  
  ASSERT(pObj);
  CEndRunInfo11* p11 = dynamic_cast<CEndRunInfo11*>(pObj);

  ASSERT(p11);
  
  delete pObj;

}
// Given a file without a format it's assumed to be a 10.

void ErInfoFactoryTests::fromfile10()
{
  //  Write an nscldaq10 begin run item:
  
  V10::CRingStateChangeItem item(V10::BEGIN_RUN, 10, 0, time(NULL), "This is a run");
  CFileDataSink sink(m_fd);
  writeItem(sink, item);
  
  CEndRunInfo* pObj = CEndRunInfoFactory::create(m_fd);
  ASSERT(pObj);
  CEndRunInfo10* p10 = dynamic_cast<CEndRunInfo10*>(pObj);
  ASSERT(p10);
  
  close(m_fd);
  delete pObj;
}

// file with a 12.0 format record
void ErInfoFactoryTests::fromfile12()
{
  // Make a format item for 12.0
  
  V12::CDataFormatItem item;
  CFileDataSink sink(m_fd);
  writeItem(sink, item);
  
  //  now the factory should throw a domain error:
  
  CEndRunInfo* pObj = CEndRunInfoFactory::create(m_fd);
  ASSERT(pObj);
  CEndRunInfo12* p12 = dynamic_cast<CEndRunInfo12*>(pObj);
  ASSERT(p12);
  close(m_fd);

  delete pObj;
}

// file with a 13.0 format record
void ErInfoFactoryTests::fromfileunrecog()
{

  // Make a fictitious format item for 13.0

  V12::CRawRingItem item;
  item.getBody() << uint64_t(1232142);

  CFileDataSink sink(m_fd);
  writeItem(sink, item);

  //  now the factory should throw a domain error:

  CPPUNIT_ASSERT_THROW(
    CEndRunInfoFactory::create(m_fd),
    std::domain_error
  );
}


// Empty file looks like 10.2 with no end run records.

void ErInfoFactoryTests::fromfileempty()
{
  int nullFd = open("/dev/null", O_RDONLY);

  CEndRunInfo* er = CEndRunInfoFactory::create(nullFd);
  EQMSG("empty file has no end runs",
        0u, er->numEnds());
}
