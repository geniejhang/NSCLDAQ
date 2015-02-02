#ifndef __CFILEDATASOURCE_H
#define __CFILEDATASOURCE_H

#include <CRingItem.h>
#include <FileDataSource.h>

// Introduce the name only, we already instantiate this in 
// FileDataSource.cpp and it is compiled into data format

using CFileDataSource = NSCLDAQ::FileDataSource<CRingItem>;

#endif
