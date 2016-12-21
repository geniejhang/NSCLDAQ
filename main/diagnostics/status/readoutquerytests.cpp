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
  CPPUNIT_TEST(listnomatch);
  CPPUNIT_TEST(listCCUSBReadout);
  CPPUNIT_TEST(listAll);
  
  CPPUNIT_TEST(noruns);
  CPPUNIT_TEST(run1);
  CPPUNIT_TEST(allruns);
  
  CPPUNIT_TEST(nostats);
  CPPUNIT_TEST(run1charliestats);
  CPPUNIT_TEST(run1counters);
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
  void listnomatch();
  void listCCUSBReadout();
  void listAll();
  
  void noruns();
  void run1();
  void allruns();
  
  void nostats();
  void run1charliestats();
  void run1counters();
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
  
  // Add some statistics for app1:
  
  CStatusDefinitions::ReadoutStatCounters c1 = {1001, 1, 100, 200, 1024};
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt, run, "Run in charlie", &c1);
  c1.s_tod += 2;
  c1.s_elapsedTime +=2;
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt, run, "Run in charlie", &c1);
  c1.s_tod += 2;
  c1.s_elapsedTime +=2;
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt, run, "Run in charlie", &c1);
   
  // And for app2:
  
  CStatusDefinitions::ReadoutStatCounters c2 = {1002, 1, 50, 25, 512};
  m_pDb->addReadoutStatistics(sev, app2, host2, startsAt+1, run, "Run in spdaq20", &c2);
  c2.s_tod += 2;
  c2.s_elapsedTime += 2;
  m_pDb->addReadoutStatistics(sev, app2, host2, startsAt+1, run, "Run in spdaq20", &c2);
  
  c2.s_tod += 2;
  c2.s_elapsedTime += 2;
  m_pDb->addReadoutStatistics(sev, app2, host2, startsAt+1, run, "Run in spdaq20", &c2);

  
  startsAt += 30;                       // 30 seconds later  a new run starts.
  run++;
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20");
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie");
  
  // Add statistics for app1:
  
  CStatusDefinitions::ReadoutStatCounters c3 = {3034, 2, 1000, 750, 10240};
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie", &c3);
  c3.s_tod += 1;
  c3.s_elapsedTime += 1;
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie", &c3);
  
  c3.s_tod += 1;
  c3.s_elapsedTime += 1;
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie", &c3);

  c3.s_tod += 1;
  c3.s_elapsedTime += 1;
  m_pDb->addReadoutStatistics(sev, app1, host1, startsAt+2, run, "Second run in charlie", &c3);

  
  // Add statistics for app2:
  
  CStatusDefinitions::ReadoutStatCounters c4 = {3030, 2, 500, 500, 10240};
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20", &c4);

  c4.s_tod +=1;
  c4.s_elapsedTime+=1;
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20", &c4);
  
  c4.s_tod +=1;
  c4.s_elapsedTime+=1;
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20", &c4);

  c4.s_tod +=1;
  c4.s_elapsedTime+=1;
  m_pDb->addReadoutStatistics(sev,app2, host2, startsAt, run, "Second run in spdaq20", &c4);

}

// Tests
CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutQTest);

void ReadoutQTest::listnomatch() {
  CRawFilter f("0 = 1");
  std::vector<CStatusDb::ReadoutApp> result;
  
  m_pDb->listReadoutApps(result, f);
  EQ(size_t(0), result.size());
}

void ReadoutQTest::listCCUSBReadout()
{
  CRelationToStringFilter f("a.name", CBinaryRelationFilter::equal, "CCUSBReadout");
  std::vector<CStatusDb::ReadoutApp> result;
  
  m_pDb->listReadoutApps(result, f);
  EQ(size_t(1), result.size());
  
  EQ(unsigned(2), result[0].s_id);
  EQ(std::string("CCUSBReadout"), result[0].s_appName);
  EQ(std::string("spdaq20.nscl.msu.edu"), result[0].s_appHost);
}

void ReadoutQTest::listAll()
{
  std::vector<CStatusDb::ReadoutApp> result;
  m_pDb->listReadoutApps(result, DAQ::acceptAll);
  
  EQ(size_t(2), result.size());
  
  EQ(std::string("Readout"), result[0].s_appName);
  EQ(std::string("charlie.nscl.msu.edu"), result[0].s_appHost);
  
  EQ(std::string("CCUSBReadout"), result[1].s_appName);
  EQ(std::string("spdaq20.nscl.msu.edu"), result[1].s_appHost);
}

// No runs match 

void ReadoutQTest::noruns()
{
  CStatusDb::RunDictionary result;
  CRawFilter f("0 = 1");
  
  m_pDb->listRuns(result, f);
  EQ(size_t(0), result.size());
}
// Match run 1.

void ReadoutQTest::run1()
{
  CStatusDb::RunDictionary result;
  CRelationToNumberFilter f("r.run", CBinaryRelationFilter::equal, 1);
  
  m_pDb->listRuns(result, f);
  EQ(size_t(2), result.size());
  
  // id 1 is Readout@charlie. for run 1:
  
  CStatusDb::ApplicationRun& r1(result[1]);
  EQ(std::string("Readout"), r1.first.s_appName);
  EQ(std::string("charlie.nscl.msu.edu"), r1.first.s_appHost);
  
    // There's only 1 run it's run1:
  
  EQ(size_t(1), r1.second.size());
  CStatusDb::RunInfo& r1run(r1.second[0]);
  EQ(uint64_t(1000), r1run.s_startTime);
  EQ(uint32_t(1),    r1run.s_runNumber);
  EQ(std::string("Run in charlie"), r1run.s_runTitle);
  
    
  
  // id2 is CCUSBReadout@spdaq20
  
  CStatusDb::ApplicationRun& r2(result[2]);
  EQ(std::string("CCUSBReadout"), r2.first.s_appName);
  EQ(std::string("spdaq20.nscl.msu.edu"), r2.first.s_appHost);
  
     // There's only 1 run and its run 1 (inspdaq 20)
  
  CStatusDb::RunInfo& r2run(r2.second[0]);
  EQ(uint64_t(1001), r2run.s_startTime);
  EQ(uint32_t(1), r2run.s_runNumber);
  EQ(std::string("Run in spdaq20"), r2run.s_runTitle);
  
}
// no filter on the runs:

void ReadoutQTest::allruns()
{
  CStatusDb::RunDictionary result;
  m_pDb->listRuns(result, DAQ::acceptAll);
  
  EQ(size_t(2), result.size());   // should have 2 readouts.
  
  CStatusDb::ApplicationRun& r1(result[1]);  // Readout
  EQ(size_t(2), r1.second.size());           // has two runs.
  
  // Spot check the runs -- just look at the start times:
  
  
  EQ(uint64_t(1000), r1.second[0].s_startTime);
  EQ(uint64_t(1032), r1.second[1].s_startTime);
  
  CStatusDb::ApplicationRun& r2(result[2]);  // CCUSBReadout.
  EQ(size_t(2), r2.second.size());           /// has 2 runs.
  
  EQ(uint64_t(1001), r2.second[0].s_startTime);
  EQ(uint64_t(1030), r2.second[1].s_startTime);
  
  
}
// No statistics returned:

void ReadoutQTest::nostats()
{
  CStatusDb::ReadoutStatDict result;
  CRawFilter                 f("0 = 1");
  m_pDb->queryReadoutStatistics(result, f);
  
  EQ(size_t(0), result.size());
}
void ReadoutQTest::run1charliestats()
{
  CStatusDb::ReadoutStatDict result;
  
  // Want run #1 and the apps in charlie:
  
  CRelationToNumberFilter runSelect("r.run", CBinaryRelationFilter::equal, 1);
  CRelationToStringFilter hostSelect("a.host", CBinaryRelationFilter::equal, "charlie.nscl.msu.edu");
  CAndFilter f;
  f.addClause(runSelect);
  f.addClause(hostSelect);
  
  m_pDb->queryReadoutStatistics(result, f);
  
  // Only one result
  
  EQ(size_t(1), result.size());
  
  CStatusDb::ReadoutAppStats& appStats(result[1]);    // apid 1.
  CStatusDb::ReadoutApp&      app(appStats.first);
  
  EQ(std::string("Readout"), app.s_appName);
  EQ(std::string("charlie.nscl.msu.edu"), app.s_appHost);
  
  std::vector<CStatusDb::RunStatistics>& stats(appStats.second);
  EQ(size_t(1), stats.size());                         // one run.
  
  CStatusDb::RunInfo&   rinfo(stats[0].first);
  std::vector<CStatusDb::ReadoutStatistics>& counters(stats[0].second);
  
  EQ(uint64_t(1000), rinfo.s_startTime);
  EQ(uint32_t(1),  rinfo.s_runNumber);
  EQ(std::string("Run in charlie"), rinfo.s_runTitle);
  
  // Run1 has 3 statistics entries.  They only differ by the timestamp.
  // Fully analyze the first counters record but then just look at the timestamps
  // for the remainder.
  
  EQ(size_t(3), counters.size());
  EQ(time_t(1001), counters[0].s_timestamp);
  EQ(unsigned(1),  counters[0].s_elapsedTime);
  EQ(uint64_t(100), counters[0].s_triggers);
  EQ(uint64_t(200), counters[0].s_events);
  EQ(uint64_t(1024), counters[0].s_bytes);
  
  EQ(time_t(1003), counters[1].s_timestamp);
  EQ(time_t(1005), counters[2].s_timestamp);
  
  
}
// All run 1 statistics (both readout programs).

void ReadoutQTest::run1counters()
{
    CStatusDb::ReadoutStatDict result;
    CRelationToNumberFilter f("r.run", CBinaryRelationFilter::equal, 1);
    
    m_pDb->queryReadoutStatistics(result, f);
    
    // Shouild be two apps:
    
    EQ(size_t(2), result.size());
    CStatusDb::ReadoutAppStats& app1(result[1]);          // Readout
    CStatusDb::ReadoutAppStats& app2(result[2]);          // CCUSBReadout.
    
    EQ(std::string("Readout"), app1.first.s_appName);
    EQ(std::string("charlie.nscl.msu.edu"), app1.first.s_appHost);
    
    EQ(std::string("CCUSBReadout"), app2.first.s_appName);
    EQ(std::string("spdaq20.nscl.msu.edu"), app2.first.s_appHost);
    
    // There should be one run in each of these:
    
    EQ(size_t(1), app1.second.size());
    CStatusDb::RunStatistics r1(app1.second[0]);
    EQ(uint64_t(1000), r1.first.s_startTime);
    EQ(uint32_t(1),  r1.first.s_runNumber);
    EQ(std::string("Run in charlie"), r1.first.s_runTitle);
  
    
    EQ(size_t(1), app2.second.size());
    CStatusDb::RunStatistics r2(app2.second[0]);
    EQ(uint64_t(1001), r2.first.s_startTime);
    EQ(uint32_t(1), r2.first.s_runNumber);
    EQ(std::string("Run in spdaq20"), r2.first.s_runTitle);
    
    std::vector<CStatusDb::ReadoutStatistics> c1(r1.second);
    std::vector<CStatusDb::ReadoutStatistics> c2(r2.second);
    
    // Each of the vectors has three elements:
    
    EQ(size_t(3), c1.size());
    EQ(size_t(3), c2.size());
    
    // Check details on the first counter element for each vector and
    // spot check the timing of the other elements.
    
  EQ(size_t(3), c1.size());
  EQ(time_t(1001), c1[0].s_timestamp);
  EQ(unsigned(1),  c1[0].s_elapsedTime);
  EQ(uint64_t(100), c1[0].s_triggers);
  EQ(uint64_t(200), c1[0].s_events);
  EQ(uint64_t(1024), c1[0].s_bytes);
  
  EQ(time_t(1003), c1[1].s_timestamp);
  EQ(time_t(1005), c1[2].s_timestamp);
  
  EQ(size_t(3), c2.size());
  EQ(time_t(1002), c2[0].s_timestamp);
  EQ(unsigned(1), c2[0].s_elapsedTime);
  EQ(uint64_t(50), c2[0].s_triggers);
  EQ(uint64_t(25), c2[0].s_events);
  EQ(uint64_t(512), c2[0].s_bytes);
  
  
}