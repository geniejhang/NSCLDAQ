#include "RingIOV11.h"

#include <CDataSource.h>
#include <CDataSink.h>
#include <NSCLDAQ11/CRingItem.h>
#include <byte_cast.h>

#include <DataFormatV11.h>

#include <iostream>

std::ostream& operator<<(std::ostream& stream,
                         const NSCLDAQ11::CRingItem& item)
{
  const char* pItem = reinterpret_cast<const char*>(item.getItemPointer());
  stream.write(pItem, item.size());
  return stream;
}


CDataSink& operator<<(CDataSink& sink,
                      const NSCLDAQ11::CRingItem& item)
{
  sink.put(item.getItemPointer(), item.size());

  return sink;
}


std::istream& operator>>(std::istream& stream,
                         NSCLDAQ11::CRingItem& item)
{
  size_t headerSize = 2*sizeof(uint32_t);

  char* pItem = reinterpret_cast<char*>(item.getItemPointer());
  stream.read(pItem, headerSize);

  uint32_t totalSize = item.size();
  pItem += headerSize;
  stream.read(pItem, totalSize-headerSize);

  return stream;
}


CDataSource& operator>>(CDataSource& source,
                        NSCLDAQ11::CRingItem& item)
{
  size_t headerSize = 2*sizeof(uint32_t);

  char* pItem = reinterpret_cast<char*>(item.getItemPointer());
  source.read(pItem, headerSize);

  uint32_t totalSize = byte_cast<uint32_t>(pItem);
  pItem += headerSize;
  source.read(pItem, totalSize-headerSize);

  return source;
}
