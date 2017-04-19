#include "CUnglom.h"
#include <V12/CRawRingItem.h>
#include <V12/CCompositeRingItem.h>
#include <V12/DataFormat.h>
#include <V12/Serialize.h>
#include <ByteBuffer.h>
#include <RingIOV12.h>

namespace DAQ {

CUnglom::CUnglom(CDataSourcePtr pSource, CDataSinkPtr pSink)
    : m_pSource(pSource),
      m_pSink(pSink)
{
}


bool CUnglom::processOne()
{
    V12::CRawRingItem item;
    readItem(*m_pSource, item);
    if (m_pSource->eof()) return false;

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

void CUnglom::writeFragment(CDataSink& sink, V12::CRingItem& item)
{
    Buffer::ByteBuffer fragment;
    fragment << item.getEventTimestamp();
    fragment << item.getSourceId();
    fragment << item.size();
    fragment << barrierType(item);
    fragment << V12::serializeItem(item);

    sink.put(fragment.data(), fragment.size());
}

} // end DAQ
