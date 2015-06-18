/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   tracker.h
# @brief  Defines tracker - transition callback for showing state transitions.
# @author <fox@nscl.msu.edu>
*/
#ifndef TRACKER_H
#define TRACKER_H
#include <string>

class CStateManager;

void tracker(
    CStateManager& sm, std::string program, std::string state, void* cd
);

#endif
