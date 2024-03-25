/**
 * @file CMyEndCommand.h
 * @brief Define an end run command.
 */

#ifndef CMYEND_H
#define CMYEND_H

#include <tcl.h>

#include <CEndCommand.h>

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;
class CExperiment;

/**
 * @class CMyEndCommand
 * @brief Provide an end command to permanently end a data-taking run
 * (list-mode data in the XIA-verse).
 */

class CMyEndCommand : public CEndCommand
{
public:
    /**
     * @struct EndEvent.
     * @brief Tcl event and Tcl interpreter command for end event.
     */
    struct EndEvent {
	Tcl_Event      s_rawEvent; //!< Generic event for the Tcl event system.
	CMyEndCommand* s_thisPtr;  //!< Pointer to this command.
    };
 
private:        
    CMyEventSegment* m_pSeg;     //!< End for modules in this segment.
    CExperiment*     m_pExp;     //!< The experiment we're reading data from.
    int              m_nModules; //!< Number of modules in the event segment.

public:
    /**
     * @brief Constructor.
     * @param interp Reference to interpreter.
     * @param pSeg   Pointer to the event segment to manipulate.
     * @param pExp   Pointer to the experiment we're reading data from.
     */
    CMyEndCommand(
	CTCLInterpreter& interp, CMyEventSegment* pSeg, CExperiment* pExp
	);
    /** @brief Destructor. */
    ~CMyEndCommand ();
  
private:
    /**
     * @brief Copy constructor.
     * @param rhs References the CMyEndCommand we are copy-constructing.
     */
    CMyEndCommand(const CMyEndCommand& rhs);
    /**
     * @brief operator=
     * @param rhs References the CMyEndCommand we are assigning to lhs.
     * @return Reference to left-hand side operand.
     */
    CMyEndCommand& operator=(const CMyEndCommand &rhs);
    /**
     * @brief operator==
     * @param rhs References the CMyEndCommand for comparison.
     */
    int operator==(const CMyEndCommand& rhs) const;
    /**
     * @brief operator!=
     * @param rhs References the CMyEndCommand for comparison.
     */
    int operator!=(const CMyEndCommand& rhs) const;
  
    // Class operations:
public:
    /**
     * @brief End the run for the event segment.
     * @return int
     * @retval 0 Success.
     * @retval TCL_ERROR If the end run operation cannot be communicated to 
     *     the modules.
     */
    int transitionToInactive();
    /** 
     * @brief Read out the data remaining on the modules. 
     * @return int
     * @retval 0 Success.
     * @retval TCL_ERROR If the mutex cannot be unlocked after some effort.
     */
    int readOutRemainingData();
    /** 
     * @brief Check whether or not an end run operation is permitted.
     * @return int
     * @retval TCL_OK If an end run is allowed.
     * @retval TCL_ERROR If the device or trigger end are not successful.
     */
    int endRun();
    /** @brief Put the end run event on the back of the Tcl event queue. */
    void rescheduleEndTransition();
    /** @brief Put the end read event on the back of the Tcl event queue. */
    void rescheduleEndRead();
    /**
     * @brief operator()
     * @param interp Reference to interpreter.
     * @param objv Command words.
     * @retval TCL_OK Success
     * @retval TCL_ERROR Failure. Human-readable reason for failure is in the 
     *   interpreter result.
     */
    virtual int operator()(
	CTCLInterpreter& interp, std::vector<CTCLObject>& objv
	);

protected:
    /**
     * @brief Handle the end run command.
     * @param pEvt The Tcl event.
     * @param flags Flags associated with the command (???).
     * @return int
     * @retval 0 (Always).
     */
    static int handleEndRun(Tcl_Event* pEvt, int flags);
    /**
     * @brief Handle the end run command.
     * @param pEvt The Tcl event.
     * @param flags Flags associated with the command (???).
     * @return int
     * @retval 0 (Always).
     */
    static int handleReadOutRemainingData(Tcl_Event* pEvt, int flags);
 
};
#endif
