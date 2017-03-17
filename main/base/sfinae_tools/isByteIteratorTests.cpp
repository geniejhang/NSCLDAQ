


#include <cppunit/extensions/HelperMacros.h>
#include <Asserts.h>


#include <is_byte_iterator.h>
#include <iterator>
#include <vector>


struct A {
    using value_type = int;
    using difference_type = int;
    using pointer = int*;
    using reference = int&;
    using iterator_category = std::random_access_iterator_tag;
};


class IsByteIteratorTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(IsByteIteratorTests);
    CPPUNIT_TEST( iterator_0 );
    CPPUNIT_TEST( iterator_1 );
    CPPUNIT_TEST( iterator_2 );
    CPPUNIT_TEST( iterator_3 );
    CPPUNIT_TEST( iterator_4 );
    CPPUNIT_TEST( iterator_5 );
    CPPUNIT_TEST( iterator_6 );
    CPPUNIT_TEST( iterator_7 );
    CPPUNIT_TEST( iterator_8 );
    CPPUNIT_TEST( iterator_9 );
    CPPUNIT_TEST( iterator_10 );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}


    void iterator_0() {
        CPPUNIT_ASSERT_MESSAGE( "std::vector<int>::iterator",
                                ! DAQ::is_byte_iterator< std::vector<int>::iterator >::value);
    }

    void iterator_1() {
        CPPUNIT_ASSERT_MESSAGE( "int",
                                ! DAQ::is_byte_iterator<int>::value);
    }

    void iterator_2() {
        CPPUNIT_ASSERT_MESSAGE( "int*",
                                ! DAQ::is_byte_iterator<int*>::value);
    }

    void iterator_3() {
        CPPUNIT_ASSERT_MESSAGE( "user defined iterator",
                                ! DAQ::is_byte_iterator<A>::value);
    }


    void iterator_4() {
        CPPUNIT_ASSERT_MESSAGE( "uint8_t*",
                                DAQ::is_byte_iterator<uint8_t*>::value);
    }

    void iterator_5() {
        CPPUNIT_ASSERT_MESSAGE( "char*",
                                DAQ::is_byte_iterator<char*>::value);
    }

    void iterator_6() {
        CPPUNIT_ASSERT_MESSAGE( "unsigned char*",
                                DAQ::is_byte_iterator<unsigned char*>::value);
    }

    void iterator_7() {
        CPPUNIT_ASSERT_MESSAGE( "char",
                                ! DAQ::is_byte_iterator<char>::value);
    }

    void iterator_8() {
        CPPUNIT_ASSERT_MESSAGE( "std::vector<char>::iterator",
                                DAQ::is_byte_iterator< std::vector<char>::iterator >::value);
    }

    void iterator_9() {
        CPPUNIT_ASSERT_MESSAGE( "std::vector<char>::const_iterator",
                                DAQ::is_byte_iterator< std::vector<char>::const_iterator >::value);
    }

    void iterator_10() {
        CPPUNIT_ASSERT_MESSAGE( "std::vector<uint8_t>::const_iterator",
                                DAQ::is_byte_iterator< std::vector<uint8_t>::const_iterator >::value);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(IsByteIteratorTests);
