// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <sys/wait.h>
#include <CDataSink.h>
#include <CDataSinkFactory.h>
#include <CRingBuffer.h>
#include <time.h>
#include <pwd.h>
#include <is_byte_iterator.h>

#include <vector>

#include <V12/DataFormat.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingItemParser.h>
#include <RingIOV12.h>
#include <ContainerDeserializer.h>
#include <ByteBuffer.h>
#include <stdlib.h>
#include <os.h>
#include <io.h>
#include <assert.h>

using namespace std;
using namespace DAQ;
using namespace DAQ::V12;

pid_t childpid;


template<class ByteIter,
         typename = typename std::enable_if< DAQ::is_byte_iterator<ByteIter>::value >::type >
void dumpBinary(ByteIter beg, ByteIter end)
{
    std::cout << "begin dump " << std::endl;
    size_t size = std::distance(beg, end);
    std::cout << "size=" << size << std::endl;
    std::cout << std::hex << std::setfill('0');
    for (auto iter=beg; iter<end; iter+=2) {
        uint16_t value = *(iter+1);
        value = ((value << 8) | (*iter));
        std::cout << std::setw(4) << value << std::endl;
    }
    std::cout << std::setfill(' ') << std::dec;
    std::cout << "end dump " << std::endl;
}

static int 
readItem(int fd, void* pBuffer)
{
  auto p = reinterpret_cast<unsigned char*>(pBuffer);
  auto pBeg = p;

  size_t bytes = ::io::readData(fd, p, 20);

  EQMSG("bytes read", size_t(20), bytes);
  
  uint32_t size, type, sourceId;
  uint64_t tstamp; 
  bool swapNeeded;
 
  Parser::parseHeader(p, p+bytes, size, type, tstamp, sourceId, swapNeeded);
  p += bytes;

  size_t remaining = size - bytes;
  bytes = read(fd, p, remaining);

  EQMSG("remaining bytes", remaining, bytes);

  return size;
}


// Static utility program that will run a specified command
// with stdout and stderr -> a pipe.  The read end of the pipe is returned.
//
static int spawn(const char* command) 
{
  int pipes[2];
  if (pipe(pipes) == -1) {
    perror("Could not open pipes");
    throw "could not open pipes";
  }

  // I can never keep these straight so:

  int readPipe = pipes[0];
  int writePipe= pipes[1];

  if ((childpid = fork()) != 0) {
    // parent...will read from the pipe:

    sleep(1);			// Let child start.
    close(writePipe);
    return readPipe;

  }
  else {
    // child...
    setsid();

    close(readPipe);


    // Set the write pipe as our stdout/stderr:

    dup2(writePipe, STDOUT_FILENO);
    close(writePipe);

    int status = system(command);
    assert(status != -1);
    
    close(STDOUT_FILENO);

    exit(0);

  }
  
}

static void textItem(DAQ::CDataSink& prod, int fd, bool check = true)
{
  vector<string>  items;
  items.push_back("String 1");
  items.push_back("String 2");
  items.push_back("The last string");

  CRingTextItem i(PACKET_TYPES, items);
  writeItem(prod, CRawRingItem(i));

  if (check) {
    char buffer[2048];
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);
    EQMSG("packet types type", PACKET_TYPES, result.first->type());

    CRingTextItem& item = dynamic_cast<CRingTextItem&>(*result.first);
    EQMSG("string count", (uint32_t)3,  item.getStringCount());

    auto strings = item.getStrings();

    EQ(string("String 1"), strings[0]);
    EQ(string("String 2"), strings[1]);
    EQ(string("The last string"), strings[2]);
  }
}

static void scaler(DAQ::CDataSink& prod, int fd, bool check=true)
{
  vector<uint32_t> scalers(32);
  std::iota(scalers.begin(), scalers.end(), 0);;

  CRingScalerItem i(0, 10, time_t(NULL), scalers);
  writeItem(prod, CRawRingItem(i));

  if (check) {
    char buffer[1024];
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);

    CRingScalerItem& item = dynamic_cast<CRingScalerItem&>(*result.first);
    EQMSG("scaler type", PERIODIC_SCALERS, item.type());
    EQMSG("start time", (uint32_t)0,         item.getStartTime());
    EQMSG("end time", (uint32_t)10,        item.getEndTime());
    EQMSG("scaler count", (uint32_t)32,        item.getScalerCount());
    auto sclrs = item.getScalers();
    for (uint32_t i =0; i < 32; i++) {
      EQMSG("ith scaler value", i, sclrs[i]);
    }
  }
}

static void eventCount(DAQ::CDataSink& prod, int fd, int count, bool check=true)
{
  CRingPhysicsEventCountItem i(count, 12);
  writeItem(prod, i);

  if (check) {
    char buffer[1024];
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);

    auto& item = dynamic_cast<CRingPhysicsEventCountItem&>(*result.first);

    EQMSG("physics event count type", PHYSICS_EVENT_COUNT, item.type());
    EQMSG("time offset", (uint32_t)12, item.getTimeOffset());
    EQMSG("event count", (uint64_t)count, item.getEventCount());
  }
}


static void event(DAQ::CDataSink& prod, int fd, bool check=true)
{
  CPhysicsEventItem i;
  auto& body = i.getBody();
  body << uint16_t(11);
  for (int i =0; i < 10; i++) {
    body << uint16_t(i);
  }
  writeItem(prod, i);

  if (check) {
    unsigned char buffer[1024];
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer + size);
    auto& item = dynamic_cast<CPhysicsEventItem&>(*result.first);

    EQMSG("physics event type", PHYSICS_EVENT, item.type());

    auto stream = Buffer::makeContainerDeserializer(item.getBody(), false);
    uint16_t temp;
    stream >> temp;
    EQMSG("body size", (uint16_t)11, temp);
    for (int i = 0; i < 10; i++) {
      stream >> temp;
      EQMSG("body element", (uint16_t)i, temp);
    }
  }


}


static void beginRun(DAQ::CDataSink& prod, int fd,  bool check = true)
{
    time_t now = time(NULL);
  CRingStateChangeItem i(BEGIN_RUN, 1234, 0, now, "This is a title");
  writeItem(prod, i);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  unsigned char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:

    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);
    auto& item = dynamic_cast<CRingStateChangeItem&>(*result.first);

    EQMSG("begin type", BEGIN_RUN, item.type());
    EQMSG("begin run #", (uint32_t)1234,      item.getRunNumber());
    EQMSG("begin elapsed time", (uint32_t)0,         item.getElapsedTime());
    EQMSG("begin title", string("This is a title"), item.getTitle());
  }
}

static void pauseRun(DAQ::CDataSink& prod, int fd, bool check=true)
{
  CRingStateChangeItem i(PAUSE_RUN, 1234, 15, time(NULL), "This is a title");
  writeItem(prod, i);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);

    auto& item = dynamic_cast<CRingStateChangeItem&>(*result.first);

    EQMSG("pause run type", PAUSE_RUN, item.type());
    EQMSG("pause run number", (uint32_t)1234,      item.getRunNumber());
    EQMSG("pause elapsed time", (uint32_t)15,         item.getElapsedTime());
    EQMSG("pause title", string("This is a title"), item.getTitle());
  }
}

static void resumeRun(DAQ::CDataSink& prod, int fd, bool check = true)
{
  CRingStateChangeItem i(RESUME_RUN, 1234, 15, time(NULL), "This is a title");
  writeItem(prod, i);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);

    auto& item = dynamic_cast<CRingStateChangeItem&>(*result.first);
    EQMSG("resume run type", RESUME_RUN, item.type());
    EQMSG("resume run #", (uint32_t)1234,      item.getRunNumber());
    EQMSG("resume elapsed time", (uint32_t)15,         item.getElapsedTime());
    EQMSG("resume title", string("This is a title"), item.getTitle());
  }
}


static void endRun(DAQ::CDataSink& prod, int fd, bool check = true)
{
  CRingStateChangeItem i(END_RUN, 1234, 25, time(NULL), "This is a title");
  writeItem(prod, i);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    int size = readItem(fd, buffer);
    auto result = Parser::parse(buffer, buffer+size);

    auto& item = dynamic_cast<CRingStateChangeItem&>(*result.first);
    
    EQMSG("type is END_RUN", END_RUN, item.type());
    EQMSG("end run #", (uint32_t)1234,       item.getRunNumber());
    EQMSG("end elapsed time", (uint32_t)25,         item.getElapsedTime());
    EQMSG("end title", string("This is a title"), item.getTitle());
  }
}

class rseltests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rseltests);
  CPPUNIT_TEST(all);
  CPPUNIT_TEST(exclude);
  CPPUNIT_TEST(only);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    if (!CRingBuffer::isRing(Os::whoami())) {
      CRingBuffer::create(Os::whoami());
    }
  }
  void tearDown() {
  }
protected:
  void all();
  void exclude();
  void only();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rseltests);


// Starts up the ring selector with no selectivity and
// ensures we can send data to that ring and get it back
// on the pipe connecting us to the child process.
// The preprocessor symobl BINDIR isthe directory in which
// the ringselector was installed.
//
void rseltests::all() 
{
  
  string programName = BINDIR;
  programName       += "/ringselector";

  int fd = spawn(programName.c_str());

  
  DAQ::CDataSink* pProd = nullptr;
  try {
    // attach to our ring as a producer.
    
    pProd = DAQ::CDataSinkFactory().makeSink(std::string("tcp://localhost/")+Os::whoami());
    
    // Make a begin_run item, commit it.
    bool check = true;
    beginRun(*pProd, fd, check);
    beginRun(*pProd, fd, check);
    for (int i =0; i < 100; i++) {
      event(*pProd,fd, check);
    }
    eventCount(*pProd, fd, 100, check);
    scaler(*pProd, fd, check);
    pauseRun(*pProd, fd, check);
    resumeRun(*pProd, fd, check);
    textItem(*pProd, fd, check);
    endRun(*pProd, fd, check);
    
    delete pProd;

  }    
  catch (...) {
    delete pProd;
    kill(childpid*-1, SIGTERM);
    
    int status;
    wait(&status);
    throw;
  }

  // Cleanup by killing the child.
  
  kill(childpid*-1, SIGTERM);
  
  int status;
  wait(&status);
  close(fd);

}
// build use the --exclude switch to not accept BEGIN_RUN items.

void rseltests::exclude()
{
  string programName = BINDIR;
  programName       += "/ringselector --exclude=BEGIN_RUN";
  int fd             = spawn(programName.c_str());

  DAQ::CDataSink* pProd = nullptr;
  try {
    
    pProd = DAQ::CDataSinkFactory().makeSink(std::string("tcp://localhost/")+Os::whoami());

    beginRun(*pProd, fd, false);
    pauseRun(*pProd, fd);
    endRun(*pProd,fd);		// Should be the first one back from the program.

    delete pProd;
  }
  catch (...) {

    kill (childpid*-1, SIGTERM);
    int s;
    wait(&s);
    throw;
  }

  kill (childpid*-1, SIGTERM);
  int s;
  wait(&s);
  close(fd);
}

// Build using the --accept switch...
void rseltests::only()
{
  string programName = BINDIR;
  programName       += "/ringselector --accept=BEGIN_RUN"; // only begin runs.
  int  fd            = spawn(programName.c_str());
 
  DAQ::CDataSink* pProd = nullptr;
  try {
    pProd = DAQ::CDataSinkFactory().makeSink(std::string("tcp://localhost/")+Os::whoami());

    beginRun(*pProd,fd);		// Should be fine.
    eventCount(*pProd, fd, 100, false);
    scaler(*pProd, fd, false);
    pauseRun(*pProd, fd, false);
    resumeRun(*pProd, fd, false);
    beginRun(*pProd,fd);

    delete pProd;
  }
  catch (...) {
    delete pProd;

    kill (childpid*-1, SIGTERM);
    int s;
    wait(&s);
    throw;
  }

  kill (childpid*-1, SIGTERM);
  int s;
  wait(&s);
  close(fd);
}
// don't know how to test for sampling.

