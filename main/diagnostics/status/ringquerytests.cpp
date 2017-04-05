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
  
  CPPUNIT_TEST(noclients);
  CPPUNIT_TEST(oneringwithclients);
  CPPUNIT_TEST(allwithclients);

  CPPUNIT_TEST(statsnone);
  CPPUNIT_TEST(statsforfox);
  CPPUNIT_TEST(statsforall);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb*  m_pDb;
  std::vector<std::pair<std::string, CStatusDefinitions::RingStatIdentification*> >
    m_RingDefs;
  std::vector<std::vector<const CStatusDefinitions::RingStatClient*> > m_clients;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite | CSqlite::create);
    createRings();
    createClients();
    addStatistics();
  }
  void tearDown() {
    delete m_pDb;
    freeRings();
    freeClients();
  }
private:
  void createRings();
  void freeRings();
  void freeClients();
  void createClients();
  void addStatistics();
  
  
protected:
  void norings();
  void onering();
  void allrings();
  
  void noclients();
  void oneringwithclients();
  void allwithclients();
  
  void statsnone();
  void statsforfox();
  void statsforall();
};
void 
RingQTests::freeRings()
{
  for (int i = 0; i  < m_RingDefs.size(); i++) {
    std::free(m_RingDefs[i].second);
  }
  m_RingDefs.clear();
}

void RingQTests::freeClients()
{
  for (int i = 0; i < m_clients.size(); i++) {
    std::vector<const CStatusDefinitions::RingStatClient*>& c(m_clients[i]);
    for(int j =0; j < c.size(); j++) {
      std::free(const_cast<void *>(reinterpret_cast<const void*>((c[j]))));
    }
    c.clear();
  }
  m_clients.clear();
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

// createClients
//      Create clients for the rings.
  
void RingQTests::createClients()
{
  std::vector<std::string> cmd1 = {
    "this", "is", "a", "test"
  };
  std::vector<std::string> cmd2 = {
    "/usr/opt/daq/11.0/bin/dumper", "--source=tcp://localhost/fox"
  };
  std::vector<std::string> cmd3 = {
    "/usr/opt/daq/current/bin/Readout", "--ring=fox", "--sourceid=3"
  };
  
  std::vector<const CStatusDefinitions::RingStatClient*> clients;
  
  // For first ring:
  
  clients.push_back(CStatusDefinitions::makeRingClient(123, 5000, 100, 678, false, cmd1));
  clients.push_back(CStatusDefinitions::makeRingClient(100, 1000, 0, 999, true, cmd3));
  
  m_clients.push_back(clients);
  
  // For second ring:
  
  clients.clear();
  
  clients.push_back(CStatusDefinitions::makeRingClient(128, 2048, 0, 768, true, cmd3));
  clients.push_back(CStatusDefinitions::makeRingClient(0, 0, 0, 900, false, cmd1));
  clients.push_back(CStatusDefinitions::makeRingClient(100, 1024, 1024, 999, false, cmd2));
  m_clients.push_back(clients);
  
  // Nothing for third ring
  
  clients.clear();
  m_clients.push_back(clients);
  
  // The code below assumes that m_ringDefs and m_clients have the same number
  // of elements.
  
  for (int i = 0; i < m_RingDefs.size(); i++) {
    
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon", 
      m_RingDefs[i].first.c_str(),
      *m_RingDefs[i].second, m_clients[i]
    );
  }
}

/// Add a few copies of the statistics with different timestamps.

void RingQTests::addStatistics()
{
  for(int t =0; t < 5; t ++) {
    for (int i =0; i < m_RingDefs.size(); i++) {
      m_RingDefs[i].second->s_tod += 2;              // Two seconds apart.
      m_pDb->addRingStatistics(
        CStatusDefinitions::SeverityLevels::INFO, "ringdaemon", 
        m_RingDefs[i].first.c_str(),
        *m_RingDefs[i].second, m_clients[i]
      );
    }
  }
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
  m_RingDefs = ringdefs;
}


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

// NO matching clients:

void RingQTests::noclients()
{
  CStatusDb::RingDirectory results;
  CRelationToStringFilter f("r.name", CBinaryRelationFilter::equal, "test");
  
  m_pDb->listRingsAndClients(results, f);
  EQ(size_t(0), results.size());
}
// Require the ring 'fox' which will have a pair of clients:

void RingQTests::oneringwithclients()
{
  CStatusDb::RingDirectory results;
  CRelationToStringFilter f("r.name", CBinaryRelationFilter::equal, "fox");
  
  m_pDb->listRingsAndClients(results, f);
  
  EQ(size_t(1), results.size());
  
  // The map entry "fox@charlie.nscl.msu.edu" will have a pair of consumers:
  
  CStatusDb::RingAndClients& clientInfo(results["fox@charlie.nscl.msu.edu"]);
  EQ(size_t(2), clientInfo.second.size());
  
  // The first client is from
  // clients.push_back(CStatusDefinitions::makeRingClient(123, 5000, 100, 678, false, cmd1));
 
  CStatusDb::RingClient& client0(clientInfo.second[0]);
 
  EQ(pid_t(678), client0.s_pid);
  EQ(false,      client0.s_isProducer);
  EQ(std::string("this is a test"), client0.s_command);
 
  // The second client is from:
  // clients.push_back(CStatusDefinitions::makeRingClient(100, 1000, 0, 999, true, cmd3));
   
  CStatusDb::RingClient& client1(clientInfo.second[1]);
  EQ(pid_t(999), client1.s_pid);
  EQ(true,       client1.s_isProducer);
  EQ(std::string( "/usr/opt/daq/current/bin/Readout --ring=fox --sourceid=3"), client1.s_command);
}

// all withclients  -- well there are only clients on the first two rings so we'll
// get two hits.  Spot check by looking at the PIDS in all of the records
// for each ring.

void RingQTests::allwithclients()
{
    CStatusDb::RingDirectory result;
    m_pDb->listRingsAndClients(result, DAQ::acceptAll);
    
    // Should have two map entries:
    
    EQ(size_t(2), result.size());
    
    // Let's look at the two result records; check size and pids of all
    // the clients:
    
    CStatusDb::RingAndClients& r1(result["fox@charlie.nscl.msu.edu"]);
    EQ(size_t(2), r1.second.size());
    std::vector<CStatusDb::RingClient>& r1c(r1.second);
    EQ(pid_t(678), r1c[0].s_pid);
    EQ(pid_t(999), r1c[1].s_pid);
    
    CStatusDb::RingAndClients& r2(result["e15010@spdaq20.nscl.msu.edu"]);
    EQ(size_t(3), r2.second.size());
    std::vector<CStatusDb::RingClient>& r2c(r2.second);

    EQ(pid_t(768), r2c[0].s_pid);
    EQ(pid_t(900), r2c[1].s_pid);
    EQ(pid_t(999), r2c[2].s_pid);
}
// Ring statistics with no results:

void RingQTests::statsnone()
{
  CStatusDb::CompleteRingStatistics results;
  CRawFilter f("0 = 1");                 // Never true.
  
  m_pDb->queryRingStatistics(results, f);
  
  EQ(size_t(0), results.size());
}
// Get statistics for fox - there will be one ring buffer.
// It will have a a pair of clients, each with 6 statistics entries that
// are identical but 2 seconds apart in their timsetamp.

void RingQTests::statsforfox()
{
    CStatusDb::CompleteRingStatistics results;
    CRelationToStringFilter           f("r.name", CBinaryRelationFilter::equal, "fox");
    
    m_pDb->queryRingStatistics(results, f);
    
    // one ring:
    
    EQ(size_t(1), results.size());
    EQ(size_t(1), results.count("fox@charlie.nscl.msu.edu"));     // the fqringname
    
    CStatusDb::RingsAndStatistics& ringData(results["fox@charlie.nscl.msu.edu"]);
    
    // Check the ring buffer data:
    
    EQ(std::string("fox@charlie.nscl.msu.edu"), ringData.first.s_fqname);
    EQ(std::string("fox"), ringData.first.s_name);
    EQ(std::string("charlie.nscl.msu.edu"), ringData.first.s_host);
    
    std::vector<CStatusDb::RingClientAndStats>& clientStats(ringData.second);
    EQ(size_t(2), clientStats.size());    // Two clients.
    
    // Look at the client info:
    
    CStatusDb::RingClient& c1(clientStats[0].first);
    EQ(pid_t(678), c1.s_pid);
    EQ(false, c1.s_isProducer);
    EQ(std::string("this is a test"), c1.s_command);
    
    CStatusDb::RingClient& c2(clientStats[1].first);
    EQ(pid_t(999), c2.s_pid);
    EQ(true, c2.s_isProducer);
    EQ(std::string("/usr/opt/daq/current/bin/Readout --ring=fox --sourceid=3"), c2.s_command);
    
    // Look at the stats for client 1 - number of them, detailed analysis
    // of the first one, and require that the time differences be 2 seconds:
    
    std::vector<CStatusDb::RingStatistics>& s1(clientStats[0].second);
    EQ(size_t(6), s1.size());
    time_t t(1000);                     // initial time:
    EQ(t, s1[0].s_timestamp);
    EQ(uint64_t(123), s1[0].s_operations);
    EQ(uint64_t(5000), s1[0].s_bytes);
    EQ(uint64_t(100), s1[0].s_backlog);
    for(int i = 1; i < 6; i++) {
      t += 2;
      EQ(t, s1[i].s_timestamp);
    }
    
    std::vector<CStatusDb::RingStatistics>& s2(clientStats[1].second);
    EQ(size_t(6), s2.size());
    t = 1000;                          // reset the timestamp.
    EQ(t, s2[0].s_timestamp);
    EQ(uint64_t(100), s2[0].s_operations);
    EQ(uint64_t(1000), s2[0].s_bytes);
    EQ(uint64_t(0), s2[0].s_backlog);
    
    for (int  i = 1; i < 6; i++) {
      t += 2;
      EQ(t, s2[i].s_timestamp);
    }
}
//  Stats with 1=1 filter:

void RingQTests::statsforall()
{
    CStatusDb::CompleteRingStatistics result;
    m_pDb->queryRingStatistics(result, DAQ::acceptAll);
    
    // Two rings have statistics, fox@charlie, e15010@spdaq20:
    
    EQ(size_t(2), result.size());
    EQ(size_t(1), result.count("fox@charlie.nscl.msu.edu"));
    EQ(size_t(1), result.count("e15010@spdaq20.nscl.msu.edu"));
    
    // Assume the rest is ok.
}