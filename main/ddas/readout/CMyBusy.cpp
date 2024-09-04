/**
 * @file CMyBusy.cpp
 * @brief Implement a busy class for DDAS. Entirely No-ops.
 */

#include "CMyBusy.h"

#include <iostream>

#include <config.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CMyBusy::CMyBusy() 
{ }

void CMyBusy::GoBusy() 
{
  //   cout << "Going busy "<< endl << flush;
}

void CMyBusy::GoClear() 
{  
  // cout << "going clear "<< endl << flush;
}
