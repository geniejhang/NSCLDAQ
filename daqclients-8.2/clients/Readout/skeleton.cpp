/* Change the line below if we ever move out of CES branch drivers:

 */
#ifndef CESCAMAC
#define CESCAMAC
#endif

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008
    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
** Ported to Linux and CES/CBD 8210 device driver.
** At present, the preprocessor symbol __unix__ is used to switch on/off
** Linux/unix specific  code. #ifndef for it is used to switch off VME 
** 68K specific code.
**   >>>>This module must be compiled without optimization<<<<
**      Ron Fox
**      January 28, 1999
*/


/*
**++
**  FACILITY:
**
**      Data acquisition system.
**
**  ABSTRACT:
**
**      skeleton.c  - This file contains an upper level readout skeleton.
**			This skeleton is called from the evtuser.c skeleton
**			at even more confined locations than evtuser.c  The
**		        user here is providing routines that tyipcally just do
**			CAMAC operations and not much else.
**			Users performing tailoring should read carefully
**			the areas which begin with the text:
**
**			INSTRUCTIONS:
**			-------------
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  CREATION DATE:      6-Mar-1990
**
**  MODIFICATION HISTORY:
**
**--
*/


/*
**
**  
INCLUDE FILES
**
*/

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <stdio.h>

#ifndef __unix__
#include "cpus.h"
#endif

#ifdef __unix__
#include <stdlib.h>
#include <daqinterface.h>
#include <spectrodaq.h>
#endif

#include <daqdatatypes.h>
#include <camac.h>
#include <macros.h>

#ifdef VME16
#undef VME16
#endif
#if CPU == MICROPROJECT
#define VME16 0	
#endif

#ifndef __unix__
#include <vme.h>
#endif
#include <buftypes.h>

/* Short circuit run time evaluation of constant BCNAF's */

#ifdef __unix__
#include <camac.h>
#else
#if CPU == IRONICS
#undef CAMBAS
#define CAMBAS(b)	0xFA800000
#endif

#if CPU == MICROPROJECT
#undef CAMBAS
#define CAMBAS(b)	0xFE800000
#endif
#endif

/*
**  INSTRUCTIONS:
**  -------------
**			    Introduction
**
**	There are several functions that the user is supposed to fill in
**	to tailor this skeleton for a particular experiment.  While the
**	skeleton is written in C, several extensions to the C language have
**	been defined to bring the syntax closer to FORTRAN.  One thing you must
**	be aware of, however is that C is case sensitive, that is, 'if' and
**	'IF' are completely different from each other and 'If'.
**
**			    Functions to fill in:
**
**	The following are the set of functions that need to be filled in:
**
**	Function    Meaning		When Called
**	initevt	    Init event readout  Power up Begin run and Resume Run.
**      endrun      End of run processing Called at end of run.
**	clrtrig1    Clr trig1 readout	Power up, Begin, resume, end of trig1.
**	iniscl	    Init scalers.	Power up Begin run and Resume Run.
**	clearevt    Clear evt readout	Power up, Begin, Resume, end of event.
**	clrtrig1    Clr trig1 readout	Power up, Begin, resume, end of trig1.
**	clrscl	    Clear scaler readout Power up, Begin, Resume, end of scaler
**	readevt	    Read an event	Event trigger.
**	readscl	    REad scaler event	Scaler time.
**	trig1ena    Enable trigger 1	Begin run, resume run.
**	trig1dis    Disable trigger1	End run
**	rdtrig1	    Readout for trig 1	User trigger 1 fires.
**	evtmax	    Max size of event	Begin run time.
**	trig1max    Max words in trig1  Begin run time.
**
**			Language elements:
**	    Statements:
**	Statements can occur anywhere on a line there can even be multiple
**	statements on a single line. The end of a statement is flagged with
**	a semicolon.  The following are legal statements:
**
**		a = b;
**		camwrite16(0,1,29,0,16,1234);
**
**	    Variables:
**	Variables must be declared at the beginning of a function.
**      C variables are the native type, however, the following FORTRAN
**      style variable declarations are supported via definitions in 
**      macros.h
**
**	    LOGICAL	- Takes the values TRUE or FALSE
**	    WORD	- INTEGER*2 sized word.
**	    INTEGER	- INTEGER*4 sized lognword.
**	    REAL	- REAL*4 sized real.
**
**	Arrays may be declared, subscripts always go from 0:size-1. In C
**	subscripts are in square brackets not rounded ones. The following
**	are legal variable declarations:
**
**	    LOGICAL flag;
**	    INTEGER i,j,k[10];
**      Note in the above, the declaration of k is identical to the FORTRAN
**      declaration:
**
**	    INTEGER*4 k(0:9)
**
**      Multidimensional arrays are also allowed, but the syntax is a bit
**      different:
**          IINTEGER twod[5][6];
**
**      In C, the last dimension runs sequentially in memory rather than
**      the first (reverse of fortran), and array references require both
**      dimensions to be used as in the declaration e.g., to fill the
**      array above:
**         
**               for(j = 0; j < 5; j++) 
**                  for(i = 0; i < 6; i++)
**                     twod[j][i] = i+j;
**         
**	    Expressions:
**	The only FORTRAN operators not allowed are exponentiation (**).
**	the following is a subset of the operators supported:
**	    + - * / AND OR EQ NE GT GE LT LE
**	The following assignment statement is an example of an expression:
**	    LOGICAL   flag;
**	    INTEGER   i1,i2,i3,i4;
**		...
**	    flag = (i1)/(i1+i2) LT (i3+i4)
**
**	Note that operators (e.g. LT above) are case sensitive.
**	Bitwise operator functions IAND, IOR, ISHIFT are defined like in FORTRAN
**	Hexadecimal constants are allowed and the syntax is 0xNNN where NNN is
**	the constant.
**
**	    Flow of control:
**
**	The following statements can control the flow of a program.
**		IF
**
**	IF(expresson) <statement>;
**
**		BLOCK IF with optional ELSE clause:
**
**	IF(expression) THEN
**	    <statements with trailing ';'s>
**	[ELSE 
**	    <statements with trailing ';'s>
**	]ENDIF
**
**		Top tested DO WHILE loop:
**
**	DO WHILE(expression)
**	    <statements>;
**	ENDDO
**
**		    Buffer manipulation:
**	The following functions manipulate the buffer:
**
**	putbufw(w)	- Put w, a 16 bit word to buffer.
**	putbufl(l)	- Put l, a 32 bit longword to the buffer in VAX word 
**			  order.
**
**		    CAMAC operations:
**
**	In the discussion below, b= branch number 0-7, c = crate number 1-7,
**	n = CAMAC slot, a = CAMAC subaddress, f = CAMAC function code.
**	d = CAMAC data transfered.
**	CAMAC operations are implemented as 'pseudo functions' which are
**	expanded in line and hence quite fast (~3usec per transaction).
**
**	camread16(b,c,n,a,f)	- Returns a 16 bit value read from CAMAC
**	camread24(b,c,n,a,f)	- Returns a 24 bit value read from CAMAC format
**				  is 68K format.
**	camwrite16(b,c,n,a,f,d)	- Write least significant 16 bits of d to CAMAC
**	camwrite24(b,c,n,a,f,d) - Write least significate 24 bits of d to CAMAC
**	camctl(b,c,n,a,f)	- Perform non data transfer CAMAC cycle.
**	rdtobuf16(b,c,n,a,f)	- Do 16 bit CAMAC read into buffer.
**	rdtobuf24(b,c,n,a,f)	- Do 24 bit CAMAC read into buffer (VAX format).
**	qtst(b)			- TRUE if most recent CAMAC operation on 
**				  given branch set Q
**	xtst(b)			- Same as qtst, but tests X status.
**
**		More complex pseudo functions:
**
**	qstop(b,c,n,a,f)	- Performs a Q stop block read into buffer.
**	branchinit(b)		- Initialize branch controller.
**	crateinit(b,c)		- C/Z/Uninhibit crate.
**
**		LAM waiting:
**
**	The Pseudo functions below set up to do LAM busy waiting on a bit
**	register decode:
**
**	BEGINLAM(numbr, numcr)	- Begin LAM mask buildup:
**					numbr	- Number of branches involved
**						  in LAM wait process.
**					numcr	- Highest crate number involved
**						  in LAM wait process.
**	ENDLAM			- End LAM mask processing.
**	READBIT(b,c,n,a,f,d)	- Read a pattern register pattern register
**				  is put in buffer and in WORD variable
**				  d
**	NEEDLAM(b,c,n)		- Indicate that LAM from given slot is needed.
**	IFTIMEOUT(maxloop)	- Loops maxloop times (>10us/pass) waiting for
**				  lams.  THen looks like IF statment, but
**				  condition is that a timeout has occured.
**	Example use of bit register functions:
**
**	LOGICAL timeout;
**	WORD	brg;
**	..
**	READDBIT(0,1,20,0,0,brg);
**	    ..
**	BEGINLAM
**  	  IF(IAND(brg, 0x1))NEEDLAM(0,2,3);
**	  IF(IAND(brg, 0x2))NEEDLAM(0,2,4);
**	  if(IAND(brg, 0x3))NEEDLAM(0,2,5);
**	  IFTIMEOUT(100) THEN
**	    timeout = TRUE;
**	    ENDDO
**	  ELSE
**	    timeout = FALSE;
**	  ENDIF
**	ENDLAM
**
**		    Device specific pseudoroutines:
**	Several device specific in-line routines are defined to make the
**  handling of more common devices easier.  Devices not supported in this set
**  can be managed with the primitive CAMAC operations. NOTE: clear functions
**  are provided for modules which can be front panel cleared just in case 
**  there is the need to clear them from CAMAC as well.
**
**    See the header macros.h for more information.
**
**	    User parameters:
**
**	The front end includes three pre-defined arrays:
**
**	    INTEGER userints[]	- Front end integer parameters.
**	    LOGICAL userflags[]	- Front end flag parameters
**	    REAL    userreals[] - Front end real paraemters.
**
** End of instructions... but look for more later in the file.
*/
extern INT16 second;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      initevt	- This function contains code which indicates how to initialize
**		  the CAMAC system for event readout.
**
**
**--
*/
void
initevt ()
{
/*
**  INSTRUCTIONS:
**  -------------
**
**	Fill in the area below the end of these instructions, but before the
**	curly bracket with code that initializes devices in the event readout
**	section of the device.  Typically what's needed here is to first
**	put all crates on-line and initialize them, and then to clear all
**	digitizers and initialize any programmable devices.  If you have
**	more than one branh highway (not just branch 0), you should initialize
**	that as well before touching crates on that branch. The routine provided
**	shows how to initialize two empty crates on branch 0.  This initializer
**	is the first of the initializers called so it's not a bad idea to do
**	all crate initializations here, unless the crates are functionally
**	broken up.  A bit register is also initialized.  See the #define
**	statments below to tailor the location of that bit register.
**  End instructions... but watch for more further down */

#define BIT_BRANCH	0		    /* Branch bit register is on. */
#define BIT_CRATE	2		    /* Crate bit register. is in */
#define BIT_SLOT	16		    /* Slot bit register is in. */
#define BIT_SUBADDRESS	0		    /* Subaddress of bit register  */


  /* Initialize a bunch of ADCs of type xD811 in slots 1-4  */



  INIT811(0,2, 1); 
  INIT811(0,2, 2); 
  INIT811(0,2, 3); 
  INIT811(0,2, 4); 

}


/*!
   Called when an end of run has been issued.
*/
void 
endrun()
{
}


/*
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      initrig1    - This section should be filled in to initialize CAMAC
**		      modules associated with user trigger 1.
**
**--
*/
void
initrig1 ()
{

/*
**  Instructions:
**  -------------
**
**	The section of code between the end of this comment and the the }
**	should be filled in with code that initializes all hardware associated
**	with user trigger1.  This trigger SHOULD NOT BE ENABLED AT THIS TIME
**	since this routine is called at power up as well as at run start.
**	The trigger should only be enabled in trig1ena().
**	The sample provided takes no action to initialize trigger 1.
** End of instructions, but there's more later...		    */

    
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      iniscl	- Initialize scaler readout.
**
**
**  IMPLICIT INPUTS:
**
**      The special variable numscalers is the number of scalers.
**
**--
*/
void
iniscl ()
{
/*
**  Instructions:
**  -------------
**	This function is called to intialize all scaler modules in the
**	system.  There is a pre-declared variable 'numscalers' which 
**	contains the number of scalers in the system.  The sample code below
**	is written to deal with either a block of 12 channel scalers (LRS2551)
**	or a block of 32 channel scalers (LRS4434), depedning on the definition
**	of the constant SCALER_CHANNELS.  If SCALER_CHANNELS is 12, then
**	code is generated for LRS2551, if SCALER_CHANNELS is 32, then for
**	LRS4434.  Several notes:
**	    1. The code below demonstrates the use of C's #define statement
**	       for generating symbolic constants (like FORTRAN parameters).
**	       Note that #defines must start in column 1.
**	    2. The code below demonstrates the use of C's #if statement to
**	       do conditional compilation.  There is a big difference between
**	      #if and IF. #if controls what code is generated, while
**	      IF controls what code is executed.  The false side of the
**	      #if statement doesn't exist and therefore cannot be used
**	      to do run time control flow.
**	    3. If all the user wants to do is modify the starting scaler,
**	      or the type of scaler, then the definitions below are all
**	      that need to be changed.  More complex changes require coding.
**	    4.The % operator is the modulus operator, that is (a % b) is the
**	      same as FORTRAN's MOD(A,B).
**	    5. Advanced note. #define creates things called MACRO's a MACRO
**	       is a stored series of text which is expanded in line when it
**	       is invoked.  MACROs can have parameters just like functions
**	       and this is how the psuedo functions and FORTRAN like syntax
**	       has been layered on to C.  For example, if you would like
**	       to define a FORTRAN like MOD pseudo function, then:
**	       12345678901234567890123456789012345678901234567890 <- Column
**	       #define MOD(a,b)  ((a) % (b))
**  End of instructions for now...				    */

#define SCALER_CHANNELS	12		    /* # Channels per module. */
#define SCALER_BRANCH	0		    /* Branch of staring scaler */
#define SCALER_CRATE	2		    /* Crate scaler is in. */
#define SCALER_FIRST_SLOT	6	    /* Slot of first scaler */

	 
    INTEGER slot,			    /* Slot we're working on. */
	    nslots,			    /* Total number of slots. */
	    i;				    /* Slot counter */
#ifdef __unix__
    unsigned numscalers;
    numscalers = daq_GetScalerCount();
#else
    extern numscalers;
#endif

	/* First compute the number of slots to init. */

    nslots = numscalers / SCALER_CHANNELS;
    IF((numscalers % SCALER_CHANNELS) NE 0) nslots = nslots + 1;
    i = 1;
    slot = SCALER_FIRST_SLOT;

	/* Select which loop to use to init the scalers. */

#if SCALER_CHANNELS EQ 12		    /* Code for LRS 2551 follows: */

    DO WHILE(i LE nslots)
      INIT2551(SCALER_BRANCH, SCALER_CRATE, slot);
      slot = slot+1;
      i = i+1;
    ENDDO;  

#else					    /* Code for LRS 4434 follows: */ 

    DO WHILE(i LE nslots)
      INIT4434(SCALER_BRANCH, SCALER_CRATE, slot);
      slot = slot+1;
      i = i+1;
    ENDDO;  

#endif
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      clearevt    - Clear user events.
**
**
**--
*/
void
clearevt ()
{
/*
**  Instructions:
**  -------------
**	This routine should be filled in with the actions needed to clear
**	all digitizers associated with the primary event.  Note that after
**	the readout, of an event, a standard NIM-out is written to with all
**	1's.  This is done in time suitable for clearing devices with front
**	panel clears.  The user need not clear devices which are cleared via
**	this signal.
**	The sample code does nothing.
**  End of instructions... but there's more.		    */
    
  /* Clear a bunch of xD811's in slots 1-4: */

  CLR811(0,2,1);
  CLR811(0,2,2);
  CLR811(0,2,3);
  CLR811(0,2,4);

}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      clrtrig1    - This function is called when a user 1 trigger read out is 
**		      complete.
**
**
**--
*/
void
clrtrig1 ()
{
/*
**  Instructions:
**  -------------
**	This function is called to clear the user1 trigger devices.  This
**	should not be used to disable triggers, since we are called at the end
**	of every trigger 1 event.  The entry trig1dis should be used instead.
**	Note when clearing trigger 1 devices, it is assumed that there need not
**	be any connection between trigger 1 and the 'event' trigger.  Therefore,
**	the front panel NIM out clear register is not written to by the uppler
**	levels of the system and must be written to by the user if that's 
**	desired.
**	    The sample code below does nothing.
**	End of Instructions for now...					    */ 
    
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      clrscl	- Clear scalers
**
**
**--
*/
void
clrscl ()
{
/*
**  Instructions:
**  -------------
**	This function is called to clear the scalers following readout, on 
**	run start and on run resume.  The sample code below is intended for 
**	use with the sample code given in iniscl(), that is we can deal
**	with either a contiguous block of 32 or 12 channel scalers.
**  End of instructions for now....				*/
     
    INTEGER slot, nslots, i;
#ifdef __unix__
    unsigned numscalers = daq_GetScalerCount();
#endif

	/* First compute the number of slots to init. */

    nslots = numscalers / SCALER_CHANNELS;
    IF((numscalers % SCALER_CHANNELS) NE 0) nslots = nslots + 1;
    i = 1;
    slot = SCALER_FIRST_SLOT;

	/* Select which clear loop to execute. */

#if SCALER_CHANNELS EQ 12		    /* Code for LRS 2551 follows: */

    DO WHILE(i LE nslots)
      CLR2551(SCALER_BRANCH, SCALER_CRATE, slot);
      slot = slot+1;
      i = i+1;
    ENDDO;  

#else			    /* Code for LRS 4434 follows: */

    DO WHILE(i LE nslots)
      CLR4434(SCALER_BRANCH, SCALER_CRATE, slot);
      slot = slot+1;
      i = i+1;
    ENDDO;  

#endif
    
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      readevt	- This function is called to read an event.
**
**  FORMAL PARAMETERS:
**
**      INT16 *bufpt	- Pointer to buffer to fill.
**
**
**  FUNCTION VALUE:
**
**      Number of words read.
**
**--
*/
WORD
#ifdef __unix__
readevt (DAQWordBufferPtr& bufpt)
#else 
readevt (WORD* bufpt)
#endif
{
#ifdef __unix__
    DAQWordBufferPtr _sbufpt = bufpt;
#else
    WORD *_sbufpt = bufpt;
#endif
    LOGICAL reject;

    reject   = FALSE;
    {    
/*------------------------- Begin user code. ----------------------------*/
/*
**  Instructions:
**  -------------
**	This routine must be filled in with code that indicates how to perform
**	the event readout.  Digitizers need not be cleared after readout,
**	that's done by a call back down here to clearevt().  Some points of
**	interest:
**	    1. the predefined array _sbufpt can be used to reference the
**	       buffer after words have been read, that is, _sbufpt[0] is
**	       the first word you read.
**	    2. The predefined logical variable reject can be set to TRUE to
**	       inform the system that the event is to be thrown away.
**	    3. The data the user reads will be prefaced by the system with
**	       a word count.
**	    The sample code below is a skeleton which cracks a single 16
**	    bit register, waits for indicated lams for each bit and then
**	    does the read out on the set bits in the register.  If a LAM timeout
**	    occured, then the top bit of the bit register is set in the buffer
**	    and the readout procedes.  Note the use of #defines to tailor
**	    the location of the bit register, the LAM's expected for each bit,
**	    and the bits that are actually used in the register.
**	    Note how conditional compilation is used to remove tests for bits
**	    which are not used by the experiment.
**	    Note the use of the pre-defined 2-d array _lammsk in the LAM
**	    decoding code below.  Setting the bits all at once is faster
**	    than one at a a time (via NEEDLAM).  Note also the difference
**	    in the way C addresses 2-d arrays.  The predefined array _lammsk
**	    is an array of branch/crate lam masks.
**  End of instructions for now...				*/


      /* We'll drive the readout on the pattern register in 
      // slot 17.  We'll only use the bottom 4 bits.  The event
      // will have the following format:
      //    bit-register
      //     Packets
      //  where each packet contains a word count, followed by a packet id
      //  followed by data relevant to the packet.
      //    The packet ID is just the number of the bit (from 1) for which
      //  the readout was done.
      */
      UINT16 bitreg = camread16(0,2,17, 0, 0);
      putbufw(bitreg);

      if(bitreg & 1) {		/* Bit 1 readout. */
	Packet(6,1);
	NIMOUT(0,2,20,0xffff);
	NIMOUT(0,2,20,0xffff);
	READ811(0,2,1,0);
	NIMOUT(0,2,20,0xffff);
	READ811(0,2,2,0);
	READ811(0,2,3,0);
	READ811(0,2,4,0);
	EndPacket;
      }
      if(bitreg & 2) {		/* Bit 2 readout. */
	Packet(10,2);
	READ811(0,2,1,1);
	READ811(0,2,2,1);
	READ811(0,2,3,1);
	READ811(0,2,4,1);
	READ811(0,2,1,2);
	READ811(0,2,2,2);
	READ811(0,2,3,2);
	READ811(0,2,4,2);
	EndPacket;
      }
      if(bitreg & 4) {		/* Bit 3 readout. */
	Packet(6,3);
	READ811(0,2,1,3);
	READ811(0,2,2,3);
	READ811(0,2,3,3);
	READ811(0,2,4,3);
	EndPacket;
      }
      if(bitreg & 8) {		/* Bit 4 readout. */
	Packet(14,4);
	READ811(0,2,1,4);
	READ811(0,2,2,4);
	READ811(0,2,3,4);
	READ811(0,2,4,4);
	READ811(0,2,1,5);
	READ811(0,2,2,5);
	READ811(0,2,3,5);
	READ811(0,2,4,5);
	READ811(0,2,1,6);
	READ811(0,2,2,6);
	READ811(0,2,3,6);
	READ811(0,2,4,6);
	EndPacket;
      }

/*-------------------------  End of user code. ---------------------------*/
}
    IF(reject) return 0;
#ifdef __unix__
    return bufpt.GetIndex() - _sbufpt.GetIndex();
#else
    return (bufpt - _sbufpt);
#endif
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      readscl	 - Read out scalers.
**
**  FORMAL PARAMETERS:
**
**	UINT32 *bufpt	        - buffer pointer.
**      int numscalers		- Number of scalers to read out.
**

**
**
**--
*/
UINT16
readscl (UINT32* buffer,int numscalers)
{
    UINT32* _sbufpt = buffer;
    UINT16*  bufpt  = (UINT16*)buffer;



    {
/*
**  Instructions:
**  -------------
**
**	This section of code must be filled in to read out scalers into a buffer
**	provided by the caller.  The special predeclared array INTEGER _sbufpt[]
**	allows access to the buffer contents by longword number.
**	The sample code below continues the examples of scaler handling shown
**	so far.  If 12 channel scalers are used, we read them out assuming
**	that we've got a contiguous block of them.
**	If 32 channel scalers are used, then we use them instead.
**	The predefined INTEGER numscalers is the number of scaler channels
**	to be read out (set by SET SCALERS command).
**	  NOTES:
**	    1. After the scalers have been read out, the upper levels of code
**		will call the scaler clear routine so it is not necessary
**		to clear scalers at this level.
**	    2.  Do not molest the code below the dashed line as it is necessary
**		to the correct operation of the system.
** End of instructions for now:				*/

	INTEGER slot, nslots, i;
	INTEGER oddregs;		/* Odd # of registers */
	INTEGER oddscaler;		/* Channel from odd scalers. */

	/* Compute number of slots and partial register count: */
	 
	nslots = numscalers / SCALER_CHANNELS;
	oddregs= numscalers % SCALER_CHANNELS;

	/* Initialize the loop variables for the readout loop: */

	i = 1;
	slot = SCALER_FIRST_SLOT;

#if SCALER_CHANNELS EQ 12		    /* Use 12 channel scalers: */

	DO WHILE(i LE nslots)		    /* Read full slots: */
	    READALL2551(SCALER_BRANCH, SCALER_CRATE, slot);
	    slot = slot+1;
	    i    = i+1;
	ENDDO;

	i = 0;				    /* i will be subaddress */
	DO WHILE(i LT oddregs)
	    READ2551(SCALER_BRANCH, SCALER_CRATE, slot, i);
	    i = i+1;
	ENDDO;
#else					    /* Use 32 channel scalers. */

	DO WHILE(i LE nslots)		    /* Read full slots: */
	    READALL4434(SCALER_BRANCH, SCALER_CRATE, slot);
	    slot = slot+1;
	    i    = i+1;
	ENDDO;

	i = 0;				    /* i will be subaddress */
	DO WHILE(i LT oddregs)
	    READ4434(SCALER_BRANCH, SCALER_CRATE, slot, i);
	    i = i+1;
	ENDDO;
#endif

/*-------------------------------END USER CODE ---------------------------*/
    }

     
    return (UINT16)((UINT32)bufpt - (UINT32)_sbufpt);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      trig1dis    - Disable trigger 1 triggers.
**
**
**--
*/
void
trig1dis ()
{

/*
** NOTE:
**    >>>>User triggers are not supported in the UNIX readout system<<<<
**  Instructions:
**  -------------
**	The code below is used to turn off user triggers.  At present, the only
**	sort of user triggers supported are time periodic user triggers for
**	user trigger 1.  User triggers are intended to trigger readout events
**	that are not necessarily part of the normal set of event triggers.
**	These might be triggers to readout calibration systems or other
**	monitoring systems.
**	  The sample code below is compiled if the #define for USERTRIG1_ENABLE
**	is set to be true.  In that case, the frequency and eventy type produced
**	for user triggers is controlled by the #define statements for
**	USER1_PERIOD	(INTEGER seconds between triggers) and:
**
**  End of instructions for now:    */

#ifndef __unix__
#define USERTRIG1_ENABLE	FALSE		/* TRUE if triggers desired */
#define USERTRIG1_PERIOD	-1		/* Seconds between triggers */

#if USERTRIG1_ENABLE

    STOPUSR1TRIG();
    
#endif
#endif
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      trig1ena    - This function is called when user triggers are to be
**		      enabled.
**
**
**--
*/
void
trig1ena ()
{
/*
**  NOTE:
**     >>>>User triggers are not supported in the UNIX environment<<<<
**  Instructions:
**  -------------
**	This section of code should be filled in to enable user triggers.
**  The sample code continues the example begun for trig1dis.
**  End of instructions	for now:	*/
#ifndef __unix__
INTEGER period;


#if USERTRIG1_ENABLE
    IF(USERTRIG1_PERIOD LE 0) THEN
	msg("FATAL - Trigger frequency less than zero in trig1ena()");
	newline; newline;
	die();
    ENDIF

    STARTUSR1TRIG(USERTRIG1_PERIOD);

#endif
#endif
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      rdtrig1	- Read out a user trigger.
**
**  FORMAL PARAMETERS:
**
**      INT16 *buffer	- Pointer to buffer to readout.
**
*/
int 
rdtrig1 (WORD* bufpt)
{
    WORD *_sbufpt;

    _sbufpt = bufpt;
    {
/*
**  NOTE:
**    >>>>The UNIX environment does not support user triggers <<<<
**  Instructions:
**  -------------
**	This area should be filled in with code that manages the readout of a
**	user 1 trigger.  The sample code below assumes that if user triggers are
**	enabled, you will want to trigger an action via setting a bit in a NIM
**	out register. 
**	  The default event type of the event being read out
**	is USERBUF1 (32).  If not data is read into the buffer, then no event
**	is generated.  Similarly, if the function returns the value zero, then
**	no event is generated.
**	NOTE:
**	    1.  The special variable bufpt is a 'pointer' to the event buffer.
**	    2.  As before, the predeclared variable WORD _sbufpt[] gives
**		you a way to modify the buffer after readout (e.g. put in
**	        a special bit in the bit register.
**	    3.  The defines for NIMOUT_xxx allow the CAMAC location of the
**		nimout to be defined, as well as the bit that's actually
**		fired off.
**  End of instructions for now...		    */
#ifndef __unix__
#define NIMOUT_BRANCH	0			    /* Branch nimout is in */
#define NIMOUT_CRATE	2			    /* Crate nimout is in. */
#define NIMOUT_SLOT	20			    /* Slot nimout lives in */
#define NIMOUT_TRIG1	0x800			    /* Bit to set. */

#if USERTRIG1_ENABLE
	NIMOUT(NIMOUT_BRANCH, NIMOUT_CRATE, NIMOUT_SLOT, NIMOUT_TRIG1);

/* -------------------------- End user code. ----------------------- */
#endif
#endif

    }
    return (bufpt - _sbufpt);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      evtmax	- Returns the size of the largest physics event.
**
**  FUNCTION VALUE:
**
**      Largest number of words that can be read out by a physics trigger.
****--
*/
WORD 
evtmax ()
{

/*
**  Instructions:
**  -------------
**	Fill in the #define below to indicate the size of the largest
**	possible physics event in words.
**  End of instructions for now...	    */

#define EVENT_MAXWORDS	40	/* Fill with correct size */

/*------------------------------ end of user code  -----------------------*/

    IF (EVENT_MAXWORDS LE 0) THEN	    /* We crash the program if the */
					    /* user didn't set the size */
	fprintf (stderr, "EVENT_MAXWORDS was not properly defined\n");
	fprintf (stderr, "Must be .GT. 0 was %d \n", EVENT_MAXWORDS);
#ifdef __unix__
	abort();
#else
        panic("BUGCHECK");
#endif
    ENDIF;
    return (EVENT_MAXWORDS);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      trig1max    - Returns the number of words readout in a user1 trigger.
**
**  FUNCTION VALUE:
**
**      Number of  words read out.
**
**
**--
*/
WORD 
trig1max ()
{
/*
** NOTE:
**    >>>>The UNIX environment does not support user triggers <<<<
**
**  Instructions:
**  -------------
**	This function should be filled in to indicate the maximum number
**	of words to be read out on a user trigger 1.  The sample code operates
**	as follows, if the #define'd constant USERTRIG1_ENABLE is FALSE, then
**	the value 0 is generated (no words read out), If USERTRIG1_ENABLE is
**	true, then the user should edit the definition of USERTRIG1_MAXWORDS
**	to reflect the largest number of words that can be read out in a
**	user1 trigger.  
**  End of all Instructions for now....				*/

#define USERTRIG1_MAXWORDS	0	    /* Edit to reflect actual count */

#if USERTRIG1_ENABLE && (!defined(__unix__))
    IF (USERTRIG1_MAXWORDS LT 0) THEN
	fprintf (stderr, "USERTRIG1_MAXWORDS incorrectly initialized\n");
	fprintf (stderr, "Must be GE 0 was %d\n", USERTRIG1_MAXWORDS);
	die();
	return USERTRIG1_MAXWORDS;
    ENDIF;
#else
    return 0;
#endif    

}

