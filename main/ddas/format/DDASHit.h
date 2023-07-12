/*
  This software is Copyright by the Board of Trustees of Michigan
  State University (c) Copyright 2014.

  You may use this software under the terms of the GNU public license
  (GPL).  The terms of this license are described at:

  http://www.gnu.org/licenses/gpl.txt

  Authors:
  Ron Fox
  Jeromy Tompkins 
  NSCL
  Michigan State University
  East Lansing, MI 48824-1321
*/

/**
 * @file DDASHit.h
 * @brief DDASHit class definition. 
 */

#ifndef DAQ_DDAS_DDASHIT_H
#define DAQ_DDAS_DDASHIT_H

#include <vector>
#include <cstdint>

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {
	/**
	 * @addtogroup format libddasformat.so
	 * @brief DDAS data format library.
	 *
	 * This library contains the DDASHitUnpacker, a class to unpack 
	 * Pixie-16 data recorded by FRIBDAQ into a generic, module-independent
	 * format defined by the DDASHit class. The DDASHit class comes with a
	 * collection of getter and setter functions to access and manipulate 
	 * data.
	 * @{
	 */

	/**
	 * @todo (ASC 7/12/23): Should follow relatively normal FRIBDAQ 
	 * conventions for DDASHit and use m_X to denote member variables.
	 */
	
	/*!
	 * \class DDASHit
	 * 
	 * \brief Encapsulation of a generic DDAS event.
	 *
	 * The DDASHit class is intended to encapsulate the information that
	 * is emitted by the Pixie-16 digitizer for a single event. It contains
	 * information for a single channel only. It is generic because it can
	 * store data for the 100 MSPS, 250 MSPS, and 500 MSPS Pixie-16 
	 * digitizers used at the lab. In general all of these contain the 
	 * same set of information, however, the meaning of the CFD data is 
	 * different for each. The DDASHit class abstracts these differences 
	 * away from the user.
	 *
	 * This class does not provide any parsing capabilities likes its
	 * companion class ddasdumper. To fill this with data, you should use 
	 * the associated DDASHitUnpacker class. Here is how you use it:
	 *
	 * \code
	 * DDASHit channel;
	 * DDASHitUnpacker unpacker;
	 * unpacker.unpack(pData, pData+sizeOfData, channel);
	 * \endcode
	 *
	 * where pData is a pointer to the first word of the event body.
	 */
	class DDASHit { 

	private:
	    /* Channel events always have the following info. */
	    double time;         ///< Assembled time including cfd
	    uint64_t coarsetime; ///< Assembled time without cfd

	    uint32_t energy;   ///< Energy of event
	    uint32_t timehigh; ///< Bits 32-47 of timestamp
	    uint32_t timelow;  ///< Bits 0-31 of timestamp
	    uint32_t timecfd;  ///< Raw cfd time

	    uint32_t finishcode;    ///< Indicates whether pile-up occurred
	    uint32_t channellength; ///< Number of 32-bit words of raw data
	    uint32_t channelheaderlength; ///< Length of header
	    uint32_t overflowcode;  ///< ADC overflow (1 = overflow)
	    uint32_t chanid;        ///< Channel index
	    uint32_t slotid;        ///< Slot index
	    uint32_t crateid;       ///< Crate index

	    uint32_t cfdtrigsourcebit; ///< Value of trigger source bit(s) for 250 MSPS and 500 MSPS
	    uint32_t cfdfailbit;  ///< Indicates whether the cfd algo failed

	    uint32_t tracelength; ///< Length of stored trace

	    uint32_t ModMSPS;     ///< Sampling rate of the module (MSPS)

	    /* A channel may have extra information... */
	    std::vector<uint32_t> energySums; ///< Energy sum data
	    std::vector<uint32_t> qdcSums;    ///< QDC sum data

	    /* A waveform (trace) may be stored too... */
	    std::vector<uint16_t> trace; ///< Trace data

	    uint64_t externalTimestamp;  ///< External timestamp

	    int      m_hdwrRevision;  ///< hardware revision
	    int      m_adcResolution; ///< ADC resolution
	    bool     m_adcOverflowUnderflow; ///< whether the ADC over- or underflowed

	public:
	    DDASHit();
	    
	private:
	    void copyIn(const DDASHit& rhs);
	    
	public:      
	    /*! \brief Copy constructor */
	    DDASHit(const DDASHit& obj) {
		copyIn(obj);
	    }
	    /*! \brief Assignment operator */
	    DDASHit& operator=(const DDASHit& obj) {
		if (this != &obj) {
		    copyIn(obj);
		}
		return *this;
	    }	    
	    virtual ~DDASHit(); 
	    void Reset();
	    
	    /*! 
	     * \brief Retrieve the energy
	     *
	     * With the advent of Pixie-16 modules with 16-bit ADCs, the 
	     * GetEnergy() method no longer includes the ADC 
	     * overflow/underflow bit. The overflow/underflow bit can be 
	     * accessed via the GetADCOverflowUnderflow() method instead.
	     *
	     * \return uint32_t  The energy.
	     */
	    uint32_t GetEnergy() const { return energy; }	    
	    /*! 
	     * \brief Retrieve most significant 16-bits of raw timestamp.
	     * \return uint32_t  The upper 16 bits of the 48-bit timestamp. 
	     */
	    uint32_t GetTimeHigh() const { return timehigh; }	    
	    /*! 
	     * \brief Retrieve least significant 32-bit of raw timestamp .
	     * \return uint32_t  The lower 16 bits of the 48-bit timestamp. 
	     */
	    uint32_t GetTimeLow() const { return timelow; }	    
	    /*!
	     * \brief Retrieve the raw CFD time.
	     * \return uint32_t  The raw CFD time value from the data word. 
	     */
	    uint32_t GetTimeCFD() const { return timecfd; }	    
	    /*! 
	     * \brief Retrieve computed time 
	     *
	     * This method performs a computation that depends on the type of 
	     * the digitizer that produced the data. In each case, the coarse 
	     * timestamp is formed using the timelow and timehigh. This is 
	     * coarse timestamp is then corrected using any CFD time that 
	     * exists.
	     *
	     * The calculations for the various modules are as follows:
	     *
	     * For the 100 MSPS modules:
	     *
	     * \f[\text{time} = 10\times((\text{timehigh} << 32) 
	     * + \text{timelow})\f]
	     *  
	     * For the 250 MSPS modules:
	     *
	     * \f[\text{time} = 8\times((\text{timehigh} << 32) 
	     * + \text{timelow}) + 4\times(\text{timecfd}/2^{14}
	     * - \text{cfdtrigsourcebit})\f]
	     *
	     * For the 500 MSPS modules:
	     *
	     * \f[\text{time} = 10\times((\text{timehigh} << 32) 
	     * + \text{timelow}) + 2\times(\text{timecfd}/2^{13}
	     * + \text{cfdtrigsourcebit} - 1)\f]
	     *
	     * \return double  The timestamp in units of nanoseconds.
	     */
	    double GetTime() const { return time; }	    
	    /*! 
	     * \brief Retrieve the 48-bit timestamp in nanoseconds without 
	     * any CFD correction.
	     * \return uint64_t  The raw 48-bit timestamp in nanoseconds. 
	     */
	    uint64_t GetCoarseTime() const { return coarsetime; }	    
	    /*! 
	     * \brief Retrieve finish code
	     *
	     * The finish code will be set to 1 if pileup was detected.
	     * 
	     * \return uint32_t  The finish code.
	     */
	    uint32_t GetFinishCode() const { return finishcode; }	    
	    /*! 
	     * \brief Retrieve number of 32-bit words that were in original 
	     * data packet.
	     *
	     * Note that this only really makes sense to be used if the object 
	     * was filled with data using UnpackChannelData().
	     *
	     * \return uint32_t  The number of 32-bit words in the event.
	     */
	    uint32_t GetChannelLength() const { return channellength; }	    
	    /*! 
	     * \brief Retrieve length of header in original data packet. 
	     * \return uint32_t  Length of the channel header. 
	     */
	    uint32_t GetChannelLengthHeader()
		const { return channelheaderlength; }	    
	    /*! 
	     * \brief Retrieve the overflow code. 
	     * \return uint32_t  The overflow code. 
	     */
	    uint32_t GetOverflowCode() const { return overflowcode; }	    
	    /*! 
	     * \brief Retrieve the slot that the module resided in. 
	     * \return uint32_t  Module slot. 
	     */
	    uint32_t GetSlotID() const { return slotid; }	    
	    /*! 
	     * \brief Retrieve the index of the crate the module resided in. 
	     * \return uint32_t  Module crate ID. 
	     */
	    uint32_t GetCrateID() const { return crateid; }	    
	    /*! 
	     * \brief Retrieve the channel index. 
	     * \return uint32_t  Channel index on the module. 
	     */
	    uint32_t GetChannelID() const { return chanid; }	    
	    /*! 
	     * \brief Retrieve the ADC frequency of the module. 
	     * \return uint32_t  Module ADC MSPS. 
	     */
	    uint32_t GetModMSPS() const { return ModMSPS; }	    
	    /*! 
	     * \brief Retrieve the hardware revision. 
	     * \return int  Module hardware revision number. 
	     */
	    int GetHardwareRevision() const { return m_hdwrRevision; }	    
	    /*! 
	     * \brief Retrieve the ADC resolution.
	     \return int  Module ADC resolution (bit depth). 
	    */
	    int GetADCResolution() const { return m_adcResolution; }	    
	    /*! 
	     * \brief Retrieve trigger source bit from CFD data. 
	     * \return uint32_t  The CFD trigger source bit.
	     */
	    uint32_t GetCFDTrigSource()
		const { return cfdtrigsourcebit; }
	    /*! 
	     * \brief Retreive failure bit from CFD data.
	     *
	     * The fail bit == 1 if the CFD fails, 0 otherwise.
	     *
	     * \return uint32_t  The CFD fail bit.
	     */
	    uint32_t GetCFDFailBit() const { return cfdfailbit; }	    
	    /*!
	     * \brief Retrieve trace length 
	     * \return uint32_t  The trace length in ADC samples.
	     */
	    uint32_t GetTraceLength() const { return tracelength; }	    
	    /*! 
	     * \brief Access the trace data 
	     * \return std::vector<uint16_t>  The ADC trace.
	     */
	    std::vector<uint16_t>& GetTrace() { return trace; }
	    /*! 
	     * \brief Access the trace data 
	     * \return const std::vector<uint16_t>  The ADC trace.
	     */
	    const std::vector<uint16_t>& GetTrace() const { return trace; }
	    /*! 
	     * \brief Access the energy/baseline sum data.
	     * \return std::vector<uint32_t>  The energy sum data.
	     */
	    std::vector<uint32_t>& GetEnergySums() { return energySums; }
	    /*! 
	     * \brief Access the energy/baseline sum data.
	     * \return const std::vector<uint32_t>  The energy sum data.
	     */
	    const std::vector<uint32_t>& GetEnergySums()
		const { return energySums; }	    
	    /*! 
	     * \brief Access the QDC data.
	     * \return std::vector<uint32_t>  The QDC sum data.
	     */
	    std::vector<uint32_t>& GetQDCSums() { return qdcSums; }	    
	    /*! 
	     * \brief Access the QDC data.
	     * \return const std::vector<uint32_t>  The QDC sum data.
	     */
	    const std::vector<uint32_t>& GetQDCSums() const { return qdcSums; }
	    /*!
	     * \brief Retrieve the external timestamp.
	     * \return uint64_t  The 48-bit external timestamp in nanoseconds.
	     */
	    uint64_t GetExternalTimestamp() const { return externalTimestamp; }
	    /*! \brief Retrieve the ADC overflow/underflow status
	     *
	     * In the 12 and 14 bit modules, this is the value of bit 15 in 
	     * the 4th header word. In the 16 bit modules, this is the value 
	     * of bit 31 in the 4th header word.
	     *
	     * \return bool
	     * \retval true   If the ADC over- or underflows.
	     * \retval false  Otherwise.
	     */
	    bool GetADCOverflowUnderflow()
		const { return m_adcOverflowUnderflow; }
	    
	    void setChannel(uint32_t channel);
	    void setSlot(uint32_t slot);
	    void setCrate(uint32_t crate);
	    void setChannelHeaderLength(uint32_t channelHeaderLength);
	    void setChannelLength(uint32_t channelLength);
	    void setOverflowCode(uint32_t overflow);
	    void setFinishCode(bool finishCode);
	    void setCoarseTime(uint64_t time);
	    void setRawCFDTime(uint32_t data);
	    void setCFDTrigSourceBit(uint32_t bit);
	    void setCFDFailBit(uint32_t bit);
	    void setTimeLow(uint32_t datum);
	    void setTimeHigh(uint32_t datum);
	    void setTime(double time);
	    void setEnergy(uint32_t value);
	    void setTraceLength(uint32_t trace);
	    void setADCFrequency(uint32_t value);
	    void setADCResolution(int value);
	    void setHardwareRevision(int value);
	    void appendEnergySum(uint32_t value);
	    void appendQDCSum(uint32_t value);
	    void appendTraceSample(uint16_t value);
	    void setExternalTimestamp(uint64_t tstamp);
	    void setADCOverflowUnderflow(bool state);
	};

	/** @} */
	
    } // end DDAS namespace
} // end DAQ namespace

#endif
