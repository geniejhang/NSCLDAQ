/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     Aaron Chester
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <stdint.h>
#include <math.h>

#include <vector>
#include <array>
#include <tuple>

#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#include "DDASHit.h"
#include "DDASHitUnpacker.h"

using namespace std;
using namespace ::DAQ::DDAS;

/** @brief One of the ways we chan check the equality of vectors. */
template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec)
{
  stream << "{ ";
  for (auto& element : vec ) stream << element << " ";
  stream << "}";

  return stream;
}

// A test suite 
class DDASHitTest : public CppUnit::TestFixture
{
private:
    DDASHit hit;

public:
    CPPUNIT_TEST_SUITE( DDASHitTest );
    CPPUNIT_TEST( crateId_0 );
    CPPUNIT_TEST( slotId_0 );
    CPPUNIT_TEST( chanId_0 );
    CPPUNIT_TEST( headerLength_0 );
    CPPUNIT_TEST( eventLength_0 );
    CPPUNIT_TEST( finishCode_0 );
    CPPUNIT_TEST( msps_0 );
    CPPUNIT_TEST( timelow_0 );
    CPPUNIT_TEST( timehigh_0 );
    CPPUNIT_TEST( coarseTime_0 );
    CPPUNIT_TEST( time_0 );
    CPPUNIT_TEST( cfdFail_0 );
    CPPUNIT_TEST( cfdTrigSource_0 );
    CPPUNIT_TEST( energySums_0 );
    CPPUNIT_TEST( qdcSums_0 );
    CPPUNIT_TEST( trace_0 );
    CPPUNIT_TEST( hdwrRevision_0 );
    CPPUNIT_TEST( adcResolution_0 );
    CPPUNIT_TEST( overunderflow_0 );
    CPPUNIT_TEST( constructor_0 );
    CPPUNIT_TEST( reset_0 );
    CPPUNIT_TEST_SUITE_END();

public:
    /** @brief Set a hit data and unpack it. */
    void setUp() {
	vector<uint32_t> data = {
	    0x0000002c, 0x0c0c0064, 0x00290321, 0x0000f687,
	    0x947f000a, 0x000808be, 0x00000001, 0x00000002,
	    0x00000003, 0x00000004, 0x00000005, 0x00000006,
	    0x00000007, 0x00000008, 0x00000009, 0x0000000a,
	    0x0000000b, 0x0000000c, 0x00020001, 0x00040003,
	    0x00060005, 0x00080007
	};
      
	DDASHitUnpacker unpacker;
	unpacker.unpack(data.data(), data.data()+data.size(), hit);
    }

    void tearDown() {}

    void crateId_0 () {
	EQMSG("Simple body extracts crate id", uint32_t(3), hit.GetCrateID()); 
    }
    
    void slotId_0 () {
	EQMSG("Simple body extracts slot id", uint32_t(2), hit.GetSlotID());
    }
    
    void chanId_0 () {
	EQMSG(
	    "Simple body extracts channel id", uint32_t(1), hit.GetChannelID()
	    ); 
    }
    
    void headerLength_0 () {
	EQMSG(
	    "Simple body extracts header length",
	    uint32_t(16), hit.GetChannelLengthHeader()
	    ); 
    }
    
    void eventLength_0 () {
	EQMSG(
	    "Simple body extracts event length",
	    uint32_t(20), hit.GetChannelLength()
	    ); 
    }

    void finishCode_0 () {
	EQMSG(
	    "Simple body extracts finish code",
	      uint32_t(0), hit.GetFinishCode()
	    ); 
    }

    void msps_0 () {
	EQMSG(
	    "Simple body extracts ADC frequency",
	    uint32_t(100), hit.GetModMSPS()
	    ); 
    }

    void timelow_0 () {
	EQMSG(
	    "Simple body extracts time low", uint32_t(63111), hit.GetTimeLow()
	    ); 
    }
    
    void timehigh_0 () {
	EQMSG(
	    "Simple body extracts time high", uint32_t(10), hit.GetTimeHigh()
	    ); 
    }
    
    void coarseTime_0 () {
	EQMSG(
	    "Simple body coarse time",
	    uint64_t(0x000a0000f687*10), hit.GetCoarseTime()
	    ); 
    }
    
    void time_0 () {
	ASSERTMSG(
	    "Simple body full time",
	    std::abs(hit.GetTime()-429497360711.601257) < 0.000001
	    );
    }

    void cfdFail_0 () {
	EQMSG(
	    "Simple body compute CFD fail bit",
	    uint32_t(1), hit.GetCFDFailBit()
	    );
    }

    void cfdTrigSource_0 () {
	EQMSG(
	    "Simple body compute CFD trig source bit",
	    uint32_t(0), hit.GetCFDTrigSource()
	    );
    }

     void energySums_0 () {
     	std::vector<uint32_t> expected = {1, 2, 3, 4};
     	EQMSG("Found all 4 energy sums", expected, hit.GetEnergySums());
     }

    void qdcSums_0 () {
	std::vector<uint32_t> expected = {5, 6, 7, 8, 9, 10, 11, 12};
	EQMSG("Found all 8 QDC sums", expected, hit.GetQDCSums());
    }

    void trace_0 () {
	std::vector<uint16_t> expected = {1, 2, 3, 4, 5, 6, 7, 8};
	EQMSG("Found all trace samples",  expected, hit.GetTrace());
    }

    void hdwrRevision_0() {
        EQMSG("Hardware revision", int(12), hit.GetHardwareRevision());
    }

    void adcResolution_0() {
        EQMSG("ADC resolution", int(12), hit.GetADCResolution());
    }

    void overunderflow_0() {
        EQMSG("ADC overflow/underflow", false, hit.GetADCOverflowUnderflow());
    }
    
    /** @brief Construct and test for zero-initialization. */
    void constructor_0() {
	DDASHit hit;
	testZeroInitialized(hit);
    }

    /** @brief Check that each member is set to zero. */
    void testZeroInitialized(DDASHit& hit) {
	EQMSG("energy",              uint32_t(0), hit.GetEnergy());
	EQMSG("time low",            uint32_t(0), hit.GetTimeLow());
	EQMSG("time high",           uint32_t(0), hit.GetTimeHigh());
	EQMSG("time CFD",            uint32_t(0), hit.GetTimeCFD());
	EQMSG("coarse time",         uint64_t(0), hit.GetCoarseTime());
	EQMSG("computed time",       0.,          hit.GetTime());
	EQMSG("finish code",         uint32_t(0), hit.GetFinishCode());
	EQMSG("channel length",      uint32_t(0), hit.GetChannelLength());
	EQMSG("chan header length",  uint32_t(0), hit.GetChannelLengthHeader());
	EQMSG("slot id",             uint32_t(0), hit.GetSlotID());
	EQMSG("crate id",            uint32_t(0), hit.GetCrateID());
	EQMSG("channel id",          uint32_t(0), hit.GetChannelID());
	EQMSG("mod MSPS",            uint32_t(0), hit.GetModMSPS());
	EQMSG("CFD trig source",     uint32_t(0), hit.GetCFDTrigSource());
	EQMSG("CFD fail bit",        uint32_t(0), hit.GetCFDFailBit());
	EQMSG("trace length",        uint32_t(0), hit.GetTraceLength());
	EQMSG("trace cleared",       size_t(0),   hit.GetTrace().size());
	EQMSG("energy sums cleared", size_t(0),   hit.GetEnergySums().size());
	EQMSG("QDC sums cleared",    size_t(0),   hit.GetQDCSums().size());
    }

    /** @brief Reset should set everything to zero as well. */
    void reset_0 () {
	hit.Reset();
	testZeroInitialized(hit);
    }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(DDASHitTest);
