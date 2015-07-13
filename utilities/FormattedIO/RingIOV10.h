#ifndef NSCLDAQ10_RINGIO_H
#define NSCLDAQ10_RINGIO_H

#include <iosfwd>

class CDataSource;
class CDataSink;

namespace DAQ
{
  namespace V10 {
    class CRingItem;
  }
}

extern std::ostream& operator<<(std::ostream& stream,
                                const DAQ::V10::CRingItem& item);

extern CDataSink& operator<<(CDataSink& stream,
                             const DAQ::V10::CRingItem& item);

extern std::istream& operator>>(std::istream& stream,
                                DAQ::V10::CRingItem& item);

extern CDataSource& operator>>(CDataSource& stream,
                               DAQ::V10::CRingItem& item);

#endif // RINGIO_H
