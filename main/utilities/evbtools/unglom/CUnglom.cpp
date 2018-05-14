#include "CUnglom.h"
#include <V12/CRawRingItem.h>
#include <V12/CCompositeRingItem.h>
#include <V12/DataFormat.h>
#include <V12/Serialize.h>
#include <ByteBuffer.h>
#include <RingIOV12.h>

namespace DAQ {



/*!
 * \brief Constructor
 *
 * \param pSource   the data source
 * \param pSink     the data sink
 */
CUnglom::CUnglom(CDataSourcePtr pSource, CDataSinkPtr pSink)
    : m_pSource(pSource),
      m_pSink(pSink)
{
}


/*!
 * \brief CUnglom::processOne
 *
 * \return true if source is not in eof condition
 * \return false otherwise
 *
 * The idea here is to handle a single ring item read in from a
 * source.
 */
bool CUnglom::processOne()
{
    if (!m_pSource || !m_pSink) {
        throw std::runtime_error("CUnglom::processOne() No Source or sink defined!");
    }

    V12::CRawRingItem item;
    readItem(*m_pSource, item);
    if (m_pSource->eof()) return false;

    // unglom needs to remove any glom info that was read in, note that this
    // does not remove any composite glom infos. Those need to remain and be
    // handled like other composite types.
    if (item.type() == V12::EVB_GLOM_INFO) {
        return true;
    }

    if (item.isComposite()) {
        // write the children of the item if composite (not a recursive operation)
        V12::CCompositeRingItem composite(item);

        for (auto& pChild : composite ) {
            writeFragment(*m_pSink, *pChild);
        }
    } else {
        writeFragment(*m_pSink, item);
    }

    return true;
}

void CUnglom::operator()()
{
    while (processOne()) {}
}

/*!
 * \brief Compute the value to put into the fragment header for the barrier type
 *
 * \param item  a ring item
 *
 * \return the barrier type
 *
 * If the type of the ring item is BEGIN_RUN, END_RUN, PAUSE_RUN, or RESUME_RUN,
 * it the value of BEGIN_RUN, END_RUN, PAUSE_RUN, or RESUME_RUN will be returned
 * respectively. Otherwise, the returned value will be 0.
 */
uint32_t CUnglom::barrierType(V12::CRingItem& item)
{
    uint32_t type = item.type();
    if (type == V12::BEGIN_RUN
            || type == V12::END_RUN
            || type == V12::PAUSE_RUN
            || type == V12::RESUME_RUN) {
        return type;
    } else {
        return 0;
    }
}


/*!
 * \brief Forms a "flat fragment" and writes it to the sink

 * \param sink  data sink
 * \param item  the ring item to output
 *
 * The fragment header is built up and then item is serialized and appended to it.
 */
void CUnglom::writeFragment(CDataSink& sink, V12::CRingItem& item)
{
    Buffer::ByteBuffer fragment;

    // we know how big our buffer needs to be so let's ensure that the allocation
    // occurs only once
    fragment.reserve(20 + item.size());
    fragment << item.getEventTimestamp();
    fragment << item.getSourceId();
    fragment << item.size();
    fragment << barrierType(item);
    fragment << V12::serializeItem(item);

    sink.put(fragment.data(), fragment.size());
}

} // end DAQ
