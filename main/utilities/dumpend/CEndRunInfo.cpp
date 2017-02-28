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
# @file   CEndRunInfo.cpp
# @brief  Implements non-pure virtual methods of CEndRun ABC
# @author <fox@nscl.msu.edu>
*/
#include "CEndRunInfo.h"
#include <iostream>


/**
 * constructor
 *    @param fd - file descriptor open on the event file.
 */
CEndRunInfo::CEndRunInfo(int fd) :
    m_nFd(fd)
{}

CEndRunInfo::~CEndRunInfo() {}


void CEndRunInfo::dump(std::ostream& stream) const
{
    stream << "Has " << numEnds() << " end run records\n";
    for (int i = 0; i < numEnds(); i++) {
        stream << "End run record # " << i << std::endl;
        dumpBodyHeader(i, *this, stream);
        dumpBody(i, *this, stream);
    }
}



void CEndRunInfo::dumpBody(int i, const CEndRunInfo &e, std::ostream& stream) const
{
    time_t tod = e.getTod(i);
    stream << "Body: \n";
    stream << "      Run                : " << e.getRunNumber(i) << std::endl;
    stream << "      Seconds run lasted : " << e.getElapsedTime(i) << std::endl;
    stream << "      Run Title          : " << e.getTitle(i)  << std::endl;
    stream << "      Run Ended at       : " << ctime(&tod) << std::endl;
}
