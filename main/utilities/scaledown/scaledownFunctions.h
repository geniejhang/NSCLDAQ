#ifndef scaledownHelpers
#define scaledownHelpers

#include <string>
#include <memory>
#include <CDataSource.h>
#include <CDataSink.h>

void usage(std::ostream&, const char*);
bool argcCheck(int);
CDataSource* createSource(std::string);
CDataSink* createSink(std::string);
int convertFactor(std::string);
void reduceSampling(CDataSink&, CRingItem, const int&, int&);

#endif