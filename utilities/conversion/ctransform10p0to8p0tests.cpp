
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <NSCLDAQ8/CScalerBuffer.h>
#include <NSCLDAQ10/CRingScalerItem.h>

#include <NSCLDAQ8/CControlBuffer.h>
#include <NSCLDAQ10/CRingStateChangeItem.h>

#include <NSCLDAQ8/CPhysicsEventBuffer.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>

#include <NSCLDAQ8/CTextBuffer.h>
#include <NSCLDAQ10/CRingTextItem.h>

#include <NSCLDAQ8/DataFormatV8.h>
#include <NSCLDAQ10/DataFormatV10.h>

#include <NSCLDAQ8/format_cast.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <CTransform10p0to8p0.h>
#undef protected
#undef private

#include <iterator>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cassert>


using namespace std;

using namespace DAQ;



class CTransform10p0to8p0Tests_Scaler : public CppUnit::TestFixture
{
private:
    Transform::CTransform10p0to8p0 m_transform;
    vector<uint32_t> m_scalers;
    V8::CScalerBuffer m_item;
    std::uint32_t m_offsetBegin;
    std::uint32_t m_offsetEnd;
    std::time_t m_tstamp;
    std::tm     m_calTime;

public:
    CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Scaler);
//    CPPUNIT_TEST(scaler_0);
//    CPPUNIT_TEST(scaler_1);
//    CPPUNIT_TEST(scaler_2);
//    CPPUNIT_TEST(scaler_4);
//    CPPUNIT_TEST(scaler_5);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform10p0to8p0Tests_Scaler() :
      m_transform(),
      m_scalers(), m_item(),
      m_offsetBegin(0),
      m_offsetEnd(0),
      m_tstamp(0),
      m_calTime() {}

    void setUp() {
        m_transform = Transform::CTransform10p0to8p0();

        m_scalers = vector<uint32_t>(16);
        iota(m_scalers.begin(), m_scalers.end(), 0);

        m_offsetBegin = 0x1234;
        m_offsetEnd   = 0x5678;

        m_tstamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm* pTime = std::localtime(&m_tstamp);

        if (pTime != nullptr) {
          m_calTime = *pTime;
        }

        NSCLDAQ10::CRingScalerItem scaler(m_offsetBegin, m_offsetEnd, m_tstamp, m_scalers);

        m_item = m_transform( scaler );

    }
  void tearDown() {
  }

protected:
//  void scaler_0() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type transforms to SCALERBF",
//                                 V8::SCALERBF, m_item.getHeader().type);
//  }

//  void scaler_1() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval offset begin value is preserved",
//                                 m_offsetBegin, m_item.getStartTime());
//  }

//  void scaler_2() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval offset end value is preserved",
//                                 m_offsetEnd, m_item.getEndTime());
//  }

////  void scaler_3() {
////    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is what we demand to be returned",
////                                 DEFAULT_TSTAMP, m_item.getTimestamp());
////  }

//  void scaler_4() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("scaler count must be the same as m_header.nevt",
//                                 std::uint32_t(m_header.nevt), m_item.getScalerCount());
//  }

//  void scaler_5() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("scaler values must be the same",
//                                 m_scalers, m_item.getScalers());
//  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Scaler);



    ///////////////////////////////////////////////////////////////////////////

    class CTransform10p0to8p0Tests_Text : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Text);
        CPPUNIT_TEST(Text_0);
        //CPPUNIT_TEST(Text_1);
        CPPUNIT_TEST(Text_2);
        CPPUNIT_TEST(Text_4);
        CPPUNIT_TEST(Text_5);
        CPPUNIT_TEST(Text_6);
        CPPUNIT_TEST(Text_7);
        CPPUNIT_TEST(Text_8);
        CPPUNIT_TEST(Text_9);
        CPPUNIT_TEST(Text_10);
        CPPUNIT_TEST(Text_11);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::CTextBuffer                v8item;
        NSCLDAQ10::CRingTextItem       v10item;
        Transform::CTransform10p0to8p0 m_transform;
        std::vector<std::string>       m_strings;
        std::uint32_t                  m_offsetTime;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to8p0Tests_Text()
          : v8item(), v10item(NSCLDAQ10::MONITORED_VARIABLES),
            m_transform(),
            m_strings() ,
            m_offsetTime() {}

        void setUp()
        {
          m_transform = Transform::CTransform10p0to8p0();

          m_strings = {"why", "did", "the", "chicken", "cross", "the", "road?"};

          std::time_t m_tstamp = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

          v10item = NSCLDAQ10::CRingTextItem(NSCLDAQ10::MONITORED_VARIABLES,
                                        m_strings, m_offsetTime, m_tstamp);

          v8item = m_transform( v10item );
        }

        void tearDown() {

        }

        void Text_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("MONITORED_VARIABLES --> RUNVARBF",
                                       std::uint16_t(V8::RUNVARBF), v8item.getHeader().type);
        }

// not sure how to test the next one b/c it depends on the output of the time() function.
        // I could play tricks with the include path, but that is non trivial.
        void Text_1()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum is set to 0",
                                       std::uint16_t(0), v8item.getHeader().cks);
        }

        void Text_2()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number is set to 0",
                                       std::uint16_t(0), v8item.getHeader().run);
        }

        void Text_4()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nevt will be set to number of strings",
                                       std::uint16_t(m_strings.size()), v8item.getHeader().nevt);
        }

        void Text_5()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("processor number set to 0",
                                       std::uint16_t(0), v8item.getHeader().cpu);

        }
        void Text_6()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nbit set to 0",
                                       std::uint16_t(0), v8item.getHeader().nbit);
        }

        void Text_7()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("buffmt set to 5",
                                       std::uint16_t(5), v8item.getHeader().buffmt);
        }

        void Text_8()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("ssignature set to 0x0102",
                                       std::uint16_t(0x0102),
                                       v8item.getHeader().ssignature);
        }

        void Text_9()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("lsignature set to 0x0102",
                                       std::uint32_t(0x01020304),
                                       v8item.getHeader().lsignature);
        }

        void Text_10()
        {
          V8::CTextBuffer textBuf(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings will be copied in unaltered",
                                       m_strings, textBuf.getStrings());
        }

        void Text_11()
        {
          std::time_t tstamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

          NSCLDAQ10::CRingTextItem text(NSCLDAQ10::PACKET_TYPES, m_strings, m_offsetTime, tstamp);

          v8item = m_transform( text );

          V8::CTextBuffer buffer(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("PACKET_TYPES --> PKTDOCBF",
                                       V8::PKTDOCBF, buffer.type());
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Text);


//    class CTransform10p0to8p0Tests_PhysicsEvent : public CppUnit::TestFixture
//    {

//        public:
//        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_PhysicsEvent);
//        CPPUNIT_TEST(Event_0);
//        CPPUNIT_TEST(Event_1);
//        CPPUNIT_TEST(Event_2);
//        CPPUNIT_TEST(Event_3);
//        CPPUNIT_TEST_SUITE_END();

//    public:
//        V8::bheader m_header;
//        NSCLDAQ10::CPhysicsEventItem v10item;
//        Transform::CTransform10p0to8p0 m_transform;
//        Buffer::ByteBuffer m_body;

//    public:
//        // We need to define a default constructor b/c the CRingTextItem classes
//        // do not define a default constructor.
//        CTransform10p0to8p0Tests_PhysicsEvent()
//          : v10item(8192),
//            m_transform(),
//            m_body() {}

//        void setUp()
//        {
//          m_transform = Transform::CTransform10p0to8p0();

//          m_header.type = V8::DATABF;
//          m_header.buffmt = V8::StandardVsn;
//          m_header.nevt = 1;
//          m_header.nwds = 18; // i think this includes the size of the header?
//          m_header.ssignature = V8::BOM16;
//          m_header.lsignature = V8::BOM32;

//          std::vector<std::uint16_t> body({2, 1234});

//        m_body = Buffer::ByteBuffer();
//          m_body << body;

//          V8::CPhysicsEventBuffer event(m_header, body);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(event) );
//        }

//        void tearDown() {

//        }

//        void Event_0()
//        {
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("DATABF --> PHYSICS_EVENT",
//                                       NSCLDAQ10::PHYSICS_EVENT, v10item.type());
//        }

//        void Event_1()
//        {
//          const char* pBody =reinterpret_cast<const char*>(v10item.getBodyPointer());

//          // copy the body into something safer and easier to work with
//          Buffer::ByteBuffer body(pBody, pBody+v10item.getBodySize());

//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body remains the same",
//                                       m_body, body);
//        }

//        void adaptSetupForMultipleEvents() {
//          std::vector<std::uint16_t> body({2, 0x1234, 2, 0x5678, 2, 0x9abc});

//          m_header.nevt = 3;
//          m_header.nwds = 22;

//          m_body = Buffer::ByteBuffer();
//          m_body << body;

//          V8::CPhysicsEventBuffer event(m_header, body);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(event) );
//        }

//        void Event_2()
//        {
//          adaptSetupForMultipleEvents();
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("2 more physics events are present",
//                                       std::size_t(2), m_transform.getRemainingEvents().size());
//        }


//        void Event_3()
//        {
//          adaptSetupForMultipleEvents();

//          v10item = m_transform.getRemainingEvents().front();

//          const char* pBody = reinterpret_cast<const char*>(v10item.getBodyPointer());

//          // copy the body into something safer and easier to work with
//          Buffer::ByteBuffer bodyBytes(pBody, pBody+v10item.getBodySize());

//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body remains the same",
//                                       Buffer::ByteBuffer({2, 0, 0x78, 0x56}), bodyBytes );
//        }

//    };
//    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_PhysicsEvent);

//    class CTransform10p0to8p0Tests_Control : public CppUnit::TestFixture
//    {

//        public:
//        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Control);
//        CPPUNIT_TEST(Control_0);
//        CPPUNIT_TEST(Control_1);
//        CPPUNIT_TEST(Control_2);
//        CPPUNIT_TEST(Control_3);
//        CPPUNIT_TEST(Control_4);
//        CPPUNIT_TEST(Control_5);
//        CPPUNIT_TEST(Control_6);
//        CPPUNIT_TEST(Control_7);
//        CPPUNIT_TEST_SUITE_END();

//    public:
//        V8::bheader m_header;
//        NSCLDAQ10::CRingStateChangeItem v10item;
//        Transform::CTransform10p0to8p0 m_transform;
//        std::uint32_t m_offset;
//        std::string m_title;
//        DAQ::V8::bftime m_tstruct;

//    public:
//        // We need to define a default constructor b/c the CRingTextItem classes
//        // do not define a default constructor.
//        CTransform10p0to8p0Tests_Control()
//          : v10item(NSCLDAQ10::BEGIN_RUN),
//            m_transform()
//        {}

//        void setUp()
//        {
//          m_transform = Transform::CTransform10p0to8p0();

//          m_header.type = V8::BEGRUNBF;
//          m_header.buffmt = V8::StandardVsn;
//          m_header.nwds = 65;
//          m_header.nevt = 0;
//          m_header.run = 1357;
//          m_header.ssignature = V8::BOM16;
//          m_header.lsignature = V8::BOM32;

//          m_title = "a title for you and me";
//          m_title.resize(80, ' '); // needed b/c CControlBuffer stretches title to 80 chars
//                                   // we must do the same if we are to use it to compare

//          m_offset = 0x12345678;

//          m_tstruct = {1, 2, 1971, 3, 4, 5, 6};

//          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
//        }

//        void tearDown() {

//        }

//        void Control_0()
//        {
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("BEGRUNBF --> BEGIN_RUN",
//                                       NSCLDAQ10::BEGIN_RUN, v10item.type());
//        }

//        void Control_1()
//        {
//          cout << m_title.size() << endl;
//          cout << v10item.getTitle().size() << endl;
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Title remains the same",
//                                       m_title, v10item.getTitle());
//        }

//        void Control_2()
//        {
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset remains the same",
//                                       m_offset, v10item.getElapsedTime());
//        }

//        void Control_3()
//        {
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number remains the same",
//                                       static_cast<std::uint32_t>(m_header.run),
//                                       v10item.getRunNumber());
//        }

//        void Control_4()
//        {

//          std::tm calTime;
//          calTime.tm_mon  = m_tstruct.month;
//          calTime.tm_mday = m_tstruct.day;
//          calTime.tm_year = m_tstruct.year - 1900;
//          calTime.tm_hour = m_tstruct.hours;
//          calTime.tm_min  = m_tstruct.min;
//          calTime.tm_sec  = m_tstruct.sec;

//          std::time_t time = std::mktime(&calTime);
//          std::time_t time2 = std::mktime(&calTime);
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp converts as expected",
//                                       time2, time);
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp converts as expected",
//                                       time, v10item.getTimestamp());
//        }

//        void Control_5 () {
//          m_header.type = V8::ENDRUNBF;

//          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("ENDRUNBF --> END_RUN",
//                                       NSCLDAQ10::END_RUN, v10item.type());

//        }

//        void Control_6 () {
//          m_header.type = V8::PAUSEBF;

//          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("PAUSEBF --> PAUSE_RUN",
//                                       NSCLDAQ10::PAUSE_RUN, v10item.type());

//        }

//        void Control_7 () {
//          m_header.type = V8::RESUMEBF;

//          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

//          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("RESUMEBF --> RESUME_RUN",
//                                       NSCLDAQ10::RESUME_RUN, v10item.type());

//        }

//    };
//    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Control);



