#ifndef NSCLDAQ10_RINGIO_H
#define NSCLDAQ10_RINGIO_H

#include <iosfwd>

class CDataSource;
class CDataSink;

namespace NSCLDAQ10
{
    class CRingItem;
}

extern std::ostream& operator<<(std::ostream& stream,
                                const NSCLDAQ10::CRingItem& item);

extern CDataSink& operator<<(CDataSink& stream,
                             const NSCLDAQ10::CRingItem& item);

extern std::istream& operator>>(std::istream& stream,
                                NSCLDAQ10::CRingItem& item);

extern CDataSource& operator>>(CDataSource& stream,
                               NSCLDAQ10::CRingItem& item);

#endif // RINGIO_H
