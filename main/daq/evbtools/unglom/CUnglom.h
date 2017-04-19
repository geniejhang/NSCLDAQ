#ifndef DAQ_CUNGLOM_H
#define DAQ_CUNGLOM_H

#include <CDataSource.h>
#include <CDataSink.h>

namespace DAQ {

namespace V12 {
class CRingItem;
}

class CUnglom
{
private:
    CDataSourcePtr m_pSource;
    CDataSinkPtr   m_pSink;

public:
    CUnglom(CDataSourcePtr pSource, CDataSinkPtr pSink);

    void operator()();

    bool processOne();

private:
    uint32_t barrierType(V12::CRingItem& item);
    void writeFragment(CDataSink& sink, V12::CRingItem& item);

};

} // end DAQ

#endif // CUNGLOM_H
