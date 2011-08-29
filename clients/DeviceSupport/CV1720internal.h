/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2011

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Kole Reece, Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/*!
 * \file CV1720internal.h - This file contains register file and bit 
 *       definitions for the CAEN V1720 FADC module.
 *       it is intended to be included by device support software for
 *       the V1720.
 */

static const unsigned READY_WAITUS(5000);

static const unsigned ADDLENGTH(0x10000);     // Size of register set. 

static const unsigned EVENT_READOUT_BUFFER(0); // Event readout fifo. 

static const unsigned ACQUISITION_CONTROL(0x8100); /* Acquisition control register. */

static const unsigned ACQUISITION_STATUS(0x8104);

static const unsigned ACQSTAT_ACQREADY(0x80);

static const unsigned ACQ_START(0x4); /* Start/stop bit. */

static const unsigned BUFFER_ORG(0x800c);
static const unsigned MAXBUFS(7);
static const unsigned CUSTOM_SIZE(0x8020);

static const unsigned EVENT_SIZE(0x814c);      // Event size register. 

static const unsigned SW_RESET =  0xEF24;      // Register  to reset the Module 

static const unsigned SW_CLEAR = 0xEF28;       //Register to clear  the board  

static const unsigned BOARD_ID = 0xEF08;       //Board ID            
                      
static const unsigned RELOAD = 0xEF34;         //Register to Reload the board firmware            

static const unsigned CHAN_THRESHOLD=0x1080;  //set the  threshold for a channel
static const unsigned CHAN_OVERUNDER = 0x1084;

static const unsigned INC_CHANNEL= 0x0100;  //Defines the distance between channnels 

static const unsigned CHAN_MASK= 0x8120;    //Enable/Disable mask

static const unsigned DPP_PAR1_CH0 = 0x1024; /* DPP-PAR1 ch0 */

static const unsigned DPP_PAR1= 0x8024;      //Broadcast DPP_PAR1 Register 

static const unsigned DPP_PAR2_CH0 = 1028;

static const unsigned DPP_PAR2= 0x8028;      //DPP_PAR2 Register 

static const unsigned DPP_PAR3_CH0 = 0x102c; 
 
static const unsigned DPP_PAR3 = 0x802C;     //DPP_PAR3 Register 
 
static const unsigned DC_OFFSET_BASE=0x1098;  // Base for DC offset for a channel

static const unsigned INDIVIDUAL_TRIG = 0x100;  // Individual trigger mode (required).

static const unsigned INVERT_SIG = 0x0200;  //Invert input Signal Regsiter 

static const unsigned CONFIG=0x8000;  // CONFIG  register in normal mode 

static const unsigned CONFIG_SET=0x8004;  // CONFIG register in bet set mode

static const unsigned CONFIG_CLEAR=0x8008; //CONFIG register in bit cler mode

static const unsigned POST_TRG=0x8114;    //POST Trigger Window

static const unsigned OVERLAP=0x0002;    // Trigger Overlapping 

static const unsigned SELF_TRIGGER=0x0080; // Self Trigger mode

static const unsigned MODE=0x00020000;  //Gate Mode 

static const unsigned GATE= 0x0000; //Selects  Gate  at Digital I/0 when written to  config register 

static const unsigned DISCRI=0x8000; //Selects Discriminator at Digital I/0 when written to  config register 

static const unsigned COINCIDENCE=0x4000; // Selects COINCIDENCE at Digital I/0 when written to  config register  

static const unsigned DIOMASK = 0xC000;   //Digital I/O mask COINCIDENCE MASK

static const unsigned TRIGAVGMASK=0x3F00;  //Field Mask For Trigger Averaging Period 

static const unsigned TRIGAVGSHIFT=0x0008; //Shift count For Trigger Averaging Period 

static const unsigned RISETIMEMASK=0x003F; //Field Mask for Rise Tine 

static const unsigned GATETAILMASK=0x03FF;    // Field Mask for Gate tail width

static const unsigned GATETAILSHIFT=0x0000;   // Shift count for gate tail Width

static const unsigned PRETRIGMASK=0xFF000;      //Field  Mask for  Gate Pre Trigger Width 

static const unsigned PRETRIGSHIFT =0x000C;      //Shift Count for Gate Pre Trigger Shift 

static const unsigned GATEHOLDOFFMASK =0x7FF00000;    //Field Mask for  Gate Hold Off Width 

static const unsigned GATEHOLDSHIFT=0x0014;          // Shift Count for Gate Hold Off Width 

static const unsigned BASELINETHRESHMASK=0x00FF0000; // Feild Mask for baseline threshold

static const unsigned BASELINETHRESHSHIFT=0x0010;    // Shift count  for  baseline Threshold   

static const unsigned BASELINEWIDTHMASK= 0x000007FF; // Field maks for baseline inhibit width 

static const unsigned BASELINEWIDTHSHIFT=0x0000;    // Shift for baseline inhibit width 

static const unsigned BASELINEPERMASK=0x0FFF0000;   // Field Mask for Baseline  Avg Window/Period 

static const unsigned BASELINEPERSHIFT=0x0010;      // Shift Count for  Baseline  Avg Window/Period 

static const unsigned COINCIDENCEMASK = 0xFF000000; //Field Mask coincidence time 

static const unsigned COINCIDENCESHIFT= 0x0018;    //Shift Count for conincedence time

static const unsigned CHANNEL_COUNT=8; // Number of Channel for this Module 

static const unsigned CHANENABLEMAX=0x00FF; //Channle enable mask 

static const unsigned TRIGAVGMAX=0x003F;       //Max Triggger Arveraging 

static const unsigned RISETIMEMAX=0x003F;     //Max Rise Time 

static const unsigned GATETAILWIDTHMAX=0x03FF; // Max fate Tail Width 

static const unsigned GATEPRETRIGMAX= 0x00FF;  // Max Gate Pre Trigger width 

static const unsigned HOLDOFFMAX=0x07FF; // HoldOff Max

static const unsigned BASELINETHRESHMAX=0x00FF;     //Max Baseline Threshold 

static const unsigned BASELINEWIDTHMAX =0x07FF;  // Max Baseline Inhibit width 

static const unsigned AVGMAX=0x0FFF;            //Max value for Basline averaging calculation

static const unsigned COINCIDENCEMAX=0x00FF;  //Largest value that can be used to set the COINCEDENCE TIME

static const unsigned EVENTSTORED = 0x812C; 

static const unsigned TRIGGER_SOURCE = 0x810C;
static const unsigned TSRC_SWTRIG    = 0x80000000;
static const unsigned TSRC_EXTRIG    = 0x40000000;

static const unsigned TRIGOUT_MASK   = 0x8110;
