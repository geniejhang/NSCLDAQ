// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "DataFormat.h"
#include <string.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////////////////////
// Event item tests.

class PhysicsItemOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PhysicsItemOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(counting);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void empty();
  void counting();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsItemOutput);

// An empty item should just contain a longword  in the
// s_body field and the size should match that.

void PhysicsItemOutput::empty() {
  pPhysicsEventItem pItem = formatEventItem(0, NULL);
  ASSERT(pItem);

  EQMSG("Size of empty event item",
	static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(uint32_t)*2), pItem->s_header.s_size);
  EQMSG("Type of empty event",
	PHYSICS_EVENT, pItem->s_header.s_type);

  
  uint32_t* pPayload = reinterpret_cast<uint32_t*>(pItem->s_body.u_noBodyHeader.s_body);
  EQMSG("Payload contents",
	sizeof(uint32_t)/sizeof(uint16_t), static_cast<size_t>(*pPayload));


  free(pItem);
}

// Check for an event with a counting pattern of 0-9
// - Size ok.
// - Payload size ok.
// - Contents ok.
// The type is assumed to have been validated by empty().
//

void
PhysicsItemOutput::counting()
{
  // build the event payload:

  uint16_t payload[10];
  for (int i =  0; i < 10; i++) { payload[i] = i;}

  pPhysicsEventItem pItem = formatEventItem(10, payload);

  ASSERT(pItem);
  
  EQMSG("Counting item size",
	static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(uint32_t)*2 + 10*sizeof(uint16_t)),
	pItem->s_header.s_size);


  struct PayloadShape {
    uint32_t s_size;
    uint16_t s_body[10];
  };
  PayloadShape* pBody = reinterpret_cast<PayloadShape*>(pItem->s_body.u_noBodyHeader.s_body);


  EQMSG("Size in payload",
	static_cast<uint32_t>(sizeof(uint32_t)/sizeof(uint16_t) + 10),pBody->s_size);

  for(uint16_t i = 0; i < 10; i++) {
    EQMSG("Contents in payload",
	  i, pBody->s_body[i]);
  }
	
       

  free(pItem);
}
/////////////////////////////////////////////////////////////////////////////////
// Event trigger count item tests.

class PhysicsCountOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PhysicsCountOutput);
  CPPUNIT_TEST(itemformat);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void itemformat();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsCountOutput);


void
PhysicsCountOutput::itemformat()
{
  pPhysicsEventCountItem pItem = formatTriggerCountItem(1234, 666, 0xaaaa);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(PhysicsEventCountItemBody);
  EQMSG("Physics count size",
	properSize, pItem->s_header.s_size);
  EQMSG("Physics count type",
	PHYSICS_EVENT_COUNT, pItem->s_header.s_type);
  EQMSG("Time offset",  static_cast<uint32_t>(1234), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG("Time stamp", static_cast<uint32_t>(666), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG("Trigger count", static_cast<uint64_t>(0xaaaa), pItem->s_body.u_noBodyHeader.s_body.s_eventCount);


  free(pItem);
}
////////////////////////////////////////////////////////////////////////////////////
//  Check scaler items.

class ScalerOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ScalerOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(counting);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {}
  void tearDown() {}

protected:
  void empty();
  void counting();
};
CPPUNIT_TEST_SUITE_REGISTRATION(ScalerOutput);


// Check zero scaler count
//
void
ScalerOutput::empty()
{
  pScalerItem pItem = formatScalerItem(0, 0x1234, 0, 1, NULL);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(ScalerItemBody)
    - sizeof(uint32_t);
    
  EQMSG("Empty scaler size", properSize, pItem->s_header.s_size);
  EQMSG("Scaler type: ", PERIODIC_SCALERS, pItem->s_header.s_type);
  EQMSG("start time", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_body.s_intervalStartOffset);
  EQMSG("stop time",  static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_intervalEndOffset);
  EQMSG("timestamp",  static_cast<uint32_t>(0x1234), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG("Count",      static_cast<uint32_t>(0),      pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);
  EQMSG("Incremental", static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_isIncremental);
  EQMSG("Time divisor", static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_intervalDivisor);

  free(pItem);
}

//
// Check with a counting pattern of 10 scalers.
void
ScalerOutput::counting()
{
  // the scalers:

  uint32_t scalers[10];
  for (int i=0; i < 10; i++) { scalers[i] = i;}

  pScalerItem pItem = formatScalerItem(10, 0x4567, 0, 1, scalers);

  // Assume the only things to check are:
  // - Size of the entire item.
  // - Count of scalers.
  // - Payload.
  //
  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(ScalerItemBody)
    + 9*sizeof(uint32_t);
  EQMSG("Counting scaler size", properSize, pItem->s_header.s_size);
  EQMSG("No of scalers", static_cast<uint32_t>(10),
        pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);

  for (uint32_t i = 0; i < 10; i++) {
    EQMSG("Scaler payload",  i, pItem->s_body.u_noBodyHeader.s_body.s_scalers[i]);
  }
  

  free(pItem);
}
///////////////////////////////////////////////////////////////////////////////
// Test text output item.

class TextOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TextOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(someStrings);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  void tearDown() {}

protected:
  void empty();
  void someStrings();

};
CPPUNIT_TEST_SUITE_REGISTRATION(TextOutput);

//
// Empty test.. 
// - Size should be sizeof(Textitem - sizeof(char)
// - Type shoujild be whatever we tell it to be.
// - Time offset, timestamp should be what we tell it to be
// - string count should be zero.
//
void
TextOutput::empty()
{
  pTextItem pItem = formatTextItem(0, 0xaaaa, 0xbbbb, NULL, MONITORED_VARIABLES);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody) - sizeof(char);
  EQMSG("Empty text item size", properSize, pItem->s_header.s_size);
  EQMSG("Type ", MONITORED_VARIABLES, pItem->s_header.s_type);
  EQMSG(" time offset", static_cast<uint32_t>(0xbbbb), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG(" timestamp", static_cast<uint32_t>(0xaaaa), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG(" string count", static_cast<uint32_t>(0),   pItem->s_body.u_noBodyHeader.s_body.s_stringCount);

  free(pItem);
}
//
// put a few strings in, check the string count and check the string contents.
//
void
TextOutput::someStrings()
{
  const char* strings[] = {	// 4 strings.
    "First string",
    "Second String",
    "Third string",
    "Last String"
  };

  uint32_t stringSize = 0;
  for (int i=0; i < 4; i++) {
    stringSize += strlen(strings[i]) + 1;
  }

  pTextItem pItem = formatTextItem(4, 0xaaaa, 0xbbbb, strings, MONITORED_VARIABLES);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody)
    + stringSize - sizeof(char);
  EQMSG("Item size",    properSize, pItem->s_header.s_size);
  EQMSG("String count", static_cast<uint32_t>(4), pItem->s_body.u_noBodyHeader.s_body.s_stringCount);
 
  char* p = pItem->s_body.u_noBodyHeader.s_body.s_strings;

  for (int i = 0; i < 4; i++) {
    EQMSG("Contents", std::string(strings[i]), std::string(p));
    p += strlen(p) + 1;
  }
  

  free(pItem);
  
}

///////////////////////////////////////////////////////////////////////////////////////////
// Check the state change items:
//

class StateChangeOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StateChangeOutput);
  CPPUNIT_TEST(begin);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}
protected:
  void begin();
};
CPPUNIT_TEST_SUITE_REGISTRATION(StateChangeOutput);

void
StateChangeOutput::begin()
{
  pStateChangeItem pItem = formatStateChange(0x66eb, 0, 1234, "This is a test title",
					     BEGIN_RUN);

  ASSERT(pItem);
  EQMSG("State change item sizse", static_cast<uint32_t>(sizeof(StateChangeItem)), pItem->s_header.s_size);
  EQMSG("Item type", BEGIN_RUN, pItem->s_header.s_type);
  EQMSG("Run Number", static_cast<uint32_t>(1234), pItem->s_body.u_noBodyHeader.s_body.s_runNumber);
  EQMSG("Time offset", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG("Timestamp", static_cast<uint32_t>(0x66eb), pItem->s_body.u_noBodyHeader.s_body.s_Timestamp);
  EQMSG("Title", std::string("This is a test title"), std::string(pItem->s_body.u_noBodyHeader.s_body.s_title));

  free(pItem);
}

