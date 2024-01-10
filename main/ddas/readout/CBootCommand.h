/* 
   This software is Copyright by the Board of Trustees of Michigan
   State University (c) Copyright 2013.

   You may use this software under the terms of the GNU public license
   (GPL).  The terms of this license are described at:

   http://www.gnu.org/licenses/gpl.txt

   Author:
       Ron Fox
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

/**
 * @file CBootCommand.h
 * @brief Class for the ddasboot command.
 */

#ifndef CBOOTCOMMAND_H
#define CBOOTCOMMAND_H

#include <TCLObjectProcessor.h>

class CMyEventSegment;
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CBootCommand
 * @details
 * This class implements the ddasboot command.  It is added to to the
 * Tcl interpreter that runs ddasreadout so that the DDAS modules can be
 * booted on demand rather than every time the Readout program starts.
 *
 * Syntax:
 * @verbatim
 *     ddasboot
 * @endverbatim
 */

class CBootCommand : public CTCLObjectProcessor
{
private:
    CMyEventSegment* m_pSegment; //!< Event segment to manipulate.
public:
    /**
     * @brief Constructor.
     * @param interp Reference to interpreter.
     * @param pCmd The command string.
     * @param pSeg The event segment to manipulate.
     */
    CBootCommand(
	CTCLInterpreter& interp, const char* pCmd, CMyEventSegment* pSeg
	);
    /** @brief Destructor. */
    virtual ~CBootCommand();

    /**
     * @brief operator().
     * @param interp Reference to interpreter.
     * @param objv Command words.
     * @return Status of the command.
     * @retval TCL_OK Success
     * @retval TCL_ERROR Failure. Human-readable reason for failure is in the 
     *   interpreter result.
     */
    virtual int operator()(
	CTCLInterpreter& interp, std::vector<CTCLObject>& objv
	);
};

#endif
