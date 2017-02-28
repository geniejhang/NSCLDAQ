
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <string>
#include <io.h>

#include "CEndRunInfo12.h"

#include <V12/DataFormat.h>
#include <V12/CRingStateChangeItem.h>
#include <CFileDataSink.h>
#include <RingIOV12.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdexcept>

using namespace DAQ;


class EndInfo12Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EndInfo12Test);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(oneEnd);
  CPPUNIT_TEST(twoEnds);
  CPPUNIT_TEST(bodyHeaderThrows);
  CPPUNIT_TEST(rangeThrows);
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
  void oneEnd();
  void twoEnds();
  void bodyHeaderThrows();
  void rangeThrows();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EndInfo12Test);

void EndInfo12Test::emptyFile() {
  // Open /dev/null and use that as the file:

  int nullFd = open("/dev/null", O_RDONLY);
  CEndRunInfo12 e(nullFd);
  close(nullFd);

  EQ(unsigned(0), e.numEnds());
}

void EndInfo12Test::oneEnd()
{
  // write a single end item.
  CFileDataSink sink(fd);

  time_t now = time(NULL);
  V12::CRingStateChangeItem item(V12::END_RUN, 1234, 456, now, "This is a title");
  writeItem(sink, item);

  // Rewind the file and create an end run info object on it:

  lseek(fd, 0, SEEK_SET);
  CEndRunInfo12 er(fd);
  close(fd);                           // By now we're done

  //  Test that we have the right stuff in the er object.

  EQ(unsigned(1), er.numEnds());
  ASSERT(!er.hasBodyHeader());     // 12.x never does.

  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456), er.getElapsedTime());
  EQ(now, er.getTod());
  EQ(std::string("This is a title"), er.getTitle());
}

void EndInfo12Test::twoEnds()
{
  CFileDataSink sink(fd);

  // Create and write the first end run:

  time_t now = time(NULL);
  V12::CRingStateChangeItem item(V12::END_RUN, 1234, 456, now, "This is a title");

  writeItem(sink, item);

  // Slightly modify the item and write it a second time:

  item.setTimestamp(now+10);
  item.setElapsedTime(466);

  writeItem(sink, item);

  // Create the ER info item and close the file:

  lseek(fd, 0, SEEK_SET);
  CEndRunInfo12 er(fd);
  close(fd);                           // By now we're done

  // Fish info out of the er info object:

  EQ(unsigned(2), er.numEnds());

  // Distinguish between the two items:

  EQ(float(456), er.getElapsedTime());
  EQ(float(466), er.getElapsedTime(1));

  EQ(now, er.getTod());
  EQ(now+10, er.getTod(1));
}

void EndInfo12Test::bodyHeaderThrows()
{
  CFileDataSink sink(fd);

  // Create and write the first end run:

  time_t now = time(NULL);
  V12::CRingStateChangeItem item(123, 34, V12::END_RUN, 1234, 456, now, "This is a title");

  writeItem(sink, item);

  // Create the ER info item and close the file:

  lseek(fd, 0, SEEK_SET);
  CEndRunInfo12 er(fd);
  close(fd);                           // By now we're done


  // Anything involving body headers should throw std::string:


    EQMSG("tstamp", uint64_t(123), er.getEventTimestamp());

    EQMSG("source id", uint32_t(34), er.getSourceId());
  CPPUNIT_ASSERT_THROW(
    er.getBarrierType(),
    std::runtime_error
  );
}

void EndInfo12Test::rangeThrows()
{
  CFileDataSink sink(fd);

  // Create and write the first end run:

  time_t now = time(NULL);
  V12::CRingStateChangeItem item(V12::END_RUN, 1234, 456, now, "This is a title");

  writeItem(sink, item);

  // Create the ER info item and close the file:

  lseek(fd, 0, SEEK_SET);
  CEndRunInfo12 er(fd);
  close(fd);                           // By now we're done

  // Attempting to get stuff for a non-existent end run buffer
  // throws std::range_error

  CPPUNIT_ASSERT_THROW(
    er.hasBodyHeader(1),
    std::range_error
  );
}
