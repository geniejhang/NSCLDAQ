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
# @file   checkfree.cpp
# @brief  Display the pct of free space on a filesystem given a filename.
# @author <fox@nscl.msu.edu>
*/

#include <io.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

int main(int argc, char**argv)
{
    const char *fname = argv[1];
    int         fd    = open(fname, O_RDONLY);
    
    double result = io::freeSpacePercent(fd);
    
    close(fd);
    
    std::cout << result << " percent\n";
}