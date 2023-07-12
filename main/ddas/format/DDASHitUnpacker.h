/**
 * @file DDASHitUnpacker.h
 * @brief Define an unpacker for DDAS data recorded by NSCLDAQ/FRIBDAQ.
 */

#ifndef DAQ_DDAS_DDASHITUNPACKER_H
#define DAQ_DDAS_DDASHITUNPACKER_H

#include "DDASHit.h"

#include <vector>
#include <cstdint>
#include <tuple>

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {
	/**
	 * @addtogroup format libddasformat.so
	 * @{
	 */
	
	/** 
	 * @class DDASHitUnpacker
	 *
	 * @brief Unpacker for DDAS data recorded by NSCLDAQ/FRIBDAQ.
	 *
	 * This class unpacks NSCLDAQ-formatted Pixie-16 data recorded by a
	 * DDAS readout program into DDASHits which encapsulate the information
	 * recorded by a single DDAS channel. This is a generic unpacker which 
	 * can accomodate information from all Pixie-16 digitizer types at 
	 * FRIB. In general, all of the digitizer outputs contain the same 
	 * information but importantly the meaning of the CFD data depends on 
	 * the digitizer type. The unpacker class abstracts this difference 
	 * away from the user. 
	 *
	 * This class does not provide any parsing capabilities likes its
	 * companion class ddasdumper. To fill this with data, you should use 
	 * the associated DDASHit class. Here is how you use it:
	 *
	 * \code
	 * DDASHit hit;
	 * DDASHitUnpacker unpacker;
	 * unpacker.unpack(pData, pData+sizeOfData, hit);
	 * \endcode
	 *
	 * where pData is a pointer to the first word of the event body.
	 */
	class DDASHitUnpacker {
	public:
	    std::tuple<DDASHit, const uint32_t*> unpack(
		const uint32_t* beg, const uint32_t* sentinel
		); 
	    const uint32_t* unpack(
		const uint32_t* beg, const uint32_t* sentinel, DDASHit& hit
		); 

	protected:
	    const uint32_t* parseBodySize(
		const uint32_t* beg, const uint32_t* sentinel
		);
	    const uint32_t* parseModuleInfo(DDASHit& hit, const uint32_t* beg);
	    const uint32_t* parseHeaderWord0(DDASHit& hit, const uint32_t* beg);
	    const uint32_t* parseHeaderWords1And2(
		DDASHit& hit, const uint32_t* beg
		);
	    const uint32_t* parseHeaderWord3(DDASHit& hit, const uint32_t* beg);
	    const uint32_t* parseTraceData(DDASHit& hit, const uint32_t* beg);
	    std::tuple<double, uint32_t, uint32_t, uint32_t>
	    parseAndComputeCFD(uint32_t ModMSPS, uint32_t data);
	    double parseAndComputeCFD(DDASHit& hit, uint32_t data);

	    /*! 
	     * \brief Retrieve and set the computed time 
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
	    void SetTime(DDASHit& hit);
	    /*! \brief Append energy sum to stored energy sums.
	     *
	     * This can be called many times and will cause each successive 
	     * value to be appended to object.
	     *
	     * \param hit  References the DDASHit we are unpacking.
	     * \param data  Energy sum to append.
	     */
	    void SetEnergySums(DDASHit& hit, uint32_t data);

	    /*! \brief Append QDC sum to stored QDC sums.
	     *
	     * Like the SetEnergySums() method, this can be called repeatedly 
	     * to append multiple values to the existing data.
	     *
	     * \param hit  References the DDASHit we are unpacking.
	     * \param data  Energy sum to store.
	     */
	    void SetQDCSums(DDASHit& hit, uint32_t data);
	    uint64_t computeCoarseTime(
		uint32_t adcFrequency, uint32_t timelow, uint32_t timehigh
		);
	    const uint32_t* extractEnergySums(
		const uint32_t* data, DDASHit& hit
		);
	    const uint32_t* extractQDC(
		const uint32_t* data, DDASHit& hit
		);
	    const uint32_t* extractExternalTimestamp(
		const uint32_t* data, DDASHit& hit
		);
	};

	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif

