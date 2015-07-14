#include "RingIOV10.h"

#include <CDataSource.h>
#include <CDataSink.h>
#include <V10/CRingItem.h>

#include <V10/DataFormatV10.h>
#include <byte_cast.h>

#include <iostream>

std::ostream& operator<<(std::ostream& stream,
                         const DAQ::V10::CRingItem& item)
{
  const char* pItem = reinterpret_cast<const char*>(item.getItemPointer());
  stream.write(pItem, item.size());
  return stream;
}


CDataSink& operator<<(CDataSink& sink,
                      const DAQ::V10::CRingItem& item)
{
  sink.put(item.getItemPointer(), item.size());

  return sink;
}

std::istream& operator>>(std::istream& stream,
                         DAQ::V10::CRingItem& item)
{
  size_t headerSize = 2*sizeof(uint32_t);

  char* pItem = reinterpret_cast<char*>(item.getItemPointer());
  stream.read(pItem, headerSize);

  uint32_t totalSize = byte_cast<uint32_t>(pItem);
  char* pBody = pItem + headerSize;
  stream.read(pBody, totalSize-headerSize);

  item.setBodyCursor(pItem + totalSize);
  item.updateSize();

  return stream;
}


CDataSource& operator>>(CDataSource& source,
                        DAQ::V10::CRingItem& item)
{
  size_t headerSize = 2*sizeof(uint32_t);

  char* pItem = reinterpret_cast<char*>(item.getItemPointer());
  source.read(pItem, headerSize);

  uint32_t totalSize = byte_cast<uint32_t>(pItem);
  char* pBody = pItem + headerSize;
  source.read(pBody, totalSize-headerSize);

  item.setBodyCursor(pItem + totalSize);
  item.updateSize();

  return source;
}
