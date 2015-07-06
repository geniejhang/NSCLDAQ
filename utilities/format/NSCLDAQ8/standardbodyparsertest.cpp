

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include <DataFormatV8.h>
#include <make_unique.h>

#define private public
#define protected public
#include <CStandardBodyParser.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class standardbodyparsertest : public CppUnit::TestFixture {
private:
  DAQ::Buffer::ByteBuffer m_bodyData;
  CStandardBodyParser   m_parser;
  BufferPtr<uint16_t> m_beg;
  BufferPtr<uint16_t> m_deadend;

public:
  CPPUNIT_TEST_SUITE(standardbodyparsertest);
  CPPUNIT_TEST(parseOne_0);
  CPPUNIT_TEST(parseOne_1);
  CPPUNIT_TEST(parseOne_2);
  CPPUNIT_TEST(parseOne_3);
  CPPUNIT_TEST(parseOne_4);
  CPPUNIT_TEST(parse_0);
  CPPUNIT_TEST(parse_1);
  CPPUNIT_TEST(parse_2);
  CPPUNIT_TEST(parse_3);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    std::vector<uint16_t> data({3, 0, 1, 2, 3});

    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << data;

    m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), false);
    m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), false);

    m_parser = CStandardBodyParser();

}

  void tearDown()
  {
  }

  void parseOne_0() {

    auto result = m_parser.parseOne(m_beg, m_deadend);

    CPPUNIT_ASSERT_MESSAGE("Iterator after parsing 1 is correct",
                           (m_beg+3) == std::get<1>(result));
  }

  void parseOne_1() {
    auto result = m_parser.parseOne(m_beg, m_deadend);

    CPPUNIT_ASSERT_MESSAGE("Parse one produces proper body",
                           std::equal(m_beg, m_beg+3, std::get<0>(result)->begin()));
  }

  void parseOne_2() {

    // create a bad set of data (size = 10 but there are not 10 words)
    vector<uint16_t> data({10, 0, 1, 2, 3});

    // load the buffer with the bad data
    ByteBuffer buffer;
    buffer << data;

    m_beg     = BufferPtr<uint16_t>(buffer.begin(), false);
    m_deadend = BufferPtr<uint16_t>(buffer.end(), false);

    CPPUNIT_ASSERT_THROW_MESSAGE("Inconsistent size of buffer causes throw",
                                 m_parser.parseOne(m_beg,  m_deadend),
                                 std::runtime_error);
  }

  void parseOne_3() {
    // create a bad set of data (size = 10 but there are not 10 words)
    vector<uint16_t> data({0, 0, 1, 2, 3});

    // load the buffer with the bad data
    ByteBuffer buffer;
    buffer << data;

    m_beg     = BufferPtr<uint16_t>(buffer.begin(), false);
    m_deadend = BufferPtr<uint16_t>(buffer.end(), false);

    CPPUNIT_ASSERT_THROW_MESSAGE("Zero buffer size causes throw",
                               m_parser.parseOne(m_beg, m_deadend),
                               std::runtime_error);
  }

    void parseOne_4() {
      m_deadend = m_beg-1;
      CPPUNIT_ASSERT_THROW_MESSAGE("Begin iter is beyond deadend iter throws",
                                 m_parser.parseOne(m_beg, m_deadend),
                                 std::runtime_error);
    }

    void parse_0() {

      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "parsing 2 physics event should produce 2 physics events",
            size_t(2), result.size());
    }

    void parse_1() {
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "first physics event makes sense",
            std::equal(m_beg, m_beg+3, result.at(0)->begin()));
    }

    void parse_2() {
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense",
            std::equal(m_beg+3, m_deadend, result.at(1)->begin()));
    }

    void parse_3() {

      // test that we can handle parsing byte-reversed data
      std::vector<uint16_t> expected({3, 0, 1, 2, 3});

      std::vector<uint16_t> data({0x0300, 0, 0x0100, 0x0200, 0x0300});

      m_bodyData = DAQ::Buffer::ByteBuffer();
      m_bodyData << data;

      m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), true);
      m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), true);

      m_parser = CStandardBodyParser();
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense even when byte swapped",
            std::equal(m_beg+3, m_deadend, result.at(1)->begin()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(standardbodyparsertest);


