// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CStatusMessage.h>
#include <CSqliteWhere.h>
#include <CSqlite.h>
#include "CStatusDb.h"



class TransQTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TransQTests);
  CPPUNIT_TEST(nomatch);
  CPPUNIT_TEST(matchreadout);
  CPPUNIT_TEST(matchreadout20);
  CPPUNIT_TEST(matchall);
  
  CPPUNIT_TEST(noapps);
  CPPUNIT_TEST(vmusbapp);
  CPPUNIT_TEST(allapps);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite);
    createTransitions();
  }
  void tearDown() {
    delete m_pDb;
  }
private:
  void createTransitions();
protected:
  void nomatch();
  void matchreadout();
  void matchreadout20();
  void matchall();
  
  void noapps();
  void vmusbapp();
  void allapps();
};

// Utilties:

// Put some nice transitions in the system:

void TransQTests::createTransitions()
{
  const char* app1="Readout";
  const char* app2="VMUSBReadout";
  const char* host1="charlie.nscl.msu.edu";
  const char* host2="spdaq20.nscl.msu.edu";
  
  int64_t time = 1000;                         // starting time.
  uint32_t sev = CStatusDefinitions::SeverityLevels::INFO;
  
  m_pDb->addStateChange(sev, app1, host1, time, "Ready", "Beginning");
  m_pDb->addStateChange(sev, app1, host2, time, "Ready", "Beginning");
  time++;                                       // Tick.
  m_pDb->addStateChange(sev, app2, host1, time, "Ready", "Beginning");
  
  time++;
  m_pDb->addStateChange(sev, app1, host1, time, "Beginning", "Active");
  m_pDb->addStateChange(sev, app2, host1, time, "Beginning", "Active");
  time++;
  m_pDb->addStateChange(sev, app1, host2, time, "Beginning", "Active");
  
}

// Tests

CPPUNIT_TEST_SUITE_REGISTRATION(TransQTests);

void TransQTests::nomatch() {
  CRawFilter f("0 = 1");
  
  std::vector<CStatusDb::StateTransition> result;
  m_pDb->queryStateTransitions(result, f);
  
  EQ(size_t(0), result.size());
}
void TransQTests::matchreadout()
{
  CRelationToStringFilter f("a.name", CBinaryRelationFilter::equal, "Readout");
  std::vector<CStatusDb::StateTransition> result;
  m_pDb->queryStateTransitions(result, f);
  
  EQ(size_t(4), result.size());
  EQ(std::string("Readout"), result[0].s_app.s_appName);
  EQ(std::string("charlie.nscl.msu.edu"), result[0].s_app.s_appHost);
  EQ(time_t(1000), result[0].s_timestamp);
  EQ(std::string("Ready"), result[0].s_leaving);
  EQ(std::string("Beginning"), result[0].s_entering);
  
  // Spot check - right host, timestamp and transition:
  
  EQ(std::string("spdaq20.nscl.msu.edu"), result[1].s_app.s_appHost);
  EQ(time_t(1000), result[1].s_timestamp);
  EQ(std::string("Ready"), result[1].s_leaving);
  EQ(std::string("Beginning"), result[1].s_entering);
  
  EQ(std::string("charlie.nscl.msu.edu"), result[2].s_app.s_appHost);
  EQ(time_t(1002), result[2].s_timestamp);
  EQ(std::string("Beginning"), result[2].s_leaving);
  EQ(std::string("Active"), result[2].s_entering);

  EQ(std::string("spdaq20.nscl.msu.edu"), result[3].s_app.s_appHost);
  EQ(time_t(1003), result[3].s_timestamp);
  EQ(std::string("Beginning"), result[3].s_leaving);
  EQ(std::string("Active"), result[3].s_entering);
  
  
}
void TransQTests::matchreadout20()
{
  CRelationToStringFilter appFilter("a.name", CBinaryRelationFilter::equal, "Readout");
  CRelationToStringFilter hostFilter("a.host", CBinaryRelationFilter::equal, "spdaq20.nscl.msu.edu");
  CAndFilter  f;
  f.addClause(appFilter);
  f.addClause(hostFilter);
  
  std::vector<CStatusDb::StateTransition> result;
  m_pDb->queryStateTransitions(result, f);
  EQ(size_t(2), result.size());
  
  // Spot check.
  
  EQ(time_t(1000), result[0].s_timestamp);
  EQ(time_t(1003), result[1].s_timestamp);
}
void TransQTests::matchall()
{
  std::vector<CStatusDb::StateTransition> result;
  m_pDb->queryStateTransitions(result, DAQ::acceptAll);
  EQ(size_t(6), result.size());                // There are 6 changes in all.
}

// No apps in listStateApplications:

void TransQTests::noapps()
{
  std::vector<CStatusDb::StateApp> result;
  CRawFilter f("0 = 1");
  
  m_pDb->listStateApplications(result, f);
  EQ(size_t(0), result.size());
}

void TransQTests::vmusbapp()
{
  std::vector<CStatusDb::StateApp> result;
  CRelationToStringFilter f("a.name", CBinaryRelationFilter::equal, "VMUSBReadout");
  
  m_pDb->listStateApplications(result, f);
  EQ(size_t(1), result.size());
  EQ(std::string("VMUSBReadout"), result[0].s_appName);
  EQ(std::string("charlie.nscl.msu.edu"), result[0].s_appHost);
}

void TransQTests::allapps()
{
  std::vector<CStatusDb::StateApp> result;
  m_pDb->listStateApplications(result, DAQ::acceptAll);
  
  EQ(size_t(3), result.size());
}