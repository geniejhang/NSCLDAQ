#ifndef NSCLDAQ11_RINGIO_H
#define NSCLDAQ11_RINGIO_H

#include <iosfwd>

class CDataSource;
class CDataSink;

namespace NSCLDAQ11
{
    class CRingItem;
}

extern std::ostream& operator<<(std::ostream& stream,
                                const NSCLDAQ11::CRingItem& item);

extern CDataSink& operator<<(CDataSink& stream,
                             const NSCLDAQ11::CRingItem& item);

extern std::istream& operator>>(std::istream& stream,
                                NSCLDAQ11::CRingItem& item);

extern CDataSource& operator>>(CDataSource& stream,
                               NSCLDAQ11::CRingItem& item);

#endif // RINGIO_H
