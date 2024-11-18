/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright MADCDELAY5.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef SIS3316REGISTERS_H
#define SIS3316REGISTERS_H
/**
 *  @file sis3316Registers.h
 *  @brief Register definitions and AM's for the SIS3316.
 */

#include <stdint.h>

// let's get in the habit of using namespaces:

namespace SIS3316 {
    namespace Registers {


const int AMSINBLE=0x0d;   // Single shot operations in Supervisory data.
const int AMBLOCK=0x0f;    // Block modes in A32 block transfer mode.


// FPGA interface registers - as a struct.

static const uint32_t FPGAOFFSET(0);    // Offset to this struct:
#pragma pack(push, 1)
struct FpgaRegisters {
    uint32_t  s_csr;
    uint32_t  s_fwid;
    uint32_t  s_irqconfig;
    uint32_t  s_irqcontrol;
    
    uint32_t  s_arbitrationcsr;
    uint32_t  s_cbltsetup;
    uint32_t  s_internalTest;
    uint32_t  s_hwversion;
    
    uint32_t  s_temperature;
    uint32_t  s_wire1eepromcsr;
    uint32_t  s_serialno;
    uint32_t  s_internalxfrspeed;
    
    uint32_t  s_adcfpgabootcontroller;
    uint32_t  s_spiflashcsr;
    uint32_t  s_spiflashdata;
    uint32_t  s_externalvetogdr;
    
    uint32_t  s_adcclocki2c;
    uint32_t  s_mgt1clocki2c;
    uint32_t  s_mgt2clocki2c;
    uint32_t  s_ddr3clocki2c;

    uint32_t  s_adcclockdstcontrol;
    uint32_t  s_nimclockmult;
    uint32_t  s_fpbuscontrol;
    uint32_t  s_nimincsr;

    uint32_t  s_acqcsr;
    uint32_t  s_coinclutcsr;
    uint32_t  s_coinclutaddr;
    uint32_t  s_coinclutdata;

    uint32_t  s_lemocoselect;
    uint32_t  s_lemotoselect;
    uint32_t  s_lemouoselect;
    uint32_t  s_trfeedbackselect;

    uint32_t  s_adc1234dataxferctl;
    uint32_t  s_adc5678dataxferctl;
    uint32_t  s_adc9abcdataxferctl;
    uint32_t  s_adcdefgdataxferctl;   // g is what you get for numbering from 1 SIS.

    uint32_t  s_adc1234dataxfersr;
    uint32_t  s_adc5678dataxfersr;
    uint32_t  s_adc9abcdataxfersr;
    uint32_t  s_adcdefgdataxfersr;   // g is what you get for numbering from 1 SIS.

    uint32_t  s_vmeadcfpgadlinkstatus;
    uint32_t  s_adcfpgaspibusystatus;
    uint32_t  s_reserved1;
    uint32_t  s_reserved2;

    uint32_t  s_reserved3;
    uint32_t  s_reserved4;
    uint32_t  s_prescalerdivider;
    uint32_t  s_prescalerlength;

    uint32_t  s_channelTriggerCounts[16];
    
    
}

// The key registers. Key registers are SIS-speak for registers that 
// do something if you write to them.. no matter what's written.

static cons uint32_t  KEYOFFSETS(0x400);       // Offset to this struct:

struct keyRegisters {
    uint32_t s_registerReset;
    uint32_t s_userFunction;
    uint32_t s_reserved1;
    uint32_t s_reserved2;

    uint32_t s_armSampleLogic;
    uint32_t s_disarmSampleLogic;
    uint32_t s_trigger;
    uint32_t s_clearTimestamp;

    uint32_t s_armBank1;    // Any armed back is
    uint32_t s_armBank2;    // disarmed by this.
    uint32_t s_enableNIMBankSwap;
    uint32_t s_disablePescaleOutputDivider;

    uint32_t s_PPSLatchClear;
    uint32_t s_logicReset;
    uint32_t s_adcClockPLLReset;
    uint32_t s_reserved3;
}

// Each bank of 4 ADCs  has two register sets:  THe ADC FPGA registers and the 
// memory data FIFO.  The latter are just data soup but the former have structure:

// FPGA Register bases (we number from 0 rather than 1):

static const FPGABASES[4] = {0x1000, 0x2000, 0x3000, 0x4000};
static const FIFOBASES[4] = {0x1000000, 0x2000000, 0x3000000, 0x4000000};

// there are a few peradc. registers:

strcut adcRegisters {
    uint32_t s_firtrgsetup_a;
    uint32_t s_threshold_a;
    uint32_t s_hethreshold_a;
    uint32_t s_reserved_1;
}

struct adcFPGARegisters {
    uint32_t s_inputTapDelay;
    uint32_t s_gainTerminationcontrol;
    uint32_t s_DCOffset;
    uint32_t s_SPIControl;
    
    uint32_t s_eventCOnfig;
    uint32_t s_channelHeaderId;
    uint32_t s_endThreshold;
    uint32_t s_trigerGateLenght;

    uint32_t s_dataconfig;
    uint32_t s_pupconfig;
    uint32_t s_pretrigger;
    uint32_t s_averageconfig;

    uint32_t s_format;
    uint32_t s_MAWtestconfig;
    uint32_t s_internalTrigDelay;
    uint32_t s_internalGateLength;

    adcRegisters s_adcSetup[4];

    uint32_t s_trgstatmode;
    uint32_t s_peakchargeconfig;
    uint32_t s_extendedbufferconfig;
    uint32_t s_extendedEventconfig;

    uint32_t s_accumulatorGateconfig[8];

    uint32_t s_FIRenergySetup[4];
    uint32_t s_histogramSEtup[4];
    uint32_t s_MAWStartIndexConfig[4];

    uint32_t s_test;
    uint32_t s_unused[3];

    uint32_t s_adfpgaVersion;
    uint32_t s_adcfpgastatus;
    uint32_t s_offsetreadback;
    uint32_t s_spireadback;

    uint32_t s_sampleAddress[4];
    uint32_t s_prevbankSampel[4];
    uint32_t s_ppsTimestampHigh;
    uint32_t s_ppsTimestampLow;

    uint32_t s_testReadback1018;
    uint32_t s_testReadback101c;
    uint32_t s_sisinternaltest;


}

#pragma pack (pop)


    }
}

#endif