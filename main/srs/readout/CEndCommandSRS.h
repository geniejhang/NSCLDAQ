#ifndef CENDCOMMANDSRS_H
#define CENDCOMMANDSRS_H

#include <CEndCommand.h>
#include <tcl.h>

class CTCLInterpreter;
class CTCLObject;
class CEventSegmentSRS;
class CExperiment;


class CEndCommandSRS : public CEndCommand
{
  public:
    struct EndEvent {
      Tcl_Event s_rawEvent;
      CEndCommandSRS* s_thisPtr;
    };
 
private:

public:
	// Constructors, destructors and other cannonical operations: 
  
    CEndCommandSRS(CTCLInterpreter& interp, CEventSegmentSRS *myevseg, CExperiment* exp);              //!< Default constructor.
  ~CEndCommandSRS (); //!< Destructor.
  
private:
  CEndCommandSRS(const CEndCommandSRS& rhs);
  CEndCommandSRS& operator=(const CEndCommandSRS &rhs);
  int operator==(const CEndCommandSRS& rhs) const;
  int operator!=(const CEndCommandSRS& rhs) const;
    
  CEventSegmentSRS *myeventsegment;
  CExperiment*     m_pExp;
  int NumModules;
  
  // Class operations:
public:  
  // int transitionToInactive();
  // int readOutRemainingData();
  int endRun();
  void rescheduleEndTransition();
  void rescheduleEndRead();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

protected:
  static int handleEndRun(Tcl_Event* pEvt, int flags);
  static int handleReadOutRemainingData(Tcl_Event* pEvt, int flags);
 
};
#endif
