



#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <DebugUtils.h>


#define private public
#define protected public
#include <CVoidBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class voidbuffertest : public CppUnit::TestFixture {
private:
  CVoidBuffer m_buffer;

public:
  CPPUNIT_TEST_SUITE(voidbuffertest);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(type_0);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_buffer = CVoidBuffer();
  }

  void tearDown()
  {
  }

void toRawBuffer_0 () {
  CRawBuffer rawBuffer;
  m_buffer.toRawBuffer(rawBuffer);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer does what it should",
                               std::size_t(0), rawBuffer.getBuffer().size());
}

void type_0 () {
  CPPUNIT_ASSERT_EQUAL_MESSAGE("type returns VOID",
                               VOID, m_buffer.type());
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(voidbuffertest);



