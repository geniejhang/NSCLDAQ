

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#define private public
#define protected public
#include <BufferPtr.h>
#undef protected
#undef private

using namespace std;

using namespace DAQ::BO;

class byteordertest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(byteordertest);
  CPPUNIT_TEST(copyAs_0);
  CPPUNIT_TEST(copyAs_1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }

  void copyAs_0() {
    union {
      uint16_t asInt;
      char asBytes[sizeof(asInt)];
    } composite;

    composite.asInt = 0x1234;
    CByteSwapper swapper(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non swapping should return the original",
                                 uint16_t(0x1234), swapper.copyAs<uint16_t>(composite.asBytes));

  }

  void copyAs_1() {
    union {
      uint16_t asInt;
      char asBytes[sizeof(asInt)];
    } composite;

    composite.asInt = 0x1234;

    CByteSwapper swapper(true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Swapping should not return the original",
                                 uint16_t(0x3412),
                                 swapper.copyAs<uint16_t>(composite.asBytes));

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(byteordertest);


