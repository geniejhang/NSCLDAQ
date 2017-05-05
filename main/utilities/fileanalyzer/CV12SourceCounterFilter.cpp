
#include "CV12SourceCounterFilter.h"

#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

namespace DAQ {
namespace V12 {

/*!
   * \brief Constructor
   *
   * \param outputFile
   */
  CSourceCounterFilter::CSourceCounterFilter(std::string outputFile)
: m_counters(), m_outputFile(outputFile), m_builtData(true)
{}

CSourceCounterFilter::~CSourceCounterFilter() 
{
}

// The default handlers
CRingItemPtr CSourceCounterFilter::handleRingItem(CRingItemPtr pItem)
{
  return handleItem(pItem);
}

CRingStateChangeItemPtr CSourceCounterFilter::handleStateChangeItem(CRingStateChangeItemPtr pItem) 
{
    return handleItem(pItem);
}

CRingScalerItemPtr CSourceCounterFilter::handleScalerItem(CRingScalerItemPtr pItem) 
{
    return handleItem(pItem);
}

CRingTextItemPtr CSourceCounterFilter::handleTextItem(CRingTextItemPtr pItem) 
{
    return handleItem(pItem);
}

CPhysicsEventItemPtr CSourceCounterFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem) 
{
    return handleItem(pItem);
}

CRingPhysicsEventCountItemPtr
CSourceCounterFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
    return handleItem(pItem);
}

CDataFormatItemPtr
CSourceCounterFilter::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    return handleItem(pItem);
}


CAbnormalEndItemPtr
CSourceCounterFilter::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    return handleItem(pItem);
}

CCompositeRingItemPtr
CSourceCounterFilter::handleCompositeItem(CCompositeRingItemPtr pItem)
{
    // this will only go one  layer deep rather than traversing the entire tree.
    // the pooint of the file analyzer program is to determine what the appropriate
    // number of end runs and source ids exists for setting up an event builder.
    // Since in event building there is only ever 1 layer of event building done,
    // there is no need to keep track of children of children.

    for (auto pChild : *pItem) {
        incrementCounter(pChild->getSourceId(), pChild->type());
    }

    return pItem;
}


bool CSourceCounterFilter::counterExists(uint32_t type) 
{
  map<uint32_t,std::map<uint32_t,uint32_t> >::iterator it;
  it = m_counters.find(type);
  return ( it!=m_counters.end() );

}

void CSourceCounterFilter::printCounters(std::ostream& stream) const
{
  map<uint32_t,map<uint32_t,uint32_t> >::const_iterator it,itend;
  map<uint32_t,uint32_t>::const_iterator idit,iditend;
  it = m_counters.begin();
  itend = m_counters.end();

  stream << "set sourceMap {";
  while (it != itend) {
    idit = it->second.begin();
    iditend = it->second.end();
    stream << it->first << " {";
    while (idit!=iditend) {
      stream << translate(idit->first) 
           << " " << idit->second 
           << " ";
      ++idit;
    }
    stream << "} ";
    ++it;
  }
  stream << "}";
}


string CSourceCounterFilter::translate(uint32_t type) const
{
  map<uint32_t,string> namemap;
  namemap[BEGIN_RUN]            = "BEGIN_RUN";
  namemap[END_RUN]              = "END_RUN";
  namemap[PAUSE_RUN]            = "PAUSE_RUN";
  namemap[RESUME_RUN]           = "RESUME_RUN";
  namemap[PACKET_TYPES]         = "PACKET_TYPES";
  namemap[MONITORED_VARIABLES]  = "MONITORED_VARIABLES";
  namemap[RING_FORMAT]          = "RING_FORMAT";
  namemap[PERIODIC_SCALERS]     = "PERIODIC_SCALERS";
  namemap[PHYSICS_EVENT]        = "PHYSICS_EVENT";
  namemap[PHYSICS_EVENT_COUNT]  = "PHYSICS_EVENT_COUNT";
  namemap[EVB_GLOM_INFO]        = "EVB_GLOM_INFO";
  namemap[ABNORMAL_ENDRUN]      = "ABNORMAL_ENDRUN";
  namemap[COMP_BEGIN_RUN]            = "COMP_BEGIN_RUN";
  namemap[COMP_END_RUN]              = "COMP_END_RUN";
  namemap[COMP_PAUSE_RUN]            = "COMP_PAUSE_RUN";
  namemap[COMP_RESUME_RUN]           = "COMP_RESUME_RUN";
  namemap[COMP_PACKET_TYPES]         = "COMP_PACKET_TYPES";
  namemap[COMP_MONITORED_VARIABLES]  = "COMP_MONITORED_VARIABLES";
  namemap[COMP_RING_FORMAT]          = "COMP_RING_FORMAT";
  namemap[COMP_PERIODIC_SCALERS]     = "COMP_PERIODIC_SCALERS";
  namemap[COMP_PHYSICS_EVENT]        = "COMP_PHYSICS_EVENT";
  namemap[COMP_PHYSICS_EVENT_COUNT]  = "COMP_PHYSICS_EVENT_COUNT";
  namemap[COMP_EVB_GLOM_INFO]        = "COMP_EVB_GLOM_INFO";
  namemap[COMP_ABNORMAL_ENDRUN]      = "COMP_ABNORMAL_ENDRUN";

  map<uint32_t,string>::const_iterator it;
  it = namemap.find(type);
  if ( it!=namemap.end() ) {
     return it->second;
  } else {
    stringstream name;
    name << "User type #" << type;
    return name.str();
  }


}

void CSourceCounterFilter::finalize() 
{
  std::ofstream dump_file(m_outputFile.c_str());
  printCounters(dump_file);
}

void CSourceCounterFilter::incrementCounter(uint32_t id, uint32_t type) 
{
    // avoid any concern with the composite bit
    id = (0x7fff & id);
    if (!counterExists(id)) {
      setupCounters(id);
    }
    m_counters[id][type] += 1;
}

void CSourceCounterFilter::setupCounters(uint32_t id) 
{
  m_counters[id][BEGIN_RUN]           = 0;
  m_counters[id][END_RUN]             = 0;
  m_counters[id][PAUSE_RUN]           = 0;
  m_counters[id][RESUME_RUN]          = 0;

  m_counters[id][PACKET_TYPES]        = 0;
  m_counters[id][MONITORED_VARIABLES] = 0;
  m_counters[id][RING_FORMAT]         = 0;

  m_counters[id][PERIODIC_SCALERS]    = 0;

  m_counters[id][PHYSICS_EVENT]       = 0;
  m_counters[id][PHYSICS_EVENT_COUNT] = 0;

  m_counters[id][EVB_GLOM_INFO]       = 0;

  m_counters[id][ABNORMAL_ENDRUN]     = 0;

  m_counters[id][COMP_BEGIN_RUN]           = 0;
  m_counters[id][COMP_END_RUN]             = 0;
  m_counters[id][COMP_PAUSE_RUN]           = 0;
  m_counters[id][COMP_RESUME_RUN]          = 0;

  m_counters[id][COMP_PACKET_TYPES]        = 0;
  m_counters[id][COMP_MONITORED_VARIABLES] = 0;
  m_counters[id][COMP_RING_FORMAT]         = 0;

  m_counters[id][COMP_PERIODIC_SCALERS]    = 0;

  m_counters[id][COMP_PHYSICS_EVENT]       = 0;
  m_counters[id][COMP_PHYSICS_EVENT_COUNT] = 0;

  m_counters[id][COMP_EVB_GLOM_INFO]       = 0;

  m_counters[id][COMP_ABNORMAL_ENDRUN]     = 0;
}


} // end V12
} // end DAQ
