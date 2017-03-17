// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <string>
#include <io.h>

#include "CEndRunInfo10.h"

#include <V10/DataFormat.h>
#include <V10/CRingStateChangeItem.h>

#include <CFileDataSink.h>
#include <RingIOV10.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdexcept>

using namespace DAQ;

class EndInfo10Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EndInfo10Test);
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

CPPUNIT_TEST_SUITE_REGISTRATION(EndInfo10Test);

void EndInfo10Test::emptyFile() {
  // Open /dev/null and use that as the file:
  
  int nullFd = open("/dev/null", O_RDONLY);

  CPPUNIT_ASSERT_THROW( CEndRunInfo10 e(nullFd), std::runtime_error);

  close(nullFd);
}

void EndInfo10Test::oneEnd()
{
  // write a single end item.
  CFileDataSink sink(fd);
  
  time_t now = time(NULL);
  V10::CRingStateChangeItem item(V10::END_RUN, 1234, 456, now, "This is a title");
  writeItem(sink, item);
  
  // Rewind the file and create an end run info object on it:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  //  Test that we have the right stuff in the er object.
  
  EQ(unsigned(1), er.numEnds());
  ASSERT(!er.hasBodyHeader());     // 10.x never does.
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456), er.getElapsedTime());
  EQ(now, er.getTod());
  EQ(std::string("This is a title"), er.getTitle());
}

void EndInfo10Test::twoEnds()
{
  CFileDataSink sink(fd);

  // Create and write the first end run:
  
  time_t now = time(NULL);
  V10::CRingStateChangeItem item(V10::END_RUN, 1234, 456, now, "This is a title");
  
  writeItem(sink, item);

  // Slightly modify the item and write it a second time:
  
  item.setTimestamp(now+10);
  item.setElapsedTime(466);

  writeItem(sink, item);
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  // Fish info out of the er info object:
  
  EQ(unsigned(2), er.numEnds());
  
  // Distinguish between the two items:
  
  EQ(float(456), er.getElapsedTime());
  EQ(float(466), er.getElapsedTime(1));
  
  EQ(now, er.getTod());
  EQ(now+10, er.getTod(1));
}

void EndInfo10Test::bodyHeaderThrows()
{  
  CFileDataSink sink(fd);

  // Create and write the first end run:
  
  time_t now = time(NULL);
  V10::CRingStateChangeItem item(V10::END_RUN, 1234, 456, now, "This is a title");
  
  writeItem(sink, item);
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  
  // Anything involving body headers should throw std::string:
  
  
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

void EndInfo10Test::rangeThrows()
{
  CFileDataSink sink(fd);

  // Create and write the first end run:
  
  time_t now = time(NULL);
  V10::CRingStateChangeItem item(V10::END_RUN, 1234, 456, now, "This is a title");
  
  writeItem(sink, item);
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done

  // Attempting to get stuff for a non-existent end run buffer
  // throws std::range_error
  
  CPPUNIT_ASSERT_THROW(
    er.hasBodyHeader(1),
    std::range_error
  );
}
