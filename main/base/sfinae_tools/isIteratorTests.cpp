


#include <cppunit/extensions/HelperMacros.h>
#include <Asserts.h>


#include <is_iterator.h>
#include <iterator>
#include <vector>


struct A {
    using value_type = int;
    using difference_type = int;
    using pointer = int*;
    using reference = int&;
    using iterator_category = std::random_access_iterator_tag;
};


class IsIteratorTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(IsIteratorTests);
    CPPUNIT_TEST( iterator_0 );
    CPPUNIT_TEST( iterator_1 );
    CPPUNIT_TEST( iterator_2 );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}


    void iterator_0() {
        CPPUNIT_ASSERT_MESSAGE( "std::vector<int>::iterator",
                                DAQ::is_iterator< std::vector<int>::iterator >::value);
    }

    void iterator_1() {
        CPPUNIT_ASSERT_MESSAGE( "int",
                                ! DAQ::is_iterator<int>::value);
    }

    void iterator_2() {
        CPPUNIT_ASSERT_MESSAGE( "int*",
                                DAQ::is_iterator<int*>::value);
    }

    void iterator_3() {
        CPPUNIT_ASSERT_MESSAGE( "user defined iterator",
                                ! DAQ::is_iterator<A>::value);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(IsIteratorTests);
