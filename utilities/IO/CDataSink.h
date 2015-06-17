
#ifndef CDATASINK_H
#define CDATASINK_H

#include <stdlib.h>

class CRingItem;

/**! Interface for CDataSinks
*
* This is a pure virtual base class that establishes an
* expected interface for all data sinks.
*/
class CDataSink
{
    
public:
    
    // The virtual destructor
    virtual ~CDataSink();


    // A method defining how to send ring items to the sink
    virtual void putItem(const CRingItem& item) =0;
    
    // A method for putting arbitrary data to a sink:
    
    virtual void put(const void* pData, size_t nBytes) = 0;

};

#endif
