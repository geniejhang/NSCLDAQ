
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <ByteBuffer.h>
#include <DebugUtils.h>
#include <vector>
#include <cstdint>

using namespace std;

using namespace DAQ::Buffer;

class bytebuffertest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(bytebuffertest);
  CPPUNIT_TEST(insertion_0);
  CPPUNIT_TEST(insertion_1);
  CPPUNIT_TEST(insert8_0);
  CPPUNIT_TEST(insert8_1);
  CPPUNIT_TEST(insert16_0);
  CPPUNIT_TEST(insert16_1);
  CPPUNIT_TEST(insert16_2);
  CPPUNIT_TEST(insert32_0);
  CPPUNIT_TEST(insert32_1);
  CPPUNIT_TEST(insert64_0);
  CPPUNIT_TEST(insert64_1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }

  void insertion_0() {

    ByteBuffer buffer;
    std::vector<uint16_t> data = {0, 1, 2, 3, 4, 5};

    buffer << data;

    std::vector<uint8_t> expected = {0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0};

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Loading the buffer works",
                                 expected, buffer);
  }

  void insertion_1() {

    ByteBuffer buffer;
    std::vector<uint16_t> data = {0, 1, 2, 3, 4, 5};

    buffer << data;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Buffer size is correct after data insertion",
                                 data.size()*sizeof(uint16_t), buffer.size());
  }

  void insert8_0() {
    ByteBuffer buffer;

    buffer << std::uint8_t(3);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size increments by 1",
                                 std::size_t(1), buffer.size());
  }

  void insert8_1() {
    ByteBuffer buffer;

    buffer << std::uint8_t(3);
    buffer << std::uint8_t(4);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("previous insertions do not get lost using insertion",
                                 std::size_t(2), buffer.size());
  }

  void insert16_0() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::uint16_t(3);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint16_t insert increases size by 2",
                                 sizeof(std::uint16_t), (buffer.size()-originalSize));
  }

  void insert16_1() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::uint16_t(3);

    ByteBuffer expected = {3, 0};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint16_t insert increases size by 2",
                                 expected, buffer);
  }

  void insert16_2() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::int16_t(-3);

    ByteBuffer expected = {253, 255};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint16_t insert stores the correct data",
                                 expected, buffer);
  }

  void insert32_0() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::uint32_t(3);

    ByteBuffer expected = {3, 0, 0, 0};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint32_t insert stores the correct value",
                                 expected, buffer);
  }

  void insert32_1() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::int32_t(-3);

    ByteBuffer expected = {253, 255, 255, 255};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("int32_t insert stores the correct value",
                                 expected, buffer);

  }

  void insert64_0() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::uint64_t(3);

    ByteBuffer expected = {3, 0, 0, 0, 0, 0, 0, 0};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint64_t insert stores the correct value",
                                 expected, buffer);
  }

  void insert64_1() {
    ByteBuffer buffer;

    auto originalSize = buffer.size();
    buffer << std::int64_t(-3);

    ByteBuffer expected = {253, 255, 255, 255, 255, 255, 255, 255};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("uint64_t insert stores the correct value",
                                 expected, buffer);

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(bytebuffertest);


