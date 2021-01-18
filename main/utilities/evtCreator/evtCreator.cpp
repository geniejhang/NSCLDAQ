/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2021.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Giordano Cerizza
	     NSCL/FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "evtCreatorMain.h"

int
main(int argc, char** argv)
{
  EvtCreatorMain creator;
  return creator(argc, argv);
}
