#ifndef CBUSYSRS_H
#define CBUSYSRS_H

#include <CBusy.h>

class CBusySRS : public CBusy
{
public:
	// Constructors, destructors and other cannonical operations: 
  
  CBusySRS();                      //!< Default constructor
  ~CBusySRS() { } //!< Destructor.
  
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:  
  virtual   void GoBusy () ;
  virtual   void GoClear ();
  //virtual   void ModuleClear();
};
#endif