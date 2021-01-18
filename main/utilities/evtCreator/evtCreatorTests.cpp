// Template for a test suite.
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <CRingBuffer.h>
#include <CDataSinkFactory.h>
#include <CDataSink.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <string>
#include <iostream>

extern std::string uniqueName(std::string);

class EvtCreatorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EvtCreatorTest);
  CPPUNIT_TEST(testrun);
  CPPUNIT_TEST_SUITE_END();

private:
  CDataSink* pSink;
public:
  void setUp() {
    CDataSinkFactory factory;
    pSink = factory.makeSink("file://./test.evt");
  }
  void tearDown() {
    delete pSink;
  }
private: 
  pid_t startEventCreator(std::string switches);
protected:
  void testrun();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EvtCreatorTest);

/**
 * start event creator - assuming it's in this directory.
 * @param tail - the command tail after the program invocation
 * @note cheat - use fork/system.
 */
pid_t
EvtCreatorTest::startEventCreator(std::string tail)
{
  pid_t pid = fork();
  if (!pid) {
    std::string command("./evtCreator ");
    command += tail;
    system(command.c_str());
    exit(EXIT_SUCCESS);
  }
  sleep(2);			
  return pid;
}

/**
 * test run file.
 * - Start event log with: -p 1 -n 1
 * - Send a begin run to the ring.
 * - Send an end run to the ring.
 * - Ensure there's an event file with the right name.
 * - unlink the event file.
 */
void EvtCreatorTest::testrun() {
  std::string options = ("-p 1 -n 1 -s file://./test.evt");
  pid_t evlogPid = startEventCreator(options);

  int status;
  waitpid(evlogPid, &status, 0);

  // Check for the event file.
  const char* pFilename = "test.evt";
  status = access(pFilename, F_OK);
  EQ(0, status);

  unlink(pFilename);
}

