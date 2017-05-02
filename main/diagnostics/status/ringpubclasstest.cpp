// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

// White box the class under test.

#define private public
#include "CPublishRingStatistics.h"
#undef private


#include "CStatusMessage.h"
#include <CRingBuffer.h>
#include <CRingMaster.h>
#include "TCLInterpreter.h"
#include "TCLObject.h"

#include <zmq.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <errno.h>
#include <os.h>
#include <sys/types.h>
#include <unistd.h>
#include <testutils.h>
#include <nsclzmq.h>

static const char* uri="inproc://test";
static std::vector<std::string> command(Os::getProcessCommand(getpid()));



// @note - the ring master must be running for these tests to work.

class RingPubTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingPubTests);
  CPPUNIT_TEST(emptyring);
  CPPUNIT_TEST(ringWithProducer);
  CPPUNIT_TEST(ringWithConsumer);
  CPPUNIT_TEST(ringWithProducerAndConsumer);
  CPPUNIT_TEST(ringWithProducerSeveralConsumers);
  CPPUNIT_TEST(multipleRings);
  CPPUNIT_TEST(getHistoryIndexNotFound);
  CPPUNIT_TEST(getHistoryIndexFound);
  CPPUNIT_TEST(makeLogMessage);
  CPPUNIT_TEST(logLargeNoPriorHistory);
  CPPUNIT_TEST(logLargeNoPriorConsumer);
  CPPUNIT_TEST(logLargeLog);
  CPPUNIT_TEST(logLargeNoLog);
  CPPUNIT_TEST(logOkLog);
  CPPUNIT_TEST(logOkNolog);
  CPPUNIT_TEST(logOkNologNoClient);
  CPPUNIT_TEST(logOkNologNohistory);
  
  CPPUNIT_TEST_SUITE_END();


private:
  ZmqSocket*          m_pSender;
  ZmqSocket*          m_pReceiver;
  CPublishRingStatistics* m_pPublisher;
  
public:
  void setUp() {
    killRings();                       // In setup in case we start with rings.
    
    // Setup the zmq connections sender is a PUSH and receiver a PULL, and we'll
    // directly receive/analyze raw messages.
    
    m_pSender     = ZmqObjectFactory::createSocket( ZMQ_PUSH);
    m_pReceiver   = ZmqObjectFactory::createSocket( ZMQ_PULL);
    
    (*m_pSender)->bind(uri);
    (*m_pReceiver)->connect(uri);
    
    // Now we can set up the publisher
    
    m_pPublisher = new CPublishRingStatistics(*m_pSender, "Test Application");
  
  }
  void tearDown() {
    delete m_pPublisher;
    delete m_pSender;
    delete m_pReceiver;
    ZmqObjectFactory::shutdown();
    killRings();                        // no rings on exit too.
  }
protected:
  void emptyring();
  void ringWithProducer();
  void ringWithConsumer();
  void ringWithProducerAndConsumer();
  void ringWithProducerSeveralConsumers();
  void multipleRings();
  
  void getHistoryIndexNotFound();
  void getHistoryIndexFound();
  void makeLogMessage();
  void logLargeNoPriorHistory();
  void logLargeNoPriorConsumer();
  void logLargeLog();
  void logLargeNoLog();
  void logOkLog();
  void logOkNolog();
  void logOkNologNoClient();
  void logOkNologNohistory();
private:
    std::vector<zmq::message_t*> receiveMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingPubTests);

/*--------------------------- Utility methods -----------------------------*/

// Recieve a multipart message and return it.

std::vector<zmq::message_t*>
RingPubTests::receiveMessage()
{
  return ::receiveMessage(*m_pReceiver);
}

/*-------------------------------------- Tests ----------------------------*/

// Make a ring buffer and publish. We should get a single message stream with
// no producers or consumers, but a ring id segment.

void RingPubTests::emptyring() {
  CRingBuffer::create("test_ring");             // Make the test ring.
  (*m_pPublisher)();                            // Publish ring info.
  
  std::vector<zmq::message_t*> message = receiveMessage();
  
  EQ(size_t(2), message.size());           // Header and ring id only.
  
  // Ensure header correctness.
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(message[0]->data());
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
  EQ(std::string("Test Application"), std::string(pHeader->s_application));
  
       /* Source gets filled in autonomously and is tested elsewhere */
  
  // Ensure ring id correctness.
  
  CStatusDefinitions::RingStatIdentification* pId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
  EQ(std::string("test_ring"), std::string(pId->s_ringName));
  
  
  // Done with the messages so:
  
  freeMessage(message);
  
  // Should be no more messages in the queue:
  
  zmq::message_t dummy;
  ASSERT(!(*m_pReceiver)->recv(&dummy, ZMQ_NOBLOCK));
  EQ(EAGAIN, errno);
  

}

// Make a ring and add ourselves as a producer.  Put some data in and
// now publication should give a RingStatClient for us.  We'll have to take
// the s_command at its word as we don't know the actual command the Makefile
// uses to run us.

void RingPubTests::ringWithProducer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer  producer("test_ring", CRingBuffer::producer);
  std::uint8_t buffer[100];   // Just some junk to write to the ringbuffer
  
  // 100 puts into the ring:
  
  for (int i = 0; i < 100; i++) {
    producer.put(buffer, sizeof(buffer));
  }
  
  // Publish and read the publication:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  // Should be three message segments and the last one should be a producer segment:
  
  EQ(size_t(3), message.size());
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  EQ(uint64_t(100), pClient->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pClient->s_bytes);
  ASSERT(pClient->s_isProducer);
  EQ(command, marshallVector(pClient->s_command));
  EQ(uint64_t(0), pClient->s_backlog);
  EQ(uint64_t(getpid()), pClient->s_pid); 
  
  freeMessage(message);
}

// Make a ring with a consumer (ourself) with only a  consumer no data
// transfer can occur.

void RingPubTests::ringWithConsumer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer consumer("test_ring");               // Consumer by default.
  
  (*m_pPublisher)();                               // publish.
  std::vector<zmq::message_t*> message = receiveMessage();
  
  EQ(size_t(3), message.size());
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  EQ(uint64_t(0), pClient->s_operations);
  EQ(uint64_t(0), pClient->s_bytes);
  ASSERT(!pClient->s_isProducer);
  EQ(uint64_t(0), pClient->s_backlog);
  EQ(uint64_t(getpid()), pClient->s_pid);
  EQ(command, marshallVector(pClient->s_command));
  
  freeMessage(message);
}
// Make a ring with both a producer and a consumer.  Send data from one to the
// other.  Publish statistics and ensure that we have the proper info for both
// producer and consumer processes.

void RingPubTests::ringWithProducerAndConsumer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer producer("test_ring", CRingBuffer::producer);
  CRingBuffer consumer("test_ring");
  
  // Exchange random bytes of data:
  
  std::uint8_t buffer[100];      // Contents don't matter we trust put/get.
  for (int i =0; i < 100; i++) {
    producer.put(buffer, sizeof(buffer));
    consumer.get(buffer, sizeof(buffer), sizeof(buffer)); // data's already there.
  }
  
  // Publish and read:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  
  EQ(size_t(4), message.size());
  
  // analzye the producer message (that's always first):
  
  CStatusDefinitions::RingStatClient* pProducer =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  ASSERT(pProducer->s_isProducer);  
  EQ(uint64_t(100), pProducer->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pProducer->s_bytes);
  EQ(uint64_t(0), pProducer->s_backlog);
  EQ(uint64_t(getpid()), pProducer->s_pid);
  EQ(command, marshallVector(pProducer->s_command));
  
  
  CStatusDefinitions::RingStatClient* pConsumer =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[3]->data());
  ASSERT(!pConsumer->s_isProducer);
  EQ(uint64_t(100), pConsumer->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pConsumer->s_bytes);
  EQ(uint64_t(getpid()), pProducer->s_pid);
  EQ(uint64_t(0), pProducer->s_backlog);
  EQ(command, marshallVector(pConsumer->s_command));
  
  freeMessage(message);
}
// Several consumers to one ring.  In this case, we have a bit ring and
// not all the consumers will look at all messages.
// Note - ring buffer usage information for consumers is in order of attachement
//        for the case shown.
//
void RingPubTests::ringWithProducerSeveralConsumers()
{
  CRingBuffer::create("test_ring");
  CRingBuffer producer("test_ring", CRingBuffer::producer);
  CRingBuffer cons1("test_ring");
  CRingBuffer cons2("test_ring");
  CRingBuffer cons3("test_ring");
  
  std::uint8_t buffer[100];
  
  // 102 is exactly divisible by three and event so there's no uncertainty
  // for the number of reads done by consumers;  Note that the way this is done,
  // the consumers that don't get all items will have backlogs:
  
  for(int i = 0; i < 102; i++) {
    producer.put(buffer, sizeof(buffer));
    cons1.get(buffer, sizeof(buffer), sizeof(buffer));
    
    // Cons 2 reads for every other put (leaves data in the buffer).
    
    if ((i%2) == 0) {
      cons2.get(buffer, sizeof(buffer), sizeof(buffer));
    }
    // Cons3 every 3'd put:
    
    if ((i%3) == 0) {
      cons3.get(buffer, sizeof(buffer), sizeof(buffer));
    }
  }
  // publish and read:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  // Number of segments is header + id + producer + 3* consumers = 6:
  
  EQ(size_t(6), message.size());
  
  // Already trust producer... let's look at the three consumers:
  
  CStatusDefinitions::RingStatClient* pCons1 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[3]->data());
  CStatusDefinitions::RingStatClient* pCons2 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[4]->data());
  CStatusDefinitions::RingStatClient* pCons3 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[5]->data());
  
  // Cons 1 sees all messages:
  
  EQ(std::uint64_t(102), pCons1->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)), pCons1->s_bytes);
  EQ(std::uint64_t(0), pCons1->s_backlog);             // so no backlog.
  
  // Cons 2 sees every other message:
  
  EQ(std::uint64_t(102/2), pCons2->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/2), pCons2->s_bytes);
  EQ(std::uint64_t(102*sizeof(buffer)/2), pCons2->s_backlog);   // 1/2 the data backlogged.
  
  // Cons 3 sees every  third message:
  
  EQ(std::uint64_t(102/3), pCons3->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/3), pCons3->s_bytes);
  EQ(std::uint64_t(102*2*sizeof(buffer)/3), pCons3->s_backlog);  // 2/3 the data backlogged.
  
  freeMessage(message);
}
// Multiple rings.. ringmaster sends data out in alpha order so we can
// predict the order for each message.

void RingPubTests::multipleRings()
{
  const char* ringNames[] = {"a", "b", "c", "d", 0};
  const char** p(ringNames);
  while (*p) {
    CRingBuffer::create(*p);
    p++;
  }
  
  (*m_pPublisher)();
  
  // Should be a message for each ring in the order of ringNames:
  
  p = ringNames;
  while(*p) {
    std::vector<zmq::message_t*> message  = receiveMessage();
    CStatusDefinitions::RingStatIdentification* pRing =
      reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
    EQ(std::string(*p), std::string(pRing->s_ringName));
    
    freeMessage(message);
    p++;
  }
  
}

//  If asking for a history index that's not found, .first will be false.

void RingPubTests::getHistoryIndexNotFound()
{
  // Create the usage entry:
  
  CPublishRingStatistics::Usage usage;
  usage.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/dumper", "--source=tcp://localhost/fox"});
  usage.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/eventlog", "--prefix=ccusb"});
  usage.s_usage.s_consumers.push_back({1234, 5000});                // dumper pid/backlog.
  usage.s_usage.s_consumers.push_back({666, 1234});                 // eventlog. pid/backlog
  
  // Create the history entry.
  
  CPublishRingStatistics::Usage history;
  history.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/dumper"});
  history.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/eventlog", "--prefix=vmusb"});
  history.s_usage.s_consumers.push_back({666,5000});
  history.s_usage.s_consumers.push_back({1234, 5000});
  
  //  Test:
  
  EQ(false, CPublishRingStatistics::getHistoryIndex(usage, history, 1).first);
}

// Asking for a history index that is found:

void RingPubTests::getHistoryIndexFound()
{
  // Create the usage entry:
  
  CPublishRingStatistics::Usage usage;
  usage.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/dumper", "--source=tcp://localhost/fox"});
  usage.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/eventlog", "--prefix=ccusb"});
  usage.s_usage.s_consumers.push_back({1234, 5000});                // dumper pid/backlog.
  usage.s_usage.s_consumers.push_back({666, 1234});                 // eventlog. pid/backlog
  
  // Create the history entry.
  
  CPublishRingStatistics::Usage history;
  history.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/eventlog", "--prefix=ccusb"});
  history.s_consumerCommands.push_back({"/usr/opt/daq/12.0/bin/dumper", "--source=tcp://localhost/fox"});
  history.s_usage.s_consumers.push_back({666,1234});
  history.s_usage.s_consumers.push_back({1234, 5000});  
   
  std::pair<bool, size_t> result = CPublishRingStatistics::getHistoryIndex(usage, history, 1);
  ASSERT(result.first);
  EQ(size_t(0), result.second);
}

// Create the right log message.

void RingPubTests::makeLogMessage()
{
  std::vector<std::string> command = {"/usr/opt/daq/current/bin/dumper", "--source=tcp://localhost/fox"};
  
  std::string result = CPublishRingStatistics::makeBacklogMessage("Backlog too big: ", command, 100, 75);
  EQ(std::string("Backlog too big:  Consumer command /usr/opt/daq/current/bin/dumper --source=tcp://localhost/fox  backlog is 75%"), result);
}

// If there's no history entries, then a ringbuffer that's over the threshold logs.
// Note that we're going to assume the threshold is 95% or higher.

void RingPubTests::logLargeNoPriorHistory()
{
    // Build a ring Usage struct with a single consumer client over the backlog.
    
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {false};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 95}};                  // One consumer with 95 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    // Ask if index 0 needs logging -- it should.
    
    ASSERT(m_pPublisher->logLargeBacklog(u, 0));
}
// If there are history entries for the ring but none for the consumer,
// we also need to emit if over threshold.

void RingPubTests::logLargeNoPriorConsumer()
{
  // Build the ring usage with a single consumer:
  
      CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {false};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 95}};                  // One consumer with 95 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    // Cheat to enter it int he map with a different PID:
    
    pid_t oldPid            = ru.s_consumers[0].first;
    ru.s_consumers[0].first = 6666;
    ru.s_consumerStats[0].s_pid = 6666;
    m_pPublisher->m_history[u.s_ringName] = u;
  
    ru.s_consumers[0].first=  oldPid;
    ru.s_consumerStats[0].s_pid = oldPid;
    
    ASSERT(m_pPublisher->logLargeBacklog(u, 0));
}
/// There's a history entry that shows this consumer has not yet logged:

void RingPubTests::logLargeLog()
{
   
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {false};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 95}};                  // One consumer with 95 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    
    m_pPublisher->m_history[u.s_ringName] = u;
    
    ASSERT(m_pPublisher->logLargeBacklog(u, 0));
}

// 'Worst case' for no log is that there's a history entry. with a false
// flag.

void RingPubTests::logLargeNoLog()
{
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {false};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 85}};                  // One consumer with 85 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    
    m_pPublisher->m_history[u.s_ringName] = u;
    
    ASSERT(!m_pPublisher->logLargeBacklog(u, 0));
  
}
// History says there was a prior log but now we're below the thresold.
// shouldl og ok:

void RingPubTests::logOkLog()
{
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {true};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 75}};                  // One consumer with 75 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    m_pPublisher->m_history[u.s_ringName] = u;                          // History said we logged:
    
    ASSERT(m_pPublisher->logBacklogOk(u, 0));             // Threshold is now good.
}
// If the backlog is larger than the threshold no logging:

void RingPubTests::logOkNolog()
{
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {true};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 85}};                  // One consumer with 85 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    m_pPublisher->m_history[u.s_ringName] = u;                          // History said we logged:
    ASSERT(!m_pPublisher->logBacklogOk(u, 0));
}
// If we're below threshold and no client is visible, we don't report:

void RingPubTests::logOkNologNoClient()
{
    CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {true};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 75}};                  // One consumer with 85 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    // Fake up a different client:
    
    pid_t originalPid = ru.s_consumers[0].first;
    ru.s_consumers[0].first = 666;
    ru.s_consumerStats[0].s_pid = 666;
    
    m_pPublisher->m_history[u.s_ringName] = u;
    ru.s_consumers[0].first = originalPid;
    ru.s_consumerStats[0].s_pid = originalPid;
    
    
    ASSERT(!m_pPublisher->logBacklogOk(u, 0));
    
    
}
// If there are no history entries for the ring, don't log ok:

void RingPubTests::logOkNologNohistory()
{
  CPublishRingStatistics::Usage u;
    u.s_ringName         = "SomeRing";
    u.s_consumerCommands = {{"/usr/opt/daq/current/bin/dumper"}};
    u.s_logged           = {true};                         // Not really needed.
    CRingBuffer::Usage& ru(u.s_usage);                    // Need to fill in the rb usage:
    ru.s_bufferSpace     = 100;                           // easy to do pcts.
    ru.s_consumers       = {{1234, 75}};                  // One consumer with 85 pct backlog.
    ru.s_consumerStats   = {{1234, 1000, 10000}};         // Don't actually think I need this but...
    
    ASSERT(!m_pPublisher->logBacklogOk(u, 0));
    
}
