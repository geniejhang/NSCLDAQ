

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <V12/CCompositeRingItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingStateChangeItem.h>
#include <ContainerDeserializer.h>

#include <CTestSourceSink.h>
#include <RingIOV12.h>

#include <fragment.h>
#include <CUnglom.h>

#include <utility>
#include <tuple>

using namespace DAQ;

class CUnglomTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CUnglomTests);
    CPPUNIT_TEST(processOne_0);
    CPPUNIT_TEST(processOne_1);
    CPPUNIT_TEST(processOne_2);
    CPPUNIT_TEST_SUITE_END();


    CTestSourceSinkPtr m_pSource;
    CTestSourceSinkPtr m_pSink;

public:
    void setUp() {

        m_pSource = std::make_shared<CTestSourceSink>();
        m_pSink   = std::make_shared<CTestSourceSink>();

    }
    void tearDown() {}

    // a composite item with no children should output nothing
    void processOne_0() {
        CUnglom glom(m_pSource, m_pSink);

        V12::CCompositeRingItem item;
        item.setType(V12::COMP_BEGIN_RUN);

        writeItem(*m_pSource, item);
        glom.processOne();

        EQMSG("amount of data outputted", size_t(0),
              m_pSink->getBuffer().size());
    }

    // a composite item with one child should output nothing
    void processOne_1() {
        CUnglom glom(m_pSource, m_pSink);

        V12::CCompositeRingItem item;
        item.setType(V12::COMP_BEGIN_RUN);
        item.appendChild<V12::CRingStateChangeItem>(V12::BEGIN_RUN);

        writeItem(*m_pSource, item);
        glom.processOne();

        readFragment();
        EQMSG("full item was read", false, m_pSink->eof() );
    }


    std::pair<EVB::FragmentHeader, V12::CRawRingItem> readFragment()
    {
        auto stream = Buffer::makeContainerDeserializer(m_pSink->getBuffer(), false);
        EVB::FragmentHeader frag;

        // because the EVB::FragmentHeader is a "packed" structure, the compiler
        // does not allow its data members to bind to uint32_t& or uint64_t& l-values.
        // This is because there is no gaurantee that alignment rules won't be broken.
        // For that reason, an intermediate step needs to be taken to read into a temporary
        // variable and then assign its value to the fragment header.
        uint64_t temp64;
        uint32_t temp32;
        stream >> temp64; frag.s_timestamp = temp64;
        stream >> temp32; frag.s_sourceId = temp32;
        stream >> temp32; frag.s_size = temp32;
        stream >> temp32; frag.s_barrier = temp32;
        V12::CRawRingItem rawItem(stream.pos(), stream.pos() + frag.s_size);

        return std::make_pair(frag, rawItem);
    }

    void processOne_2() {
        CUnglom glom(m_pSource, m_pSink);

        V12::CRawRingItem item(V12::PHYSICS_EVENT, 0, 1);

        writeItem(*m_pSource, item);
        glom.processOne();

        EVB::FragmentHeader frag;
        V12::CRawRingItem rawItem;

        std::tie(frag, rawItem) = readFragment();

        EQMSG("frag timestamp", uint64_t(0), frag.s_timestamp );
        EQMSG("frag sourceId", uint32_t(1), frag.s_sourceId );
        EQMSG("frag size", uint32_t(20), frag.s_size );
        EQMSG("frag barrier", uint32_t(0), frag.s_barrier );
        EQMSG("item type", V12::PHYSICS_EVENT, rawItem.type());
        EQMSG("item timestamp", uint64_t(0), rawItem.getEventTimestamp());
        EQMSG("item source id", uint32_t(1), rawItem.getSourceId());
        EQMSG("item size", uint32_t(20), rawItem.size());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CUnglomTests);
