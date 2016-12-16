// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CStatusDb.h"
#include <CSqliteWhere.h>
#include <CSqlite.h>
#include "CStatusMessage.h"



class LogQTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LogQTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(onematch);
  CPPUNIT_TEST(inmatch);
  CPPUNIT_TEST(allmatch);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite);
    
    // Put some records into the database:
    
    m_pDb->addLogMessage(
      CStatusDefinitions::SeverityLevels::INFO, "app1", "spdaq20.nscl.msu.edu",
      1000, "Some sample message"
    );                                     // id=1
    m_pDb->addLogMessage(
      CStatusDefinitions::SeverityLevels::INFO, "app2", "charlie.nscl.msu.edu",
      1001, "A second sample message"
    );
    m_pDb->addLogMessage(
      CStatusDefinitions::SeverityLevels::WARNING, "app1", "spdaq20.nscl.msu.edu",
      1002, "a third sample message"
    );
  }
  void tearDown() {
    delete m_pDb;
  }
protected:
  void empty();
  void onematch();
  void inmatch();
  void allmatch();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LogQTest);

// No matches gives empty result set.

void LogQTest::empty() {
  std::vector<CStatusDb::LogRecord> recs;
  CRawFilter none("0 = 1");                // never matches.
  m_pDb->queryLogMessages(recs, none);
  
  EQ(size_t(0), recs.size());
}
// Match on application = app2 should give the second log message.

void LogQTest::onematch()
{
  std::vector<CStatusDb::LogRecord> recs;
  CRelationToStringFilter sel("application", CBinaryRelationFilter::equal, "app2");
  
  m_pDb->queryLogMessages(recs, sel);
  
  EQ(size_t(1), recs.size());
  
  // Now look at the results contents.
  
  CStatusDb::LogRecord& r(recs[0]);
  EQ(unsigned(2), r.s_id);
  EQ(std::string("INFO"), r.s_severity);
  EQ(std::string("app2"), r.s_application);
  EQ(std::string("charlie.nscl.msu.edu"), r.s_source);
  EQ(std::time_t(1001), r.s_timestamp);
  EQ(std::string("A second sample message"), r.s_message);
}

// Match for id set:

void LogQTest::inmatch()
{
  std::vector<double> ids = {1, 3};
  CInFilter f("id", ids);
  
  std::vector<CStatusDb::LogRecord> recs;
  m_pDb->queryLogMessages(recs, f);
  
  EQ(size_t(2), recs.size());
  
  // Assume that if the id is right the rest is probably too given onematch passing
  
  EQ(unsigned(1), recs[0].s_id);
  EQ(unsigned(3), recs[1].s_id);
}
// Get all records:

void LogQTest::allmatch()
{
  std::vector<CStatusDb::LogRecord> recs;
  m_pDb->queryLogMessages(recs, DAQ::acceptAll);
  
  EQ(size_t(3), recs.size());
  
  // Assume the prior tests have shown everything else is ok.
}