/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Aaron Chester
         Jeromy Tompkins
	 NSCL
	 Michigan State University
	 East Lansing, MI 48824-1321
*/

#include <math.h>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>

#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#include "DDASHitUnpacker.h"

using namespace std;
using namespace ::DAQ::DDAS;

// A test suite for testing the 250MSPS, 16Bit digitizer output.
// This must be tested separately from the older 250MSPS, 14Bit
// digitizers because the data formats differ slightly in the
// fourth data word.
class DDASUnpacker250MSPS16BitTest : public CppUnit::TestFixture
{
private:
    DDASHit hit;

public:
    CPPUNIT_TEST_SUITE( DDASUnpacker250MSPS16BitTest );
    CPPUNIT_TEST(msps_0);
    CPPUNIT_TEST(rev_0);
    CPPUNIT_TEST(resolution_0);
    CPPUNIT_TEST(coarseTime_0);
    CPPUNIT_TEST(time_0);
    CPPUNIT_TEST(cfdFail_0);
    CPPUNIT_TEST(cfdTrigSource_0);
    CPPUNIT_TEST(traceLength_0);
    CPPUNIT_TEST(trace_0);
    CPPUNIT_TEST_SUITE_END();

public:
    /** 
     * @brief Define and unpack and event. 
     * @details
     * Define an event with no QDC or energy sums information but a very short
     * trace. The real distinction that we want to test is the ability to 
     * extract a proper trace length and overflow/underflow bit.
     */
    void setUp()
	{
	    vector<uint32_t> data = { 0x0000000c, 0x0f1000fa, 0x000a4321,
		0x0000f687, 0xf47f000a, 0x800208be,
		0x45670123};
      
	    DDASHitUnpacker unpacker;
	    tie(hit, ignore) = unpacker.unpack(
		data.data(), data.data()+data.size()
		);
	}

    void tearDown() {}

    /// @name UnpackerTests250MSPS16Bit
    ///@{
    /** 
     * @brief Tests for unpacking module info, time, CFD, etc. for 
     * 16-bit 250 MSPS modules. 
     */    
    void msps_0 ()
	{
	    EQMSG(
		"Simple body extracts ADC frequency",
		uint32_t(250), hit.GetModMSPS()
		); 
	}

    void rev_0 ()
	{
	    EQMSG(
		"Simple body extracts hardware revision",
		15, hit.GetHardwareRevision()
		); 
	}

    void resolution_0 ()
	{
	    EQMSG(
		"Simple body extracts ADC resolution",
		16, hit.GetADCResolution()
		); 
	}

    void coarseTime_0 ()
	{
	    EQMSG(
		"Simple body compute coarse time",
		uint64_t(0x000a0000f687)*8, hit.GetCoarseTime()
		); 
	}

    void time_0 ()
	{
	    ASSERTMSG(
		"Simple body compute time",
		std::abs(hit.GetTime() - 343597888567.2810059) < 0.000001
		); 
	}

    void cfdFail_0 ()
	{
	    EQMSG(
		"Simple body compute CFD fail bit",
		uint32_t(1), hit.GetCFDFailBit()
		);
	}

    void cfdTrigSource_0 ()
	{
	    EQMSG(
		"Simple body compute CFD trig source bit",
		uint32_t(1), hit.GetCFDTrigSource()
		);
	}

    void traceLength_0 ()
	{
	    EQMSG(
		"Bit 31 does not get included in trace length",
		uint32_t(2), hit.GetTraceLength()
		);
	}

    void trace_0()
	{
	    ASSERTMSG(
		"Trace content",
		std::vector<uint16_t>({0x0123, 0x4567}) == hit.GetTrace()
		);
	}

    void overflowUnderflow_0 ()
	{
	    EQMSG(
		"Bit 31 is the overflow underflow bit",
		true, hit.GetADCOverflowUnderflow()
		);
	}
    ///@}
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( DDASUnpacker250MSPS16BitTest );
