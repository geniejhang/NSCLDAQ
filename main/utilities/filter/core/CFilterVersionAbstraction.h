#ifndef DAQ_CFILTERVERSIONABSTRACTION_H
#define DAQ_CFILTERVERSIONABSTRACTION_H

#include <memory>
#include <cstdint>

namespace DAQ {

class CDataSource;
class CDataSink;
class CFilterMediator;

class CFilterVersionAbstraction;
using CFilterVersionAbstractionUPtr = std::unique_ptr<CFilterVersionAbstraction>;
using CFilterVersionAbstractionPtr  = std::shared_ptr<CFilterVersionAbstraction>;

class CFilterVersionAbstraction {
public:
    virtual ~CFilterVersionAbstraction() {};
    virtual void readDatum(CDataSource& source) = 0;
    virtual void processDatum() = 0;
    virtual void outputDatum(CDataSink&) = 0;
    virtual uint32_t getDatumType() const = 0;
    virtual void cleanUp() = 0;

    virtual void setExcludeList(const std::string& excludeList) = 0;
    virtual void setSampleList(const std::string& sampleList) = 0;

    virtual void initialize() = 0;
    virtual void finalize() = 0;

    virtual void setFilterMediator(CFilterMediator& mediator) = 0;
    virtual CFilterMediator* getFilterMediator() = 0;
};


} // end DAQ


#endif // CFILTERVERSIONABSTRACTION_H
