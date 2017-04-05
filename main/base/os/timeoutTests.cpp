
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CTimeout.h"
#include <thread>
#include <chrono>

using namespace DAQ;
using namespace std::chrono;

class timeoutTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(timeoutTests);
  CPPUNIT_TEST(expired_0);
  CPPUNIT_TEST(expired_1);
  CPPUNIT_TEST(remainingSeconds_0);
  CPPUNIT_TEST(remainingSeconds_1);
  CPPUNIT_TEST(remainingTime_0);
  CPPUNIT_TEST(remainingTime_1);
  CPPUNIT_TEST(getTotalTime_0);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
  }
  void tearDown() {
  }

  void expired_0() {
      CTimeout timeout(seconds(10000));

      EQMSG("Timeout should not expire if specified time has not passed",
            false, timeout.expired());
  }

  void expired_1() {
      CTimeout timeout(seconds(0));
      std::this_thread::sleep_for(milliseconds(100));

      EQMSG("Timeout should expire if specified time has passed",
            true, timeout.expired());
  }

  void remainingSeconds_0 () {
      CTimeout timeout(nanoseconds(2432521231231235009));

      CPPUNIT_ASSERT_MESSAGE(
           "Remaining time should be nonzero if not expired",
                  timeout.getRemainingTime().count() > 0);

  }

  void remainingSeconds_1 () {
      CTimeout timeout(0);
      std::this_thread::sleep_for(milliseconds(200));

      EQMSG("Remaining time should be zero if expired",
            double(0), timeout.getRemainingSeconds());

  }


  void remainingTime_0 () {
      CTimeout timeout(seconds(1123));

      CPPUNIT_ASSERT_MESSAGE(
           "Remaining time should be nonzero if not expired",
                  timeout.getRemainingTime() > seconds(1120));

  }

  void remainingTime_1 () {
      CTimeout timeout(0);
      std::this_thread::sleep_for(milliseconds(200));

      ASSERTMSG("Remaining time should be zero if expired (as nano)",
                nanoseconds(0) == timeout.getRemainingTime());

      ASSERTMSG("Remaining time should be zero if expired (as int)",
                0 == timeout.getRemainingTime().count());

  }


  void reset_0() {
    CTimeout timeout(seconds(10));
    std::this_thread::sleep_for(seconds(3));

    ASSERTMSG("Remaining time should be less than 9",
              timeout.getRemainingSeconds() < 8 );

    timeout.reset();

    // note that I am being pretty generous concerning the time left.
    // all i care to see is that the number of remaining seconds
    // increased.
    ASSERTMSG("Reset time have reset the remaining seconds",
              timeout.getRemainingSeconds() > 8 );
  }


    void getTotalTime_0() {
    CTimeout timeout(milliseconds(1002));

    ASSERTMSG("total time should be the same as construction",
              milliseconds(1002) == timeout.getTotalTime());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(timeoutTests);

