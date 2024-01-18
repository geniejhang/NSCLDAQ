/**
 * @file CMyBusy.h
 * @brief Define a mock class that manages the busy for the DDAS readout 
 * framework.
 */

#ifndef CMYBUSY_H
#define CMYBUSY_H

#include <CBusy.h>

/**
 * @class CMyBusy
 * @details
 * A class to handle busy. This is used by the skeleton code to define busy as 
 * part of its trigger configuration. This class subclasses the abstract base 
 * class CBusy and implements its mandatory interface as a mock up.
 */

class CMyBusy : public CBusy
{
    // Constructors, destructors and other canonical operations: 
public:
    /** @brief Constructor. */
    CMyBusy() {};
    /** @breif Destructor. */
    ~CMyBusy() {} 
  
    // Selectors for class attributes:
public:
  
    // Mutators:
protected:  
  
    // Class operations:
public:
    /** @brief Called when data-taking is stopping. */
    virtual   void GoBusy () ;
    /** @brief Called when the system is able to accept a new trigger. */
    virtual   void GoClear ();
};

#endif
