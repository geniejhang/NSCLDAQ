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
    uint32_t  s_bcstsetup;
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

struct adcRegisters {
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
// Below are bit defintions for registers described above.

// The CSR is a funky register.  Rather than setting bits to 
// set things; the top 16 bits clear operations and the bottom
// 32 bits set operations.  The bottom 32 bits read state.
// e.g bit 32 clears the operation of restarting the FFPGAs
// bit 16 starts rebooting the FPGAS and reads whether the FPGAs 
// are being rebooted.  Don't blame me, blame STRUCK:
// Blame me for the lazyness of not provideing e.g. CSR_READ_FPGA_BOOT
// use the set bit as documented above.
static const uint32_t CSR_CLEAR_FPGA_BOOT(0x80000000);
static const uint32_t CSR_SET_FPGA_BOOT(0x00008000);

static const uint32_t CSR_CLEAR_LED2_APPMODE (0x00400000);
static const uint32_t CSR_SET_LED2_APPMODE   (0x00000040);
static const uint32_t CSR_CLEAR_LED1_APPMODE (0x00200000);
static const uint32_t CSR_SET_LED1_APPMODE   (0x00000020);   // LED application mode.
static const uint32_t CSR_CLEAR_LEDU_APPMODE (0x00100000);
static const uint32_t CSR_SET_LEDU_APPMODE   (0x00000010);
static const uint32_t CSR_CLEAR_LED_APPMODE  (0x00080000);
static const uint32_t CSR_SET_LED_APPMODE    (0x00000008);

static const uint32_t CSR_CLEAR_LED2 (0x00040000);
static const uint32_t CSR_SET_LED2   (0x00000004);
static const uint32_t CSR_CLEAR_LED1 (0x00020000);
static const uint32_t CSR_CLEAR_LEDU (0x00010000);
static const uint32_t CSR_SET_LEDU   (0x00000001);

// The firmware register include ths model number
// as well as the firmware major and minor revision numbers.
// The major revision actually is the functionality of the
// firmware.

static const uint32_t FWID_MODULEID_MASK(0XFFFF0000);
static const uint32_t FWID_MODULEID_VALUE(0x33160000);   // reg and mask should give this.
static const uint32_t FWID_MAJOR_MASK(0x0000ff00);
static const uint32_t FWID_MINOR_MASK(0x000000ff);

// Possible major firware values after anding with the mask:

static const uint32_t FWID_STDNGAMMA(0X00002000);   // For sis 3316
static const uint32_t FWID_STDNGAMMA_2(0x00004000);  // For sis 3316-2.

// Interrupt configuration register bits... Note that these are plain old
// bit fields.  For each we provide a mask and shift.
// to read register & mask >> shift gives a value.
// to write:  (value <<shift) | (register & ~mask).

static const uint32_t IRQCFG_ROAK_MASK(0x1000);
static const uint32_t IRQCFG_ROAK_SHIFT(12);
static const uint32_t IRQCFG_ENA_MASK(0X800);
static const uint32_t IRQCFG_ENA_SHIFT(11);
static const uint32_t IRQCFG_IPL_MASK(0x700);
static const uint32_t IRQCFG_IPL_SHIFT(8);
static const uint32_t IRQCFG_VECTOR_MASK(0x00ff);
static const uint32_t IRQCFG_VECTOR_SHIFT(0);

// The interrupt control/status register is a mess, there are write
// bits and read bits and they differ.  e.g. bit 31 when written is
// "UPdate IRQ Pulse" when read is the status of IRQ source 7.
// sigh so we have IRQCTL_RD and IRQCTL_WR bit/shifts/fields etc.
// to distingquish.  Use an _RD_ bit to read and an WR bit to write.

                  // the write bits.

static const uint32_t IRQCTL_WR_UPDATEPULSE (0x80000000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ7_SRC(0x00800000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ6_SRC(0x00400000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ5_SRC(0x00200000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ4_SRC(0x00100000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ3_SRC(0x00080000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ2_SRC(0X00040000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ1_SRC(0x00020000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ0_SRC(0X00010000);
static const uint32_t IRQCTL_WR_ENABLE_IRQ7_SRC(0x80);
static const uint32_t IRQCTL_WR_ENABLE_IRQ6_SRC(0x40);
static const uint32_t IRQCTL_WR_ENABLE_IRQ5_SRC(0x20);
static const uint32_t IRQCTL_WR_ENABLE_IRQ4_SRC(0x10);
static const uint32_t IRQCTL_WR_ENABLE_IRQ3_SRC(0x08);
static const uint32_t IRQCTL_WR_ENABLE_IRQ2_SRC(0x04);
static const uint32_t IRQCTL_WR_ENABLE_IRQ1_SRC(0x02);
static const uint32_t IRQCTL_WR_ENABLE_IRQ0_SRC(0x01);

                // the read bits.

static const uint32_t IRQCTL_RD_STATUS_IRQ7(0x80000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ6(0x40000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ5(0x20000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ4(0x10000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ3(0x08000000); // End address threshold level
static const uint32_t IRQCTL_RD_STATUS_IRQ2(0x04000000); // End address threshold edge
static const uint32_t IRQCTL_RD_STATUS_IRQ1(0x02000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ0(0x01000000);

static const uint32_t IRQCTL_RD_STATUS_FLAG7(0x00800000);
static const uint32_t IRQCTL_RD_STATUS_FLAG6(0x00400000);
static const uint32_t IRQCTL_RD_STATUS_FLAG5(0x00200000);
static const uint32_t IRQCTL_RD_STATUS_FLAG4(0x00100000);
static const uint32_t IRQCTL_RD_STATUS_FLAG3(0x00080000);
static const uint32_t IRQCTL_RD_STATUS_FLAG2(0x00040000);
static const uint32_t IRQCTL_RD_STATUS_FLAG1(0x00020000);
static const uint32_t IRQCTL_RD_STATUS_FLAG0(0x00010000);

static const uint32_t IRQCTL_RD_VMEIRQSTATUS(0x8000);
static const uint32_t IRQCTL_RD_INTIRQSTATUS(0x4000);
static const uint32_t IRQCTL_RD_ENABLE_IRQ7_SRC(0x80);
static const uint32_t IRQCTL_RD_ENABLE_IRQ6_SRC(0x40);
static const uint32_t IRQCTL_RD_ENABLE_IRQ5_SRC(0x20);
static const uint32_t IRQCTL_RD_ENABLE_IRQ4_SRC(0x10);
static const uint32_t IRQCTL_RD_ENABLE_IRQ3_SRC(0x08);
static const uint32_t IRQCTL_RD_ENABLE_IRQ2_SRC(0x04);
static const uint32_t IRQCTL_RD_ENABLE_IRQ1_SRC(0x02);
static const uint32_t IRQCTL_RD_ENABLE_IRQ0_SRC(0x01);

// The Arbitration CSR. \
// For the most part this is done straightforwardly however
// a read of the KILL always gives a zero.

static const uint32_t ARB_KILL_REQ      (0x80000000);
static const uint32_t ARB_RD_OTHER_GRANT(0x00200000);
static const uint32_t ARB_RD_OWN_GRANT  (0x00100000);
static const uint32_t ARB_RD_OTHER_REQ  (0x00020000);
static const uint32_t ARB_RD_OWN_REQ    (0x00010000);
static const uint32_t ARB_REQUEST       (0x00000001);

// Bits in the broad cast setup register
// For a pleasant change, the read an write bits mean
// the same thing and are present for both


static const uint32_t BCST_ADDR_MASK(0xff000000);
static const uint32_t BCST_ADDR_SHIFT(24);
static const uint32_t BCST_ENA_MASTER(0X20);
static const uint32_t BCST_ENA_BCST(0X10);

// Hardware version register:

static const uint32_t HWVERS_IS_2(0x80);
static const uint32_t HWVERS_VERSION_MASK(0xf);
static const uint32_t HWVERS_VERSION_SHIFT(0);

// inline. to convert the temp registe value to Centigrade

static inline TEMP_TO_C(uint32_t value) {
    int16_t v(value & 0xffff);
    float result = v;
    result = result / 4.0;
    return result;
}


#pragma pack (pop)


    }
}

#endif