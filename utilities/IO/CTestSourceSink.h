#ifndef CTESTSOURCESINK_H
#define CTESTSOURCESINK_H

#include "CDataSource.h"
#include "CDataSink.h"
#include <vector>

class CRingItem;

class CTestSourceSink : public CDataSource, public CDataSink
{
  private:
    std::vector<char> m_buffer;

  public:
    CTestSourceSink();
    CTestSourceSink(size_t buffer_size);
    virtual ~CTestSourceSink();

    // A method defining how to send ring items to the sink
    virtual void putItem(const CRingItem& item);

    // A method for putting arbitrary data to a sink:

    virtual void put(const void* pData, size_t nBytes);

    virtual void read(char* pData, size_t nBytes);
    virtual CRingItem* getItem() {return nullptr;}

    const std::vector<char>& getBuffer() const { return m_buffer; };
};

#endif

