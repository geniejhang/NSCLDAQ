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

/*
** skeleton.h:
**   Provides prototypes for the functions which are exported by the
**   NSCL readout skeleton.
**
** Author:
**   Ron Fox
**   NSCL
**   Michigan State University
**   East Lansing, MI 48824-1321
**   fox@nscl.msu.edu
**
*/
#ifndef _SKELETON_H
#define _SKELETON_H


#ifndef _DAQTYPES_H
#include <daqdatatypes.h>
#endif
#ifndef __DAQ_SPECTRODAQ_H
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <spectrodaq.h>
#define __DAQ_SPECTRODAQ_H
#endif


void initevt();			/* Initialize event readout.    */
void initrig1();		/* Initialize auxiliary trigger */
void iniscl();			/* Initialize Scaler readout.   */
void clearevt();			/* Clear the event              */
void clrtrig1();		/* Clear aux trigger readout    */
void clrscl();			/* Clear scaler readout.        */
UINT16  readevt(DAQWordBufferPtr& Buffer); /* Read an event  */
UINT16  readscl(UINT32* pBuffer, int nScalers); /* Read scaler set   */
void trig1dis();		/* Disable Aux. trigger         */
void trig1ena();		/* Enable  Aux. trigger         */
int  rdtrig1(UINT16* pBuffer);	/* Read out Aux. trigger.       */
UINT16 evtmax();		/* Get maximum event size.      */
UINT16 trig1max();		/* Get max readout size for aux. trigger */
void   endrun();                /* Callout for end of run. */
#endif
