// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSqlite.h>
#include <CSqliteWhere.h>
#include <CStatusMessage.h>


#include "CStatusDb.h"



class ReadoutQTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ReadoutQTest);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite);
    createAppsAndRuns();
  }
  void tearDown() {
    delete m_pDb;
  }
private:
  void createAppsAndRuns();
protected:
  void aTest();
};
// Utilities

void ReadoutQTest::createAppsAndRuns()
{
  uint32_t sev = CStatusDefinitions::SeverityLevels::INFO;
  const char* app1="Readout";
  const char* app2="CCUSBReadout";
  
  const char* host1 = "charlie.nscl.msu.edu";
  const char* host2 = "spdaq20.nscl.msu.edu";
  
  uint32_t run(1);
  uint64_t startsAt(1000);
  
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt, run, "Run in charlie");
  m_pDb->addReadoutStatistics(sev, app2, host2, startsAt+1, run, "Run in spdaq20");
  
  startsAt += 30;                       // 30 seconds later  a new run starts.
  run++;
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20");
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie");
}

// Tests
CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutQTest);

void ReadoutQTest::aTest() {
}
