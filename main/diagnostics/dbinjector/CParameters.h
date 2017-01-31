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
# @file   CParameters.h
# @brief  Provide accessors for command line parameters.
# @author <fox@nscl.msu.edu>
*/

#ifndef CPARAMETERS_H
#define CPARAMETERS_H

#include "cmdopts.h"
#include <string>

/**
 * @class CParameters
 *    provide accessors for the command line parameters.  This is only required
 *    because at least --filename may require some shell expansions (if defaulted
 *    e.g.).
 */
class CParameters
{
private:
    gengetopt_args_info  m_params;
public:
    CParameters(int argc, char** argv);
    std::string service() const;
    std::string filename() const;
    
};

#endif