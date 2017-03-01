#ifndef CFILTERVERSIONABSTRACTION_H
#define CFILTERVERSIONABSTRACTION_H

namespace DAQ {

class CFilterVersionAbstraction {
public:
    virtual CFilterVersionAbstraction();
    virtual void readDatum(CDataSource& source) = 0;
    virtual void processDatum() = 0;
    virtual void outputDatum(CDataSink&) = 0;
    virtual uint32_t getDatumType() const = 0;
    virtual void cleanup() = 0;
};


} // end DAQ


#endif // CFILTERVERSIONABSTRACTION_H
