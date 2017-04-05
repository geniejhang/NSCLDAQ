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
# @file   CParameters.cpp
# @brief  Implement command line parameters getters.
# @author <fox@nscl.msu.edu>
*/

#include "CParameters.h"
#include <stdexcept>
#include <wordexp.h>

/**
 * constructor
 *    Parse the parameters into the member struct (m_params).
 *
 *  @param[in] argc   - Number of command line parameters.
 *  @param[in] argv   - The command line parameters themselves.
 *  @note cmdline_parser claims to return nonzero on errors be we know that
 *                       it actually exits with a message.  Nonetheless we'll
 *                       valiantly check the error code and throw std::invalid_argument
 *                       on error.
 */
CParameters::CParameters(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &m_params)) {
        throw std::invalid_argument("error parsing command line parameters");
    }
    
}
/**
 * service
 *   @return std::string - return the service_arg without modification.
 */
std::string
CParameters::service() const
{
    return m_params.service_arg;
}
/**
 * filename
 *    Return the filename.  Note that we are going to do shell substitutions
 *    on this first as the default has a ~ specification we need to expand.
 *
 * @return std::string - database filename.
 */
std::string
CParameters::filename() const
{
    wordexp_t result;
    if (wordexp(m_params.file_arg, &result, 0)) {
        throw std::invalid_argument("Failed doing tilde expansion on --filename");
    }
    std::string strresult(result.we_wordv[0]);
    wordfree(&result);
             
    return strresult;         
}