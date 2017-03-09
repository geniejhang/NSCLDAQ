

include_HEADERS = is_iterator.h



noinst_PROGRAMS = unittests


unittests_SOURCES = TestRunner.cpp isIteratorTests.cpp isByteIteratorTests.cpp
unittests_CXXFLAGS = -I@top_srcdir@/base/testutils
unittests_LDADD = @CPPUNIT_LDFLAGS@


TESTS = unittests
