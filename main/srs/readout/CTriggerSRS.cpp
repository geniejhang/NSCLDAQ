/* Null trigger */

#include <config.h>
// #include <config_pixie16api.h>
#include <iostream>
#include "CTriggerSRS.h"
#include <stdlib.h>


//#ifdef HAVE_STD_NAMESPACE
using namespace std;
//#endif

// CTrigger::CTrigger(): m_retrigger(false), m_fifoThreshold(EXTFIFO_READ_THRESH*10),
//   m_wordsInEachModule(nullptr)
// {
//   //  If FIFO_THRESHOLD is defined and is a positive integer, it replaces
//   //  the default value of m_fifoThreshold - the number of words that must be
//   //  in the fifo for the trigger to fire.
//   //
//   const char* pThreshold = getenv("FIFO_THRESHOLD");
//   if (pThreshold) {
//     char* pEnd;
//     unsigned long newThreshold = strtoul(pThreshold, &pEnd, 0);
//     if (newThreshold && (pEnd != pThreshold)) {
//       m_fifoThreshold = newThreshold;
//     }
//   }
//   std::cerr << "Using a FIFO threshold of " << m_fifoThreshold << " words\n";
// }


CTriggerSRS::CTriggerSRS()
{
  last_trigg_update = std::chrono::steady_clock::now();
  // //  If FIFO_THRESHOLD is defined and is a positive integer, it replaces
  // //  the default value of m_fifoThreshold - the number of words that must be
  // //  in the fifo for the trigger to fire.
  // //
  // const char* pThreshold = getenv("FIFO_THRESHOLD");
  // if (pThreshold) {
  //   char* pEnd;
  //   unsigned long newThreshold = strtoul(pThreshold, &pEnd, 0);
  //   if (newThreshold && (pEnd != pThreshold)) {
  //     m_fifoThreshold = newThreshold;
  //   }
  // }
  // std::cerr << "Using a FIFO threshold of " << m_fifoThreshold << " words\n";
}

CTriggerSRS::~CTriggerSRS()
{
  // delete []m_wordsInEachModule;
}

void CTriggerSRS::setup()
{
    // Start the trigger timeout now:
    // m_lastTriggerTime = time(nullptr);
}

void CTriggerSRS::teardown()
{
    // called as data taking ends.  
    // DDAS does not need any further signal as data taking ends
    // since this function is also called on a pause of data taking
    // don't even think about desyncing modules here
}


// control for determing if trigger should poll modules or pass control back
// to CEventSegment for processing the previous block of data
void CTriggerSRS::Reset()
{
    // m_retrigger = false;
}

/* Receive the number of modules in the pixie16 setup from the event 
   segment class */
void CTriggerSRS::Initialize(int nummod)
{
    NumberOfModules = nummod;
    // m_retrigger = false;
    // delete []m_wordsInEachModule;
    // m_wordsInEachModule = new unsigned int[NumberOfModules]; 
}

//Simon - dummy just triggers every 3 secs...
bool CTriggerSRS::operator()() 
{ 

    // elapsed time since last update
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_trigg_update);

    // if 3 seconds have passed
    if (duration.count() >= 3) {
      last_trigg_update = now;
      // std::cout << "trigger!!!" << std::endl;
      return true;
    }
    else{
      return false;
    }



//     try{

//         /* If pixie16 is in the middle of processing a data buffer in the event 
//            segment, continue processing the data buffer.  Return a true trigger 
//            to pass control back the event segment. */
//         if(m_retrigger){
//             m_lastTriggerTime = time(nullptr);   // Reset the trigger timeout.
//             return true;
//         } else {
//             /* If there are no buffers currently being processed in the event segment 
//                look at the pixie16 hardware to see if data currently needs to be 
//                read out. */

//             // Read number of 32-bit words inside the FIFO on Pixie16
//             int retval = -1;
//             bool thresholdMade(false);
//             for (int i=0; i<NumberOfModules; i++) {
//                 // Check how many words are stored in Pixie16's readout FIFO
//                 ModNum = i;
//                 nFIFOWords = 0;
//                 retval = Pixie16CheckExternalFIFOStatus(&nFIFOWords,ModNum);
//                 if (retval < 0) {
//                   std::cerr << "Failed to read ExtFiFo status for module: "
//                     << ModNum;
//                     nFIFOWords = 0;                         // For safety.
//                 }
//                 m_wordsInEachModule[i] = nFIFOWords;       // Save words each module has.
                
//                 /* Trigger a read if the number of words in the external FIFO of 
//                    any pixie16 module in a crate exceeds a threshold defined in 
//                    pixie16app_defs.h */

//                 if(nFIFOWords > m_fifoThreshold){
// 		  // std::cout << "CTriggerSRS:: trig satisfied...mod=" << i
// 		  // 	    << " nwords=" << nFIFOWords << std::endl;
// #ifdef PRINTQUEINFO
// 		  std::cout << "CTriggerSRS:: trig satisfied...mod=" << i
// 			    << " nwords=" << nFIFOWords << std::endl;
// #endif
// 		  m_retrigger = true;
// 		  thresholdMade =  true;   // Once polling is done, trigger.
// 		  //std::cerr << "---- CTriggerSRS.cpp: m_retrigger " << m_retrigger << " thesholdMade: " << thresholdMade << " ---- " << std::endl;
//                 }
		
//             } // end module loop
//             if (thresholdMade) {
//               m_lastTriggerTime = time(nullptr);   // Record the trigger time.
//               return true;   // Some module was above threshold
//             }

//         }
        

//         // If the trigger has timed out, then trigger anyway:
        
//         time_t now = time(nullptr);
//         if ((now - m_lastTriggerTime)  > TRIGGER_TIMEOUT_SECS) {
//           m_lastTriggerTime = now;
//           return true;
//         }
//         return false;	// Currently not enough data to trigger

//     }
//     catch(...){
//         cout << "exception in trigger " << endl;
//     }
  // time_t now = time(nullptr);
  // if ((now - m_lastTriggerTime)  > TRIGGER_TIMEOUT_SECS) {
  //   m_lastTriggerTime = now;
  //   return true;
  // }
  // return false;
 // return false;
}



