// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// The def/undefs let us look into the object's guts for white box testing.


#define private public
#define protected public
#include "CTclRingCommand.h"
#undef private
#undef protected

#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CRingBuffer.h>
#include <CRingDataSink.h>
#include <V12/DataFormat.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CCompositeRingItem.h>
#include <RingIOV12.h>


#include <iostream>

using namespace DAQ;

class RingTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingTests);
  CPPUNIT_TEST(construct);
  
  // Tests for top level command processing
  
  CPPUNIT_TEST(needsubcommand);
  CPPUNIT_TEST(badsubcommand);
  
  // Tests for the attach command
  
  CPPUNIT_TEST(needURI);
  CPPUNIT_TEST(needvalidURI);
  CPPUNIT_TEST(validRing);
  CPPUNIT_TEST(alreadyAttached);
  
  // Tests for the detach command:
  
  CPPUNIT_TEST(detachNeedURI);
  CPPUNIT_TEST(detachNeedAttachedURI);
  CPPUNIT_TEST(detachOk);
  
  // Tests for the get command:
  
  CPPUNIT_TEST(getNeedUri);
  CPPUNIT_TEST(getNeedAttachedUri);
  CPPUNIT_TEST(getBodyHeaderBegin);
  CPPUNIT_TEST(getBodyHeaderScaler);
  CPPUNIT_TEST(getBodyHeaderPacketTypes);
  CPPUNIT_TEST(getRingFormat);
  CPPUNIT_TEST(getBodyHeaderPhysics);
  CPPUNIT_TEST(getPhysicsEventCountBodyHeader);
  CPPUNIT_TEST(getGlomInfo);
  CPPUNIT_TEST(getWithPredicate);
  CPPUNIT_TEST(getComposite);
  CPPUNIT_TEST(getTimeout_0);
  CPPUNIT_TEST(getTimeout_1);

  // Test for Abnormal End.
  
  CPPUNIT_TEST(getAbnormalEnd);

  
  CPPUNIT_TEST_SUITE_END();


private:
    CTCLInterpreter* m_pInterp;
    V12::CTclRingCommand* m_pCommand;
public:
  void setUp() {
    m_pInterp   = new CTCLInterpreter();
    m_pCommand  = new V12::CTclRingCommand(*m_pInterp);
    try {CRingBuffer::create("tcltestring");} catch(...) {}
  }
  void tearDown() {
    delete m_pCommand;
    delete m_pInterp;
    try {CRingBuffer::remove("tcltestring");} catch(...) {}
    unlink("/dev/shm/tcltestring");    // In case it lingered.
  }
protected:
  void construct();
  void needsubcommand();
  void badsubcommand();
  
  void needURI();
  void needvalidURI();
  void validRing();
  void alreadyAttached();
  
  void detachNeedURI();
  void detachNeedAttachedURI();
  void detachOk();
  
  void getNeedUri();
  void getNeedAttachedUri();
  void getBodyHeaderBegin();
  void getBodyHeaderScaler();
  void getBodyHeaderPacketTypes();
  void getBodyHeaderVars();
  void getRingFormat();
  void getBodyHeaderPhysics();
  void getPhysicsEventCountBodyHeader();
  void getComposite();
  void getGlomInfo();
  void getWithPredicate();
  
  void getAbnormalEnd();
  void getTimeout_0();
  void getTimeout_1();


private:
    int tryCommand(const char* command);
    std::string getResult();
    void insertStateChange(int type);
    void insertScalerItem();
    void emitStringList(int type);
    void emitFormat();
    void emitEvent();
    void emitEventCount();
    void emitGlomParams();
    void emitComposite();
    
    int getDictItem(Tcl_Obj* obj, const char* key, std::string& value);
    
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingTests);


/*--------------------------------- Utility factorizations ---------------------------*/

// Factor out the stuff needed to run a command:
// returns the status.

int
RingTests::tryCommand(const char* cmd)
{
    Tcl_Interp* pInterp = m_pInterp->getInterpreter();
    return Tcl_GlobalEval(pInterp, cmd);
}
// Get interpreter result:

std::string
RingTests::getResult()
{
    Tcl_Interp* pInterp = m_pInterp->getInterpreter();
    return std::string(Tcl_GetStringResult(pInterp));
}

/**
 * get an item by key from an object that has a dict:
 *  @param obj  - the dict.
 *  @param key  - the key to retrieve.
 *  @param value - The value of the key (meaningful only if TCL_OK is returned)
 *  @return Resulst of Tcl_DictObjGet()
 */
int RingTests::getDictItem(Tcl_Obj* obj, const char* key, std::string& value)
{
    Tcl_Interp* pI = m_pInterp->getInterpreter();
    Tcl_Obj*    keyObj = Tcl_NewStringObj(key, -1);
    Tcl_Obj*    valObj;
    
    int status = Tcl_DictObjGet(pI, obj, keyObj, &valObj);
    if ((status == TCL_OK) && valObj) {
        value = Tcl_GetString(valObj);
    } else {
        status = TCL_ERROR;            // In case valObj was zero.
    }
    return status;
}
/**
 * insertStateChange - insert a state change item in the tcltestring
 * @param type - Actual state change type (e.g. BEGIN_RUN).
 * @param bheader - true if a body header is to be constructed.
 */
void RingTests::insertStateChange(int type)
{
    CRingDataSink ring("tcltestring");
    V12::CRingStateChangeItem item(12345678, 1, type, 123, 0, 0, std::string("A test title"));
    writeItem(ring, item);
}
/**
 * insertScalerItem - insert a scaler item in the tcltestring
 * @param bheader - true if the item must have a body header.
 */
void
RingTests::insertScalerItem()
{
    CRingDataSink ring("tcltestring");
    std::vector<uint32_t> scalers;
    for (int i =0; i < 10; i++) {
        scalers.push_back(i);
    }
    V12::CRingScalerItem item(12345678, 1, 0, 10, 0, scalers, 2, 8);
    writeItem(ring, item);

}
void
RingTests::emitStringList(int type)
{
    std::vector<std::string> strings;
    strings.push_back("Type 1");
    strings.push_back("Type 2");
    strings.push_back("Type 3");
    CRingDataSink ring("tcltestring");
    V12::CRingTextItem item(type, 1234, 1, strings, 20, 1111, 1);
    writeItem(ring, item);
}

void
RingTests::emitFormat()
{
    CRingDataSink ring("tcltestring");
    V12::CDataFormatItem item;
    writeItem(ring, item);
}
void
RingTests::emitEvent()
{
    CRingDataSink ring("tcltestring");
    Buffer::ByteBuffer body;
    body << uint32_t(12);
    for (uint16_t i = 0; i < 10; i++) {
        body << i;
    }
    V12::CPhysicsEventItem item(1234, 2, body);
    writeItem(ring, item);
}


void RingTests::emitEventCount() {
    CRingDataSink ring("tcltestring");
    V12::CRingPhysicsEventCountItem item(1234, 2, 1000, 123, 0);
    writeItem(ring, item);

}
void RingTests::emitGlomParams()
{
    CRingDataSink ring("tcltestring");
    V12::CGlomParameters item(12345, 11, 10, true, V12::CGlomParameters::average);
    writeItem(ring, item);
}

void RingTests::emitComposite()
{
    CRingDataSink ring("tcltestring");
    V12::CCompositeRingItem item(V12::COMP_BEGIN_RUN, V12::NULL_TIMESTAMP, 3);
    item.appendChild( std::make_shared<V12::CRingStateChangeItem>(12345678,
                                                                  1,
                                                                  V12::BEGIN_RUN,
                                                                  123,
                                                                  0,
                                                                  0,
                                                                  "A test title"));

    item.appendChild( std::make_shared<V12::CRingStateChangeItem>(12345679,
                                                                  2,
                                                                  V12::BEGIN_RUN,
                                                                  123,
                                                                  2,
                                                                  2,
                                                                  "A test title"));
    writeItem(ring, item);
}

/*---------------------------------------------- TESTS ------------------------------------*/

// Construction should have created a 'ring' command.

void RingTests::construct() {
    Tcl_CmdInfo info;
    
    ASSERT(0 != Tcl_GetCommandInfo(m_pInterp->getInterpreter(), "ring", &info));
}
// Just the 'ring' command by itself returns an error because it needs a
// subcommand

void RingTests::needsubcommand() {
    int status = tryCommand("ring");
    EQ(TCL_ERROR, status);
    EQ(std::string("Insufficient parameters"), getResult() );
    
}

// invalid subcommand is an error too:

void RingTests::badsubcommand() {
    int status = tryCommand( "ring george");
    EQ(TCL_ERROR, status);
    EQ(std::string("bad subcommand"), getResult());
    
}

// The ring attach command should complain if there's no ring URI.

void RingTests::needURI() {
    
    int status = tryCommand( "ring attach");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring attach needs a ring URI"), getResult());
}
// The ring attach command should complain if there is no such ring:

void RingTests::needvalidURI() {
    try {
        CRingBuffer::remove("no-such-ring");
    } catch(...) {}                             /// Catch in case the ring never existed (likely).
    
    
    int status = tryCommand( "ring attach tcp://localhost/no-such-ring");
    EQ(TCL_ERROR, status);
    EQ(std::string("Failed to attach ring"), getResult());
}    

void RingTests::validRing() {
    
    int status = tryCommand( "ring attach tcp://localhost/tcltestring"); // See setUp().
    EQ(TCL_OK, status);
    
    // The map in m_pCommand should have this too:
    
    ASSERT(m_pCommand->m_attachedRings.end() !=
           m_pCommand->m_attachedRings.find("tcp://localhost/tcltestring"));
}

// An error to doubly attach a URI.

void RingTests::alreadyAttached() {
    
    
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    status     = tryCommand("ring attach tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring already attached"), getResult());
    
}


void RingTests::detachNeedURI() {
    
    int status = tryCommand("ring detach");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring detach needs a URI"), getResult());
}
void RingTests::detachNeedAttachedURI() {
    
    int status = tryCommand( "ring detach tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring is not attached"), getResult());
    
}
void RingTests::detachOk() {
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    status     = tryCommand("ring detach tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    ASSERT(m_pCommand->m_attachedRings.end() ==
           m_pCommand->m_attachedRings.find("tcp://localhost/tcltestring"));
    
}


  
void RingTests::getNeedUri(){
    
    int status = tryCommand("ring get ");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring get needs a URI"), getResult());
    
}
void RingTests::getNeedAttachedUri() {
    int status = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring is not attached"), getResult());
}

// Attach, insert, then get should work to make us get a ring
// item.

void RingTests::getBodyHeaderBegin() {
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertStateChange(V12::BEGIN_RUN);
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    
    /* We expect the result to be a dict with:
       type       - ring item type (textual)
       run        - run number
       timeoffset - Time offset in seconds.
       realtime   - Time of day of the ring item (can use with [time format])
       title      - The run title.
   */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    status = getDictItem(result, "type", item);
    EQMSG("state change items have type entry", TCL_OK, status);
    EQ(std::string("Begin Run"), item);
    
    status = getDictItem(result, "run", item);
    EQMSG("state change items have run entry", TCL_OK, status);
    EQ(std::string("123"), item);
    
    status = getDictItem(result, "timeoffset", item);
    EQMSG("state change items have timeoffset entry", TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "realtime", item);
    EQMSG("state change items have realtime entry", TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "title", item);
    EQMSG("state change items have title entry", TCL_OK, status);
    EQ(std::string("A test title"), item);
    
    status = getDictItem(result, "timestamp", item);
    EQMSG("state change items have timestamp entry", TCL_OK, status);
    EQ(std::string("12345678"), item);
    
    status = getDictItem(result, "source", item);
    EQMSG("state change items have source entry", TCL_OK, status);
    EQ(std::string("1"), item);

}


void RingTests::getBodyHeaderScaler() {

    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertScalerItem();
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    // We expect a dict containing the following:
    //  - no bodyheader key.
    //  - type - going to be Scaler
    //  - start - When in the run the start was.
    //  - end   - When in the run the end of the period was.
    //  - realtime  Time emitted ([clock format] can take this)
    //  - divisor - What to divide start or end by to get seconds.
    //  - incremental - Bool true if this is incremental.
    //  - scalers     - List of scaler values.
    
    std::string item;
    status = getDictItem(result, "type", item);
    EQMSG("scaler has type entry", TCL_OK, status);
    EQ(std::string("Scaler"), item);
    
    status = getDictItem(result, "start", item);
    EQMSG("scaler has start entry", TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "end", item);
    EQMSG("scaler has end entry", TCL_OK, status);
    EQ(std::string("10"), item);
    
    status = getDictItem(result, "divisor", item);
    EQMSG("scaler has divisor entry", TCL_OK, status);
    EQ(std::string("2"), item);
    
    status = getDictItem(result, "incremental", item);
    EQMSG("scaler has incremental entry", TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(result, "realtime", item);
    EQMSG("scaler has realtime entry", TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "scalerwidth", item);
    EQMSG("scaler has scalerwidth entry", TCL_OK, status);
    EQ(std::string("32"), item);

    Tcl_Obj* scalers;
    Tcl_Obj* key = Tcl_NewStringObj("scalers", -1);
    status = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &scalers);
    EQMSG("scaler has scalers entry", TCL_OK, status);
    ASSERT(scalers);
    
    // This must be a 10 element list with a counting pattern:
    
    CTCLObject scls(scalers);
    scls.Bind(m_pInterp);
    EQ(10, scls.llength());
    for(int i = 0; i < 10; i++) {
        EQ(i, int(scls.lindex(i)));
    }
    // Now there should be a body:
    
   /*
     * There should be a bodyheader which itself is a dict containing:
     * timestamp - The timstamp field.
     * source    - Id of the source.
     * barrier   - Barrier type
     */
    status = getDictItem(result, "timestamp", item);
    EQ(TCL_OK, status);
    EQ(std::string("12345678"), item);
    
    status = getDictItem(result, "source", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    
}

void RingTests::getBodyHeaderPacketTypes(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitStringList(V12::PACKET_TYPES);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* pResult = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    
    /* pResult should have a dict and:
    *  -  There will be a bodyheader key.
    *  -  type will be the item type e.g. 
    *  -  timeoffset will have the offset time.
    *  -  divisor will have the time divisor to get seconds.
    *  -  realtime will have something that can be given to [clock format] to get
    *     when this was emitted
    *  -  strings - list of strings that are in the ring item.
    */
    
    std::string item;
    
    stat  = getDictItem(pResult, "type", item);
    EQMSG("text types have type entry", TCL_OK, stat);
    EQ(item, std::string("Packet types"));
    
    stat = getDictItem(pResult, "timeoffset", item);
    EQMSG("text types have timeoffset entry", TCL_OK, stat);
    EQ(item, std::string("20"));
    
    stat = getDictItem(pResult, "divisor", item);
    EQMSG("text types have divisor entry", TCL_OK, stat);
    EQ(item, std::string("1"));
    
    stat = getDictItem(pResult, "realtime", item);
    EQMSG("text types have realtime entry", TCL_OK, stat);
    EQ(item, std::string("1111"));
    
    Tcl_Obj* strings;
    Tcl_Obj* key = Tcl_NewStringObj("strings", -1);
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), pResult, key, &strings);
    
    CTCLObject stringList(strings);
    stringList.Bind(m_pInterp);
    EQ(3, stringList.llength());
    EQ(std::string("Type 1"), std::string(stringList.lindex(0)));
    EQ(std::string("Type 2"), std::string(stringList.lindex(1)));
    EQ(std::string("Type 3"), std::string(stringList.lindex(2)));
    
    stat = getDictItem(pResult, "timestamp", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1234"));

    stat = getDictItem(pResult, "source", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1"));
}

void RingTests::getRingFormat(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitFormat();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /*
     * Format items have:
     *    - no body header
     *    - type ("Ring item format version")
     *    - major - Major version number.
     *    - minor - Minor version number.
     */
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;
    
    EQMSG("data format has type entry", TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Data Format"), item);
    
    EQMSG("data format has major entry", TCL_OK, getDictItem(result, "major", item));
    EQ(std::string("12"), item);
    
    EQMSG("data format has minor entry", TCL_OK, getDictItem(result, "minor", item));
    EQ(std::string("0"), item);
    

    EQMSG("data format has timestamp entry", TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("NULL_TIMESTAMP"), item);

    EQMSG("data format has source entry", TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("0"), item);

}

void RingTests::getBodyHeaderPhysics(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEvent();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /* This dict contains:
     *    - type - "Event"
     *    - size - Number of bytes in the event.
     *    - body - The event data as a bytearray (Use [binary scan] to pick it apart)
     *    - There shouild be no bodyheader.
     */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    EQMSG("event types have type entry", TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Event"), item);
    
    EQMSG("event types have size entry", TCL_OK, getDictItem(result, "size", item));
    EQ(std::string("24"), item);
    
    Tcl_Obj*    key = Tcl_NewStringObj("body", -1);
    Tcl_Obj*    byteArray;
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &byteArray);
    EQMSG("event types have body entry", TCL_OK, stat);
    ASSERT(byteArray);              // Null if no such key.
    
    int size;
    struct _eventBody {
        uint32_t size;
        uint16_t body[10];
    } *event = reinterpret_cast<struct _eventBody*>(Tcl_GetByteArrayFromObj(byteArray, &size));
    EQ(24, size);
    EQ(static_cast<int>(24/sizeof(uint16_t)), static_cast<int>(event->size));
    for (int i =0; i < 10; i++) {
        EQ(i, static_cast<int>(event->body[i]));
    }
    
    EQMSG("event types have timestamp entry", TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("1234"), item);

    EQMSG("event types have source entry", TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("2"), item);

}


void RingTests::getPhysicsEventCountBodyHeader() {
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEventCount();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;

    /* result should be a dict with:
     * *   type : "Trigger count"
     * *    bodyheader item.
     * *   timeoffset - 123 (offset into the run).
     * *   divisor    - 1   divisor needed to turn that to seconds.
     * *   triggers   - 1000 Number of triggers.
     * *   realtime   - 0 time of day emitted.
     */
    
    EQMSG("event count types have type entry", TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Trigger count"), item);
    
    
    EQMSG("event count types have timeoffset entry", TCL_OK, getDictItem(result, "timeoffset", item));
    EQ(std::string("123"), item);
    
    EQMSG("event count types have divisor entry", TCL_OK, getDictItem(result, "divisor", item));
    EQ(std::string("1"), item);
    
    EQMSG("event count types have triggers entry", TCL_OK, getDictItem(result, "triggers", item));
    EQ(std::string("1000"), item);
    
    EQMSG("event count types have realtime entry", TCL_OK, getDictItem(result, "realtime", item));
    EQ(std::string("0"), item);

    EQMSG("event count types have timestamp entry", TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("1234"), item);

    EQMSG("event count types have source entry", TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("2"), item);

}
void RingTests::getGlomInfo(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitGlomParams();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    /*  We should have:
     *   * type  "Glom Parameters"
     *   * Never a body header.
     *   * isBuilding  1
     *   * timestampPolicy "average"
     *   * coincidenceWindow 10
     *
     */
    
    EQMSG("glom params type have type entry", TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Glom Parameters"), item);
        
    EQMSG("glom params type have isBuilding entry",
          TCL_OK, getDictItem(result, "isBuilding", item));
    EQ(std::string("1"), item);
    
    EQMSG("glom params type have timestampPolicy entry",
          TCL_OK, getDictItem(result, "timestampPolicy", item));
    EQ(std::string("average"), item);
    
    EQMSG("glom params type have coincidenceWindow entry",
          TCL_OK, getDictItem(result, "coincidenceWindow", item));
    EQ(std::string("10"), item);

    EQMSG("glom params type have timestamp entry",
          TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("12345"), item);

    EQMSG("glom params type have source entry",
          TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("11"), item);

}

void RingTests::getComposite(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitComposite();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);

    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());

    /* We expect the result to be a dict with:
       type       - Composite Begin
       timestamp  - evt timestamps
       sourceid   - source id
       children   - list of two begin types
   */

    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Composite Begin Run"), item);

    EQ(TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("NULL_TIMESTAMP"), item);

    EQ(TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("3"), item);

    EQMSG("composite item type has children entry",
          TCL_OK, getDictItem(result, "children", item));

    Tcl_Obj* pChildren = Tcl_NewStringObj(item.c_str(), -1);

    int length;
    stat = Tcl_ListObjLength(m_pInterp->getInterpreter(),
                          pChildren, &length);
    EQMSG("There should be 2 children", 2, length);

    Tcl_Obj* dict = Tcl_NewStringObj("", -1);

    EQMSG("get first child",
          TCL_OK, Tcl_ListObjIndex(m_pInterp->getInterpreter(), pChildren, 0, &dict));

    int status = getDictItem(dict, "type", item);
    EQMSG("child 1 type", TCL_OK, status);
    EQ(std::string("Begin Run"), item);

    status = getDictItem(dict, "run", item);
    EQMSG("child 1 run", TCL_OK, status);
    EQ(std::string("123"), item);

    status = getDictItem(dict, "timeoffset", item);
    EQMSG("child 1 timeoffset", TCL_OK, status);
    EQ(std::string("0"), item);

    status = getDictItem(dict, "realtime", item);
    EQMSG("child 1 realtime", TCL_OK, status);
    EQ(std::string("0"), item);

    status = getDictItem(dict, "title", item);
    EQMSG("child 1 title", TCL_OK, status);
    EQ(std::string("A test title"), item);

    status = getDictItem(dict, "timestamp", item);
    EQMSG("child 1 timestamp", TCL_OK, status);
    EQ(std::string("12345678"), item);

    status = getDictItem(dict, "source", item);
    EQMSG("child 1 source", TCL_OK, status);
    EQMSG("child 1 source value", std::string("1"), item);


    EQMSG("get child 2",
          TCL_OK, Tcl_ListObjIndex(m_pInterp->getInterpreter(), pChildren, 1, &dict));

    status = getDictItem(dict, "type", item);
    EQMSG("child 2 type", TCL_OK, status);
    EQ(std::string("Begin Run"), item);

    status = getDictItem(dict, "run", item);
    EQMSG("child 2 run", TCL_OK, status);
    EQ(std::string("123"), item);

    status = getDictItem(dict, "timeoffset", item);
    EQMSG("child 2 timeoffset", TCL_OK, status);
    EQMSG("child 2 timeoffset value", std::string("2"), item);

    status = getDictItem(dict, "realtime", item);
    EQMSG("child 2 realtime", TCL_OK, status);
    EQMSG("child 2 realtime value", std::string("2"), item);

    status = getDictItem(dict, "title", item);
    EQMSG("child 2 title", TCL_OK, status);
    EQ(std::string("A test title"), item);

    status = getDictItem(dict, "timestamp", item);
    EQMSG("child 2 timestamp", TCL_OK, status);
    EQ(std::string("12345679"), item);

    status = getDictItem(dict, "source", item);
    EQMSG("child 2 source", TCL_OK, status);
    EQ(std::string("2"), item);


}



void RingTests::getWithPredicate()
{
    // We're going to request an item using a predicate that only allows
    // BEGIN_RUN and END_RUN items.  Then we'll emit a begin run and a bunch
    // of events and an end run.  We should only see the begin and
    // end runs.
    
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    insertStateChange(V12::BEGIN_RUN);
    
    for (int i =0; i < 100; i++) {
        emitEvent();
        emitEvent();
    }    
    insertStateChange(V12::END_RUN);

    stat = tryCommand("ring get tcp://localhost/tcltestring [list 1 2]");
    Tcl_Obj* event1 = Tcl_GetObjResult(m_pInterp->getInterpreter());
   
    std::string item;
    getDictItem(event1, "type", item);
    EQ(std::string("Begin Run"), item);
    
    stat = tryCommand("ring get tcp://localhost/tcltestring [list 1 2]");
    Tcl_Obj* event2 = Tcl_GetObjResult(m_pInterp->getInterpreter());

    getDictItem(event2, "type", item);
    EQ(std::string("End Run"), item);
    
    
}
void RingTests::getAbnormalEnd()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    CRingDataSink ring("tcltestring");
    V12::CAbnormalEndItem item;
    writeItem(ring, item);
    
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    Tcl_Obj* received = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    std::string itemValue;
    getDictItem(received, "type", itemValue);
    EQ(std::string("Abnormal End"), itemValue);
}

void RingTests::getTimeout_0()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    CRingBuffer ring("tcltestring", CRingBuffer::producer);

    stat = tryCommand("ring get -timeout 1 tcp://localhost/tcltestring ");

    EQ(TCL_OK, stat);
    EQ(std::string(""), getResult());
}

void RingTests::getTimeout_1()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");

    insertStateChange(V12::BEGIN_RUN);

    stat = tryCommand("ring get -timeout 1 tcp://localhost/tcltestring 1");

    EQ(TCL_OK, stat);

    Tcl_Obj* event2 = Tcl_GetObjResult(m_pInterp->getInterpreter());

    std::string itemValue;
    getDictItem(event2, "type", itemValue);
    EQ(std::string("Begin Run"), itemValue);
}

