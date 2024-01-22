/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

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
 * @file Skeleton.h
 * @brief Defines a skeleton for production readout software for NSCLDAQ 10.0 
 * and later.
 */

#ifndef SKELETON_H
#define SKELETON_H

#include <CReadoutMain.h>

class CTCLInterpreter;
class CExperiment;

/**
 * @class Skeleton
 * @brief A skeleton for the production readout software for NSCLDAQ 10.0
 * and later.
 * @details
 * The programmatic interface to NSCLDAQ 10.0 at the application level is a 
 * 'close match' to that of earlier versions. The software itself is a 
 * complete re-write so some incompatibilities may exist. If you find an 
 * incompatibility, please post it at daqbugs.nscl.msu.edu so that it can 
 * be documented, and addressed. Note that this does not necessarily mean 
 * that the incompatibility will be 'fixed'.
 *
 * How to use this skeleton:
 *
 * This skeleton is the 'application' class for the production readout 
 * software. The application class has several member functions you can 
 * override and implement to perform user specific initialization.
 * 
 * These are:
 * - AddCommands         : Extend the Tcl interpreter with additional commands.
 * - SetupRunVariables   : Creates an initial set of run variables.
 * - SetupStateVariables : Creates an initial set of state variables.
 * - SetupReadout        : Sets up the software's trigger and its response to 
 *                         that trigger.
 * - SetupScalers        : Sets up the response to the scaler trigger and, if 
 *                         desired, modifies the scaler trigger from a periodic
 *                         trigger controlled by the 'frequency' Tcl variable 
 *                         to something else.
 *
 * For more information about how to tailor this skeleton, see the comments 
 * in Skeleton.cpp
 */

class Skeleton : public CReadoutMain
{
private:
    
    // If you need per instance data add it here.

public:
    
    // Overrides for the base class members described above:

    /**
     * @brief Setup the Readout.
     * @param pExperiment Pointer to the experiment object.
     */
    virtual void SetupReadout(CExperiment* pExperiment);
    /**
     * @brief Setup the scaler Readout.
     * @param pExperiment Pointer to the experiment object.
     */
    virtual void SetupScalers(CExperiment* pExperiment);
    /**
     * @brief Used to add Tcl commands. See the CTCLObjectProcessor class.
     * @param pInterp Pointer to CTCLInterpreter object that encapsulates the 
     *   Tcl_Interp* of our main interpreter.
     */
    virtual void addCommands(CTCLInterpreter* pInterp);
    /**
     * @brief Setup run variables.
     * @param pInterp Pointer to CTCLInterpreter object that encapsulates the 
     *   Tcl_Interp* of our main interpreter.
     */
    virtual void SetupRunVariables(CTCLInterpreter* pInterp);
    /**
     * @brief Setup state variables.
     * @param pInterp Pointer to CTCLInterpreter object that encapsulates the 
     *   Tcl_Interp* of our main interpreter.
     */
    virtual void SetupStateVariables(CTCLInterpreter* pInterp);

private:
    
    // If you want to define some private member utilities, define them here.

};

#endif
