// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>


#include "CEndRunInfo11.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <V11/CRingStateChangeItem.h>
#include <V11/CRingItem.h>
#include <V11/DataFormatV11.h>

#include <RingIOV11.h>
#include <CFileDataSink.h>

#include <io.h>
#include <time.h>
#include <stdexcept>

using namespace DAQ;

class TestEndInfo11 : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestEndInfo11);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(oneWithBh);
  CPPUNIT_TEST(oneWoBH);
  CPPUNIT_TEST(noSuchEr1);
  CPPUNIT_TEST(nobodyHeaderThrows);

  // probably if I can do two I can do n.
  
  CPPUNIT_TEST(twoWithBh);
  CPPUNIT_TEST(twoWoBh);
  CPPUNIT_TEST(twoWithMixed);

  CPPUNIT_TEST_SUITE_END();


private:
  int fd;

public:
  void setUp() {

      // open a unique file atomically
      char tmplate[] = "testrunXXXXXX";
      fd = mkstemp(tmplate);
      unlink(tmplate);
   }
  void tearDown() {
  }
protected:
  void emptyFile();
  void oneWithBh();
  void oneWoBH();
  void noSuchEr1();
  void nobodyHeaderThrows();
  
  void twoWithBh();
  void twoWoBh();
  void twoWithMixed();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestEndInfo11);

void TestEndInfo11::emptyFile() {
  int nullFd = open("/dev/null",   O_RDONLY);
  CEndRunInfo11 er(nullFd);
  close(fd);                                        // By now we're done.
  
  EQ(0U, er.numEnds());                  // /dev/null has no end records.
}

// File with a single end run record.

void TestEndInfo11::oneWithBh()
{
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  V11::CRingStateChangeItem end(
    666, 1, 2,
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  EQ(1U, er.numEnds());
  
  // See if the contents are right:
  
  ASSERT(er.hasBodyHeader());
  EQ(uint64_t(666), er.getEventTimestamp());
  EQ(uint32_t(1), er.getSourceId());
  EQ(uint32_t(2),  er.getBarrierType());
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456.0), er.getElapsedTime());
  EQ(std::string("This is a title"), er.getTitle());
  EQ(now, er.getTod());
}

void TestEndInfo11::oneWoBH()
{
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  V11::CRingStateChangeItem end(
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  EQ(1U, er.numEnds());
  
  // See if the contents are right:
  
  ASSERT(!er.hasBodyHeader());
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456.0), er.getElapsedTime());
  EQ(std::string("This is a title"), er.getTitle());
  EQ(now, er.getTod());  
}

void TestEndInfo11::noSuchEr1()
{
  int nullFd = open("/dev/null",   O_RDONLY);
  CEndRunInfo11 er(nullFd);
  close(nullFd);                                        // By now we're done.
  
  // Asking for any info from any er should throw:
  
  
  CPPUNIT_ASSERT_THROW(
    er.hasBodyHeader(0),
    std::range_error
  );
}
void TestEndInfo11::nobodyHeaderThrows()
{
  // write a single End Run with a body header.
  
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  V11::CRingStateChangeItem end(
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Getting any body header info throws strings:
  
  CPPUNIT_ASSERT_THROW(
    er.getEventTimestamp(),
    std::string
  );
  CPPUNIT_ASSERT_THROW(
    er.getSourceId(),
    std::string
  );
  CPPUNIT_ASSERT_THROW(
    er.getBarrierType(),
    std::string
  );
}

// This run has two end run records with body headers.
//
void TestEndInfo11::twoWithBh()
{
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  
  // First one:
  
  V11::CRingStateChangeItem end(
    666, 1, 2,
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
    
  // Second one:
  
  V11::CRingStateChangeItem end2(
    676, 2, 2,
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end2);

  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Now check what we found:
  
  EQ(2U, er.numEnds());
  
  ASSERT(er.hasBodyHeader(0));
  ASSERT(er.hasBodyHeader(1));
  
  // These should be enough to tell us that we are getting the
  // right items.
  
  EQ(uint64_t(666), er.getEventTimestamp(0));
  EQ(uint64_t(676), er.getEventTimestamp(1));
  
  EQ(uint32_t(1), er.getSourceId());
  EQ(uint32_t(2), er.getSourceId(1));
  
  
}


void TestEndInfo11::twoWoBh()
{
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  
  // first one:
  
  V11::CRingStateChangeItem end(
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
  
  // Second one: + 10 seconds to tell them apart.

  V11::CRingStateChangeItem end2(
    V11::END_RUN, 1234, 456, now+10, "This is a title"
  );
  writeItem(sink, end2);

  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Now check what we found:
  
  EQ(2U, er.numEnds());
  
  ASSERT(!er.hasBodyHeader(0));
  ASSERT(!er.hasBodyHeader(1));
  
  // This should be enough to differentiate them:
  
  EQ(now, er.getTod());    // 0.
  EQ(now+10, er.getTod(1));
  
}

void TestEndInfo11::twoWithMixed()   // One with body hdr one without.
{
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  
  // First one:
  
  V11::CRingStateChangeItem end(
    666, 1, 2,
    V11::END_RUN, 1234, 456, now, "This is a title"
  );
  writeItem(sink, end);
  // second one:
  
  V11::CRingStateChangeItem end2(
    V11::END_RUN, 1234, 456, now+10, "This is a title"
  );
  writeItem(sink, end2);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Now check what we found:
  
  EQ(2U, er.numEnds());
  
  // Should be enough to differentiate.
  
  ASSERT(er.hasBodyHeader(0));
  ASSERT(!er.hasBodyHeader(1));
}
