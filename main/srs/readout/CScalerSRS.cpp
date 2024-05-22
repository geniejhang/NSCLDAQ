/*********************************************************
 Scaler class for SRS (Not implemented yet)
*********************************************************/

#include "CScalerSRS.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string.h>


/* Constructor */
CScalerSRS::CScalerSRS(unsigned short moduleNr, unsigned short crateid) 
{
  moduleNumber = moduleNr;
}

CScalerSRS::~CScalerSRS()
{

}

void CScalerSRS::initialize() 
{

}

void CScalerSRS::disable(){
  // modules do not need scalers disabled at end of run
}

vector<uint32_t> CScalerSRS::read()
{
  try {
    scalers.clear(); //SNL added for new readout
    return scalers;
  }
  catch(...){
    cout << "exception in scaler " << endl;
    return scalers;
  }
}

void CScalerSRS::clear() 
{

}

