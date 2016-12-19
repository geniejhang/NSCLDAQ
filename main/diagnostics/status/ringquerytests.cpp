// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <CSqliteWhere.h>
#include <CSqlite.h>
#include "CStatusMessage.h"
#include "CStatusDb.h"

#include <cstring>
#include <cstdlib>

class RingQTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingQTests);
  CPPUNIT_TEST(norings);
  CPPUNIT_TEST(onering);
  CPPUNIT_TEST(allrings);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb*  m_pDb;
  std::vector<std::pair<std::string, CStatusDefinitions::RingStatIdentification*> >
    m_RingDefs;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite | CSqlite::create);
    createRings();
    //createClients();
  }
  void tearDown() {
    delete m_pDb;
    freeRings();
  }
private:
  void createRings();
  void freeRings();
  void createClients();
  
protected:
  void norings();
  void onering();
  void allrings();
  
};
void 
RingQTests::freeRings()
{
  for (int i = 0; i  < m_RingDefs.size(); i++) {
    std::free(m_RingDefs[i].second);
  }
  m_RingDefs.clear();
}

static std::vector<std::pair<std::string, CStatusDefinitions::RingStatIdentification*> >
makeRingDefs()
{
  std::vector<std::pair<std::string, std::string> > ringAndHosts = {
    {"fox", "charlie.nscl.msu.edu"}, {"e15010", "spdaq20.nscl.msu.edu"},
    {"test", "charlie.nscl.msu.edu"}
  };
  std::vector<std::pair<std::string, CStatusDefinitions::RingStatIdentification*> > result ;
  
  for (int i = 0; i < ringAndHosts.size(); i++) {
    CStatusDefinitions::RingStatIdentification* id =
      CStatusDefinitions::makeRingid(ringAndHosts[i].first.c_str());
      
    id->s_tod = 1000;                           // Doesn't matter if no stats.
    
    std::pair<std::string, CStatusDefinitions::RingStatIdentification*> element(
      ringAndHosts[i].second, id
    );
    result.push_back(element);
  }
  return result;
}



// Utilities:

void RingQTests::createRings()
{

  auto ringdefs = makeRingDefs();
  
  std::vector<const CStatusDefinitions::RingStatClient*> stats;
  for (int i = 0; i < ringdefs.size(); i++) {
    CStatusDefinitions::RingStatIdentification& id(*ringdefs[i].second);

    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      ringdefs[i].first.c_str(),
      id, stats
    );
  }
}

/**
 * createRingClients
 *    Clients get made for the first and second of the rings
 *      (fox@charlie.nscl.msu.edu and e15010@spdaq20.nscl.msu.edu)
 */

// The tests:

CPPUNIT_TEST_SUITE_REGISTRATION(RingQTests);

void RingQTests::norings() {
  std::vector<CStatusDb::RingBuffer> results;
  CRawFilter f("0 = 1");                     // Never gets hits.
   
  m_pDb->listRings(results, f);
  
  EQ(size_t(0), results.size());
}
// LOTR references aside, this just gets the ring named e15010 -- if successful.

void RingQTests::onering()
{
  std::vector<CStatusDb::RingBuffer> results;
  CRelationToStringFilter f("r.name", CBinaryRelationFilter::equal, "e15010");
  
  m_pDb->listRings(results, f);
  
  EQ(size_t(1), results.size());
  
  // Now the contents:
  
  CStatusDb::RingBuffer& r(results[0]);
  EQ(unsigned(2), r.s_id);
  EQ(std::string("e15010@spdaq20.nscl.msu.edu"), r.s_fqname);
  EQ(std::string("e15010"), r.s_name);
  EQ(std::string("spdaq20.nscl.msu.edu"), r.s_host);
}
// List all rings.

void RingQTests::allrings()
{
  std::vector<CStatusDb::RingBuffer> results;
  
  m_pDb->listRings(results, DAQ::acceptAll);
  
  EQ(size_t(3), results.size());
  
  // Check the ordering (by fully qualified ring name) ids are 2,1,3:
  
  EQ(unsigned(2), results[0].s_id);
  EQ(unsigned(1), results[1].s_id);
  EQ(unsigned(3), results[2].s_id);
  
  
}