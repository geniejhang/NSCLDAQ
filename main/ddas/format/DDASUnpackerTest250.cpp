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

class DDASUnpacker250Test : public CppUnit::TestFixture
{
private:
    DDASHit hit;

public:
    CPPUNIT_TEST_SUITE(DDASUnpacker250Test);
    CPPUNIT_TEST(msps_0);
    CPPUNIT_TEST(revision_0);
    CPPUNIT_TEST(resolution_0);
    CPPUNIT_TEST(coarseTime_0);
    CPPUNIT_TEST(time_0);
    CPPUNIT_TEST(cfdFail_0);
    CPPUNIT_TEST(cfdTrigSource_0);
    CPPUNIT_TEST_SUITE_END();

public:
    /** @brief Define and unpack and event. */
    void setUp()
	{
	    vector<uint32_t> data = { 0x0000000c, 0x0f0e00fa, 0x00084321,
		0x0000f687, 0xf47f000a, 0x000008be};
	    DDASHitUnpacker unpacker;
	    tie(hit, ignore) = unpacker.unpack(data.data(), data.data()+data.size());
	}
    
    void tearDown() {}

    /// @name UnpackerTests250MSPS
    ///@{
    /** 
     * @brief Tests for unpacking module info, time, CFD, etc. for 
     * 14-bit 250 MSPS modules. 
     */
    void msps_0 ()
	{
	    EQMSG(
		"Simple body extracts adc frequency",
		uint32_t(250), hit.GetModMSPS()
		); 
	}

    void revision_0 ()
	{
	    EQMSG(
		"Simple body extracts hdwr revision",
		15, hit.GetHardwareRevision()
		);
	}

    void resolution_0 ()
	{
	    EQMSG(
		"Simple body extracts adc resolution",
		14, hit.GetADCResolution()
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
		"Simple body compute cfd fail bit",
		uint32_t(1), hit.GetCFDFailBit()
		);
	}

    void cfdTrigSource_0 ()
	{
	    EQMSG(
		"Simple body compute cfd trig source bit",
		uint32_t(1), hit.GetCFDTrigSource()
		);
	}
    ///@}
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( DDASUnpacker250Test );
