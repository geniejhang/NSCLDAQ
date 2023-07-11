/**
 * @file DDASHitUnpacker.cpp
 * @brief Implementation of an unpacker for DDAS data recorded by 
 * NSCLDAQ/FRIBDAQ.
 */

#include "DDASHitUnpacker.h"
#include "DDASBitMasks.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace DAQ {
    namespace DDAS {
	
	/**
	 * @brief Unpack data into a DDASHit.
	 *
	 * This expects data from a DDAS readout program. It will parse the 
	 * entire body of the event in a manner that is consistent with the 
	 * data present. In other words, it uses the sizes of the event encoded
	 * in the data to determine when the parsing is complete.
	 *
	 * While it parses, it stores the results into the data members of the 
	 * object hit. Prior to parsing, all data members are reset to 0 using
	 * the Reset() method. 
	 *
	 * @param[in] beg  Pointer to the first 32-bit word of the hit (first 
	 *   non-header word).
	 * @param[in] sentinel  Pointer to the last word of the hit.
	 * @param[in,out]  hit  Reference to the DDASHit object filled during 
	 *   unpacking.
	 *
	 * @throw std::runtime_error  If the hit data buffer is empty.
	 * @throw std::runtime_error  If the hit's length is not the value 
	 *   specified in the header.
	 *
	 * @return uint32_t*  Pointer to the next data word after the hit.
	 */
	const uint32_t*
	DDASHitUnpacker::unpack(
	    const uint32_t* beg, const uint32_t* sentinel, DDASHit& hit
	    )
	{

	    if (beg == sentinel) {
		std::stringstream errmsg;
		errmsg << "DDASHitUnpacker::unpack() ";
		errmsg << "Unable to parse empty data buffer.";
		throw std::runtime_error(errmsg.str());
	    }

	    const uint32_t* data = beg;

	    data = parseBodySize(data, sentinel);
	    data = parseModuleInfo(hit, data);
	    data = parseHeaderWord0(hit, data);
	    data = parseHeaderWords1And2(hit, data);
	    data = parseHeaderWord3(hit, data);

	    // finished upacking the minimum set of data
	    
	    uint32_t channelheaderlength = hit.GetChannelLengthHeader();
	    uint32_t channellength = hit.GetChannelLength();
	    size_t tracelength = hit.GetTraceLength();
	    
	    // more unpacking data
	    
	    if(channellength != (channelheaderlength + tracelength/2)){
		std::stringstream errmsg;
		errmsg << "ERROR: Data corruption: ";
		errmsg << "Inconsistent data lengths found in header ";
		errmsg << "\nChannel length = "
		       << std::setw(8) << channellength;
		errmsg << "\nHeader length  = "
		       << std::setw(8) << channelheaderlength;
		errmsg << "\nTrace length   = "
		       << std::setw(8) << tracelength;
		throw std::runtime_error(errmsg.str());
	    }

	    // if channel header length is 8 then the extra 4 words are energy
	    // sums and baselines
	    if(channelheaderlength == 6) {
		data = extractExternalTimestamp(data, hit);

	    } else if(channelheaderlength == 8) {
		data = extractEnergySums(data, hit);

	    } else if (channelheaderlength == 10) {
		data = extractEnergySums(data, hit);
		data = extractExternalTimestamp(data, hit);

	    } else if(channelheaderlength == 12) {
		data = extractQDC(data, hit);

	    } else if (channelheaderlength == 14) {
		data = extractQDC(data, hit);
		data = extractExternalTimestamp(data, hit);

	    } else if(channelheaderlength == 16) {
		// if channel header length is 16 then the extra 12 words are
		// energy and QDC sums
		data = extractEnergySums(data, hit);
		data = extractQDC(data, hit);

	    } else if(channelheaderlength == 18) {
		// if channel header length is 18 then the extra 12 words are
		// energy and QDC sums
		data = extractEnergySums(data, hit);
		data = extractQDC(data, hit);
		data = extractExternalTimestamp(data, hit);

	    }

	    // if trace length is non zero, retrieve the trace
	    if(tracelength != 0) {
		data = parseTraceData(hit, data);
	    } //finished unpacking trace


	    return data;
	}

	/**
	 * @brief Unpack data into a DDASHit.
	 *
	 * This expects data from a DDAS readout program. It will parse the 
	 * entire body of the event in a manner that is consistent with the 
	 * data present. In other words, it uses the sizes of the event encoded
	 * in the data to determine when the parsing is complete.
	 *
	 * While it parses, it stores the results into the data members of the 
	 * object hit. Prior to parsing, all data members are reset to 0 using
	 * the Reset() method. 
	 *
	 * @param beg  Pointer to the first word of the hit (first 
	 *   non-header word).
	 * @param sentinel  Pointer to the last word of the hit.
	 *
	 * @return std::tuple<DDASHit, const uint32_t*>  The first element is 
	 *   a filled DDASHit, the second element a pointer to the next data 
	 *   word after the hit.
	 */
	tuple<DDASHit, const uint32_t*>
	DDASHitUnpacker::unpack(const uint32_t *beg, const uint32_t* sentinel)
	{
	    DDASHit hit;
	    const uint32_t* data = unpack(beg, sentinel, hit);
	    return make_tuple(hit, data);
	}

	/**
	 * @brief Unpack the trace data.
	 *
	 * The 16-bit trace data is stored two samples to one 32-bit word in
	 * little-endian. The data for sample i is stored in the lower 16 bits
	 * while the data for sample i+1 is stored in the upper 16 bits. For 
	 * ADCs with less than 16-bit resolution, those bits are set to 0.
	 *
	 * @param hit  References the hit we are unpacking.
	 * @param data  Pointer to the 32-bit trace word to unpack.
	 *
	 * @return const uint32_t*  Pointer to the next 32-bit word.
	 */
	const uint32_t* DDASHitUnpacker::parseTraceData(
	    DDASHit& hit, const uint32_t* data
	    )
	{
	    vector<uint16_t>& trace = hit.GetTrace();
	    size_t tracelength = hit.GetTraceLength();
	    trace.reserve(tracelength);
	    for(size_t z = 0; z < tracelength/2; z++){
		uint32_t datum = *data++;
		trace.push_back(datum & LOWER16BITMASK);
		trace.push_back((datum & UPPER16BITMASK)>>16);
	    }

	    return data;
	}

	/**
	 * @brief Ensure there is enough data to parse.
	 *
	 * @param data  Pointer to the first data word of the hit.
	 * @param sentinel  Pointer to the last data word of the hit.
	 * 
	 * @throws std::runtime_error  If there are an insufficient number of 
	 *   16-bit data words in the event or if the pointer to the last data
	 *   word is nullptr.
	 *
	 * @return uint32_t*  Pointer to the next data word. 
	 */
	const uint32_t*
	DDASHitUnpacker::parseBodySize(
	    const uint32_t* data, const uint32_t* sentinel
	    )
	{
	    uint32_t nShorts = *data; 

	    // make sure there is enough data to parse
	    if (
		(data + nShorts/sizeof(uint16_t) > sentinel)
		&& (sentinel != nullptr)
		) {
		throw std::runtime_error(
		    "DDASHitUnpacker::unpack() Incomplete event data."
		    );
	    }

	    return (data+1);
	}

	/**
	 * @brief Parse the module identifying information encoded in the hit.
	 *
	 * @param hit  References the DDASHit we are unpacking.
	 * @param data  The 32-bit data word containing the module identifying
	 *   information.
	 *
	 * @return uint32_t*  Pointer to the next data word.
	 */
	const uint32_t*
	DDASHitUnpacker::parseModuleInfo(
	    DDASHit& hit, const uint32_t* data
	    )
	{
	    uint32_t datum = *data++;

	    //next word is the module revision, adc bit, and msps
	    hit.setADCFrequency(datum & LOWER16BITMASK);
	    hit.setADCResolution((datum>>16) & 0xff);
	    hit.setHardwareRevision((datum>>24) & 0xff);

	    return data;
	}

	/**
	 * @brief Parse the word 0 of the Pixie-16 data header.
	 *
	 * Word 0 contains:
	 * - Crate/slot/channel information.
	 * - The header and channel lengths in 32-bit words.
	 * - The ADC overflow code.
	 * - The module finish code (equals 1 if piled up).
	 *
	 * @param hit  References the DDASHit we are unpacking.
	 * @param data  The 32-bit data word to parse.
	 *
	 * @return uint32_t*  Pointer to the next data word (word 1).
	 */
	const uint32_t*
	DDASHitUnpacker::parseHeaderWord0(DDASHit& hit, const uint32_t* data)
	{
	    uint32_t datum = *data++;
	    // Using the first word of DDAS information extract channel
	    // identifiers
	    hit.setChannel(datum & CHANNELIDMASK);
	    hit.setSlot((datum & SLOTIDMASK) >> 4);
	    hit.setCrate((datum & CRATEIDMASK) >> 8);
	    hit.setChannelHeaderLength((datum & HEADERLENGTHMASK) >> 12 );
	    hit.setChannelLength((datum & CHANNELLENGTHMASK) >> 17);
	    hit.setOverflowCode((datum & OVERFLOWMASK) >> 30);
	    hit.setFinishCode((datum & FINISHCODEMASK) >> 31 );
      
	    return data;
	}

	/**
	 * @brief Parse words 1 and 2 of the Pixie-16 data header.
	 *
	 * Words 1 and 2 contain the timestamp and CFD information. The meaning
	 * of the CFD word depends on the module type. The unpacker abstracts 
	 * this meaning away from the user. Note that we know the module type 
	 * if the module identifier word was unpacked before calling this 
	 * function.
	 *
	 * Word 1 contains:
	 * - The lower 32 bits of the 48-bit timestamp.
	 * Word 2 contains:
	 * - The upper 16 bits of the 48-bit timestamp.
	 * - The CFD result.
	 *
	 * @param hit  References the DDASHit we are unpacking.
	 * @param data  Pointer to word 1.
	 *
	 * @return uint32_t*  Pointer to word 3.
	 */
	const uint32_t*
	DDASHitUnpacker::parseHeaderWords1And2(
	    DDASHit& hit, const uint32_t* data
	    )
	{
	    uint32_t timelow      = *data++;
	    uint32_t datum1       = *data++;
	    uint32_t timehigh     = datum1&0xffff;
	    uint32_t adcFrequency = hit.GetModMSPS();

	    double   cfdCorrection;
	    uint32_t cfdtrigsource, cfdfailbit, timecfd;
	    uint64_t coarseTime;

	    coarseTime = computeCoarseTime(adcFrequency, timelow, timehigh) ;
//      tie(cfdCorrection, timecfd, cfdtrigsource, cfdfailbit)
//                                 = parseAndComputeCFD(adcFrequency, datum1);
	    cfdCorrection = parseAndComputeCFD(hit, datum1);

	    hit.setTimeLow(timelow);
	    hit.setTimeHigh(timehigh);
	    hit.setCoarseTime( coarseTime ); 
//      hit.setRawCFDTime(timecfd);
//      hit.setCFDTrigSourceBit( cfdtrigsource );
//      hit.setCFDFailBit( cfdfailbit );
	    hit.setTime(static_cast<double>(coarseTime) + cfdCorrection);

	    return data;
	}

	/**
	 * @brief Parse word 3 of the Pixie-16 data header.
	 *
	 * Word 3 contains:
	 * - The ADC trace overflow flag.
	 * - The trace length in samples (16-bit words).
	 * - The hit energy.
	 *
	 * @param hit  References the DDASHit we are unpacking.
	 * @param data  Pointer to word 1.
	 *
	 * @return uint32_t*  Pointer to the first word of the Pixie-16 
	 *   data body.
	 */
	const uint32_t*
	DDASHitUnpacker::parseHeaderWord3(DDASHit& hit, const uint32_t* data)
	{

	    //if (hit.GetADCResolution()==16 && hit.GetModMSPS()==250) {
	    if (true) {
		hit.setTraceLength((*data >> 16) & 0x7fff);
		hit.setADCOverflowUnderflow(*data >> 31);
		hit.setEnergy(*data & LOWER16BITMASK);
	    } else {
		hit.setTraceLength((*data & UPPER16BITMASK) >> 16);
		hit.setADCOverflowUnderflow((*data >> 15) & 0x1);
		hit.setEnergy((*data & 0x7fff));
	    }

	    return (data+1);
	}

	/**
	 * @brief Determine the CFD correction to the leading-edge time in 
	 * nanoseconds from the CFD word.
	 *
	 * The value of the CFD correction depends on the module. Because the 
	 * module information is encoded in the data, this function should be 
	 * called after parseModuleInfo().
	 *
	 * @param ModMSPS  The module ADC frequency in MSPS.
	 * @param data  The 32-bit data word encoding the CFD information.
	 *
	 * @return std::tuple<double, uint32_t, uint32_t, uint32_t>  (CFD 
	 *   correction in nanoseconds, value of the CFD encoded in the data, 
	 *   CFD trigger source bit, CFD fail bit).
	 */
	tuple<double, uint32_t, uint32_t, uint32_t>
	DDASHitUnpacker::parseAndComputeCFD(uint32_t ModMSPS, uint32_t data)
	{

	    double correction;
	    uint32_t cfdtrigsource, cfdfailbit, timecfd;

	    // check on the module MSPS and pick the correct cfd unpacking
	    // algorithm 
	    if(ModMSPS == 100){
		// 100 MSPS modules don't have trigger source bits
		cfdfailbit    = ((data & BIT31MASK) >> 31) ; 
		cfdtrigsource = 0;
		timecfd       = ((data & BIT30to16MASK) >> 16);
		correction    = (timecfd/32768.0) * 10.0; // 32768 = 2^15
	    }
	    else if (ModMSPS == 250) {
		// cfd fail bit in bit 31
		cfdfailbit    = ((data & BIT31MASK) >> 31 );
		cfdtrigsource = ((data & BIT30MASK) >> 30 );
		timecfd       = ((data & BIT29to16MASK) >> 16);
		correction    = (timecfd/16384.0 - cfdtrigsource)*4.0; 
	    }
	    else if (ModMSPS == 500) {
		// no fail bit in 500 MSPS modules

		cfdtrigsource = ((data & BIT31to29MASK) >> 29 );
		timecfd       = ((data & BIT28to16MASK) >> 16);
		correction    = (timecfd/8192.0 + cfdtrigsource - 1)*2.0;
		cfdfailbit    = (cfdtrigsource == 7) ? 1 : 0;
	    }

	    return make_tuple(correction, timecfd, cfdtrigsource, cfdfailbit);

	}

	/**
	 * @brief Determine the CFD correction to the leading-edge time in 
	 * nanoseconds from the CFD word.
	 *
	 * The value of the CFD correction depends on the module. Because the 
	 * module information is encoded in the data, this function should be 
	 * called after parseModuleInfo().
	 *
	 * @param hit  References the DDASHit we are unpacking.
	 * @param data  The 32-bit data word encoding the CFD information.
	 *
	 * @return double  The CFD correction in nanoseconds.
	 */
	double
	DDASHitUnpacker::parseAndComputeCFD(DDASHit& hit, uint32_t data)
	{

	    double correction;
	    uint32_t cfdtrigsource, cfdfailbit, timecfd;
	    uint32_t ModMSPS = hit.GetModMSPS();

	    // check on the module MSPS and pick the correct cfd unpacking
	    // algorithm 
	    if(ModMSPS == 100){
		// 100 MSPS modules don't have trigger source bits
		cfdfailbit    = ((data & BIT31MASK) >> 31) ; 
		cfdtrigsource = 0;
		timecfd       = ((data & BIT30to16MASK) >> 16);
		correction    = (timecfd/32768.0) * 10.0; // 32768 = 2^15
	    }
	    else if (ModMSPS == 250) {
		// cfd fail bit in bit 31
		cfdfailbit    = ((data & BIT31MASK) >> 31 );
		cfdtrigsource = ((data & BIT30MASK) >> 30 );
		timecfd       = ((data & BIT29to16MASK) >> 16);
		correction    = (timecfd/16384.0 - cfdtrigsource)*4.0; 
	    }
	    else if (ModMSPS == 500) {
		// no fail bit in 500 MSPS modules
		cfdtrigsource = ((data & BIT31to29MASK) >> 29 );
		timecfd       = ((data & BIT28to16MASK) >> 16);
		correction    = (timecfd/8192.0 + cfdtrigsource - 1)*2.0;
		cfdfailbit    = (cfdtrigsource == 7) ? 1 : 0;
	    }

	    hit.setCFDFailBit(cfdfailbit);
	    hit.setCFDTrigSourceBit(cfdtrigsource);
	    hit.setRawCFDTime(timecfd);

	    return correction;
	}

	    /*! @brief Compute time in nanoseconds from raw data (no CFD 
	     * correction).
	     *
	     * This method is very similar to the SetTime() method. It differs 
	     * in that it does not apply a correction for the CFD time. It 
	     * simply forms the timestamp from the low and high bits and then 
	     * converts it to a time in nanoseconds.
	     *
	     * The calculations for the various modules are as follows:
	     *
	     * For the 100 MSPS module:
	     *
	     * \f[\text{time} = 10\times((\text{timehigh} << 32) 
	     * + \text{timelow})\f]
	     *  
	     * For the 250 MSPS module...
	     *
	     * \f[\text{time} = 8\times((\text{timehigh} << 32) 
	     * + \text{timelow})\f]
	     *
	     * For the 500 MSPS module,
	     *
	     * \f[\text{time} = 10\times((\text{timehigh} << 32) 
	     * + \text{timelow})\f]
	     *
	     * @param adcFrequency  Module ADC frequency in MSPS.
	     * @param timelow  Data word containing the lower 32 bits of the 
	     *   48-bit timestamp.
	     * @param timehigh  Data word containing the upper 16 bits of the 
	     *   48-bit timestamp. 
	     * 
	     * @return uint64_t  The 48-bit coarse timestamp in nanoseconds.
	     */
	uint64_t
	DDASHitUnpacker::computeCoarseTime(
	    uint32_t adcFrequency, uint32_t timelow, uint32_t timehigh
	    )
	{
      
	    uint64_t toNanoseconds = 1;

	    if(adcFrequency == 100){
		toNanoseconds = 10;
	    }
	    else if (adcFrequency == 250) {
		toNanoseconds = 8;
	    }
	    else if (adcFrequency == 500) {
		toNanoseconds = 10;
	    }

	    uint64_t tstamp = timehigh;
	    tstamp = tstamp << 32;
	    tstamp |= timelow;

	    return tstamp*toNanoseconds;
	}

	/**
	 * @brief Unpack energy sums.
	 *
	 * Energy sums consist of 4 32-bit words, which are, in order:
	 * 0. The trailing (pre-gap ) sum.
	 * 1. The gap sum.
	 * 2. The leading (post-gap) sum.
	 * 3. The 32-bit IEEE 754 floating point baseline value.
	 *
	 * If the hit is not reset between calls to this function, the energy 
	 * sum data will be appended to the end of the exisiting energy sums.
	 *
	 * @param data  Pointer to the first 32-bit word containing the energy 
	 *   sum data.
	 * @param hit  References the DDASHit we are unpacking.
	 *
	 * @return  uint32_t*  Pointer to the word after the energy sums.
	 */
	const uint32_t*
	DDASHitUnpacker::extractEnergySums(const uint32_t* data, DDASHit& hit)
	{
	    vector<uint32_t>& energies = hit.GetEnergySums();
	    energies.reserve(4);
	    energies.insert(energies.end(), data, data+4);
	    return data + 4;
	}

	/**
	 * @brief Unpack QDC values.
	 *
	 * QDC sums consist of 8 32-bit words. If the hit is not reset between
	 * calls to this function, the QDC sum data will be appended to the 
	 * end of the exisiting QDC sums.
	 *
	 * @param data  Pointer to the first 32-bit word containing the QDC 
	 *   sum data.
	 * @param hit  References the DDASHit we are unpacking.
	 *
	 * @return  uint32_t*  Pointer to the word after the QDC sums.
	 */
	const uint32_t*
	DDASHitUnpacker::extractQDC(const uint32_t* data, DDASHit& hit)
	{
	    vector<uint32_t>& qdcVals = hit.GetQDCSums();
	    qdcVals.reserve(8);
	    qdcVals.insert(qdcVals.end(), data, data+8);
	    return data + 8;
	}

	/**
	 * @brief Unpack the external timestamp data.
	 *
	 * Unpack and set the external timestamp. The external timestamp 
	 * supplied to DDAS can be up to 64 bits long.
	 *
	 * @param data  Pointer to the 32-bit word containing the lower 16 
	 *   bits of the 64-bit external timestamp.
	 * @param hit  References the DDASHit we are unpacking.
	 *
	 * @return  uint32_t*  Pointer to the word after the external 
	 *   timestamp.
	 */
	const uint32_t* 
	DDASHitUnpacker::extractExternalTimestamp(
	    const uint32_t* data, DDASHit& hit
	    ) 
	{
	    uint64_t tstamp = 0;
	    uint32_t temp = *data++;
	    tstamp = *data++;
	    tstamp = ((tstamp << 32) | temp); 
	    hit.setExternalTimestamp(tstamp);
	    return data;
	}


    }  // end DDAS namespace
} // end DAQ namespace
