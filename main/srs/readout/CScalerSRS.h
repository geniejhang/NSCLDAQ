/*********************************************************
 Declaration of Scaler class for SRS (Skeleton for now)
*********************************************************/

#ifndef CSCALERSRS_H
#define CSCALERSRS_H

#include <config.h>
#include <CScaler.h>
#include <vector>

using namespace std;

class CScalerSRS : public CScaler
{
 private:
  vector<uint32_t> scalers;
 public:
  CScalerSRS(unsigned short moduleNr, unsigned short crateid); // Constructor
  ~CScalerSRS();
  virtual void initialize();
  virtual vector<uint32_t> read();
  virtual void clear();
  virtual void disable();
  
};

#endif
