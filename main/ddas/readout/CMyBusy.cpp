/**
 * @file CMyBusy.cpp
 * @brief Implement mock busy class (it does nothing).
 */

#include "CMyBusy.h"

#include <iostream>

#include <config.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

void CMyBusy::GoBusy() 
{
    // cout << "Going busy "<< endl << flush;
}

void CMyBusy::GoClear() 
{  
    // cout << "going clear "<< endl << flush;
}

