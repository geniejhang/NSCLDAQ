/** 
 * @file DDASHit.cpp
 * @brief Implement DDASHit class used to encapsulate DDAS events.
 */

#include "DDASHit.h"

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "DDASBitMasks.h"

/*! 
 * \brief Default constructor
 *
 * All member data are zero-initialized.
 */
DAQ::DDAS::DDASHit::DDASHit() :
    time(0),
    coarsetime(0),
    energy(0),
    timehigh(0),
    timelow(0),
    timecfd(0),
    finishcode(0),
    channellength(0),
    channelheaderlength(0),
    overflowcode(0),
    chanid(0),
    slotid(0),
    crateid(0),
    cfdtrigsourcebit(0),
    cfdfailbit(0),
    tracelength(0),
    ModMSPS(0),
    energySums(),
    qdcSums(),
    trace(),
    externalTimestamp(0),
    m_adcResolution(0),
    m_hdwrRevision(0),
    m_adcOverflowUnderflow(false)
{}

/*! 
 * \brief Resets the state of all member data to that of 
 * initialization
 *
 * For primitive types, this sets the values to 0. For vector 
 * data (i.e. trace), the vector is cleared and resized to 0.
 */
void
DAQ::DDAS::DDASHit::Reset() {
    time = 0;
    coarsetime = 0;
    energy = 0;
    timehigh = 0;
    timelow = 0;
    timecfd = 0;
    finishcode = 0;
    channellength = 0;
    channelheaderlength = 0;
    overflowcode = 0;
    chanid = 0;
    slotid = 0;
    crateid = 0;
    cfdtrigsourcebit = 0;
    cfdfailbit = 0;
    tracelength = 0;
    ModMSPS = 0;

    energySums.clear();
    qdcSums.clear();
    trace.clear();
      
    externalTimestamp = 0;
    m_adcResolution = 0;
    m_hdwrRevision = 0;
    m_adcOverflowUnderflow = false;
}
	
/*! 
 * \brief Destructor  
 */
DAQ::DDAS::DDASHit::~DDASHit()
{}

/**
 * @brief Set the channel ID.
 * @param channel  Channel value for this hit.
 */
void
DAQ::DDAS::DDASHit::setChannel(uint32_t channel)
{
    chanid = channel;
}

/**
 * @brief Set the slot ID.
 * @param slot  Slot value for this hit.
 */
void
DAQ::DDAS::DDASHit::setSlot(uint32_t slot)
{
    slotid = slot;
}

/**
 * @brief Set the crate ID.
 * @param crate  Crate ID value for this hit.
 */
void
DAQ::DDAS::DDASHit::setCrate(uint32_t crate)
{
    crateid = crate;
}

/**
 * @brief Set the channel header length
 * @param channelHeaderLength  Channel header length of this hit.
 */
void
DAQ::DDAS::DDASHit::setChannelHeaderLength(uint32_t channelHeaderLength)
{
    channelheaderlength = channelHeaderLength;
}

/**
 * @brief Set the channel length.
 * @param channelLength  The length of the hit.
 */
void
DAQ::DDAS::DDASHit::setChannelLength(uint32_t channelLength)
{
    channellength = channelLength;
}

/**
 * @brief Set the overflow code.
 *
 * The value overflowcode == 1 indicates the trace has overflowed 
 * the ADC.
 *
 * @param overflowBit  The overflow bit value for this hit.
 */
void
DAQ::DDAS::DDASHit::setOverflowCode(uint32_t overflowBit)
{
    overflowcode = overflowBit;
}

/**
 * @brief Set the finish code.
 * @param finishCode  Finish code for this hit.
 */
void
DAQ::DDAS::DDASHit::setFinishCode(bool finishCode)
{
    finishcode = finishCode;
}

/**
 * @brief Set the coarse timestamp.
 *
 * The coarse timestamp is the leading-edge time in nanoseconds, 
 * without the CFD correction applied.
 * 
 * @param time  The leading-edge time for this hit.
 */
void
DAQ::DDAS::DDASHit::setCoarseTime(uint64_t time)
{
    coarsetime = time;
}

/**
 * @brief Set the raw CFD time.
 * @param data  The raw CFD value from the data word.
 */
void
DAQ::DDAS::DDASHit::setRawCFDTime(uint32_t data)
{
    timecfd = data;
}

/**
 * @brief Set the CFD trigger source bit.
 *
 * The 250 MSPS and 500 MSPS modules de-serialize data into an FPGA 
 * which operates at some fraction of the ADC sampling rate. The CFD 
 * trigger source bit specifies which fractional time offset from the 
 * FPGA clock tick the CFD zero-crossing occured. For 100 MSPS modules,
 * the source bit is always equal to 0 (FPGA captures data also at
 * 100 MSPS).
 *
 * @param bit  The CFD trigger source bit value for this hit.
 */
void
DAQ::DDAS::DDASHit::setCFDTrigSourceBit(uint32_t bit)
{
    cfdtrigsourcebit = bit;
}

/**
 * @brief Set the CFD fail bit.
 *
 * The CFD fail bit == 1 if the CFD algorithm fails. The CFD can fail 
 * if the threshold value is too high or the CFD algorithm fails to 
 * find a zero-crossing point within 32 samples of the leading-edge 
 * trigger point.
 *
 * @param bit  The CFD fail bit value.
 */
void
DAQ::DDAS::DDASHit::setCFDFailBit(uint32_t bit)
{
    cfdfailbit = bit;
}

/**
 * @brief Set the lower 32 bits of the 48-bit timestamp.
 * @param datum  The lower 32 bits of the timestamp.
 */
void
DAQ::DDAS::DDASHit::setTimeLow(uint32_t datum)
{
    timelow = datum;
}

/**
 * @brief Set the higher 16 bits of the 48-bit timestamp.
 * @param datum  The higher 16 bits of the 48-bit timestamp; the lower 
 *   16 bits of the 32-bit word passed to this function.
 */
void
DAQ::DDAS::DDASHit::setTimeHigh(uint32_t datum)
{
    timehigh = datum & 0xffff;
}

/**
 * @brief Set the hit time.
 * @param compTime  The computed time for this hit with the CFD 
 *   correction applied.
 */
void
DAQ::DDAS::DDASHit::setTime(double compTime)
{
    time = compTime;
}

/**
 * @brief Set the energy for this hit.
 * @param value  The energy for this hit.
 */
void
DAQ::DDAS::DDASHit::setEnergy(uint32_t value)
{
    energy = value;
}
	
/**
 * @brief Set the ADC trace length.
 * @param length  The length of the trace in 16-bit words (samples).
 */	
void
DAQ::DDAS::DDASHit::setTraceLength(uint32_t length)
{
    tracelength = length;
}

/**
 * @brief Set the value of the ADC frequency in MSPS for the ADC
 * which recorded this hit.
 * @param value  The ADC frequency in MSPS.
 */
void
DAQ::DDAS::DDASHit::setADCFrequency(uint32_t value)
{
    ModMSPS = value;
}

/**
 * @brief Set the value of the ADC resolution (bit depth) for the ADC
 * which recorded this hit.
 * @param value  The ADC resolution.
 */
void
DAQ::DDAS::DDASHit::setADCResolution(int value)
{
    m_adcResolution = value;
}

/**
 * @brief Set the ADC hardware revision for the ADC which recorded 
 * this hit.
 * @param value  The hardware revision of the ADC.
 */
void
DAQ::DDAS::DDASHit::setHardwareRevision(int value)
{
    m_hdwrRevision = value;
}

/**
 * @brief Set the crate ID.
 * @param value  Crate ID value for this hit.
 */
void
DAQ::DDAS::DDASHit::appendEnergySum(uint32_t value)
{
    energySums.push_back(value);
}

/**
 * @brief Append a QDC value to the vector of QDC sums.
 * @param value  The QDC value appended to the vector.
 */
void
DAQ::DDAS::DDASHit::appendQDCSum(uint32_t value)
{
    qdcSums.push_back(value);
}

/**
 * @brief Append a 16-bit ADC trace sample to the trace vector.
 * @param value  The 16-bit ADC sample appended to the vector.
 */
void
DAQ::DDAS::DDASHit::appendTraceSample(uint16_t value)
{
    trace.push_back(value);
}

/**
 * @brief Set the value of the external timestamp.
 * @param value  The value of the external timestamp supplied to DDAS.
 */
void
DAQ::DDAS::DDASHit::setExternalTimestamp(uint64_t value)
{
    externalTimestamp = value;
}

/**
 * @brief Set ADC OverflowUnderflow
 * @param state  The ADC under-/overflow state. True if the ADC under- 
 *  or overflows the ADC.
 */
void
DAQ::DDAS::DDASHit::setADCOverflowUnderflow(bool state)
{
    m_adcOverflowUnderflow = state;
}

/**
 * @brief Copy in data from another DDASHit.
 * @param rhs  Reference to the DDASHit to copy.
 */
void
DAQ::DDAS::DDASHit::copyIn(const DDASHit& rhs) {
    time = rhs.time;
    coarsetime = rhs.coarsetime;
    energy= rhs.energy;
    timehigh = rhs.timehigh;
    timelow = rhs.timelow;
    timecfd = rhs.timecfd;
    finishcode = rhs.finishcode;
    channellength = rhs.channellength;
    channelheaderlength = rhs.channelheaderlength;
    overflowcode = rhs.overflowcode;
    chanid = rhs.chanid;
    slotid= rhs.slotid;
    crateid = rhs.crateid;
    cfdtrigsourcebit = rhs.cfdtrigsourcebit;
    cfdfailbit = rhs.cfdfailbit;
    tracelength =rhs.tracelength;
    ModMSPS = rhs.ModMSPS;
    energySums = rhs.energySums;
    qdcSums =rhs.qdcSums;
    trace = rhs.trace;
    externalTimestamp= rhs.externalTimestamp;
    m_hdwrRevision = rhs.m_hdwrRevision;
    m_adcResolution= rhs.m_adcResolution;
    m_adcOverflowUnderflow = rhs.m_adcOverflowUnderflow;      
}
