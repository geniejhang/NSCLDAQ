/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "EVBFramework.h"
#include "CEVBFrameork.h"
/**
 * EVBFramework.cpp:
 * Implements the API back to the framework from client code.
 */


/**
 * getProgramOptions - Returns the program options parsed by gengetopt.
 *                     this function supports user extensions to the gengetopt file
 *                     that defines program options.
 *
 * @return const struct gengetopt_args_info
 */
const struct gengetopt_args_info* 
EVBFramework::getProgramOptions()
{
  return CEVBFramework::getInstance()->getParsedArgs();
}
/**
 * submit a set of fragments to the event builder.
 *
 * @param flist - List of fragments to send.
 */
void
EVBFramework::submitFragmentList(CEVBEventList& flist)
{
  CEVBFramework::getInstance()->send(flist);
}
/**
 * Run the event builder client main.
 *
 * @param argc -number of arguments.
 * @param argv -Pointers to the argument list elements.
 */
int
EVBFramework::main(int argc, char**argv)
{
  return CEVBFramworkApp::main(argc, argv);
}
