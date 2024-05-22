#ifndef CTRIGGERSRS_H
#define CTRIGGERSRS_H

#include <CEventTrigger.h>
#include <time.h>
#include <chrono>

class CTriggerSRS : public CEventTrigger
{
 
private:

  // bool           m_retrigger;           // retrigger flag for pixie16 buffer readout
  // unsigned int   nFIFOWords;   // words in pixie16 output data buffer
  int            NumberOfModules;        // number of pixie16 modules
  // unsigned short ModNum;      // pixie16 module number
  // unsigned       m_fifoThreshold;
	// time_t         m_lastTriggerTime;   // Last time operator() returned true.
	// unsigned int*  m_wordsInEachModule;
  std::chrono::steady_clock::time_point last_trigg_update;
public:
	// Constructors, destructors and other cannonical operations: 
  
  CTriggerSRS ();                      //!< Default constructor.
  ~CTriggerSRS (); //!< Destructor.
  
  
  // Selectors for class attributes:
public:
  time_t start,end;
  // Mutators:
protected:  
  
  // Class operations:
public:  
  virtual void setup();
  virtual void teardown();
  virtual   bool operator() ();
  virtual   void Initialize( int nummod ); 
  void Reset();
};
#endif