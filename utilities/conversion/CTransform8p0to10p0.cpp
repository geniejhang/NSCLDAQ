#include "CTransform8p0to10p0.h"

#include <NSCLDAQ8/DataFormatV8.h>
#include <NSCLDAQ8/CRawBuffer.h>
#include <NSCLDAQ8/CScalerBuffer.h>
#include <NSCLDAQ8/CPhysicsEventBuffer.h>
#include <NSCLDAQ8/CControlBuffer.h>
#include <NSCLDAQ8/CTextBuffer.h>
#include <NSCLDAQ8/format_cast.h>

#include <NSCLDAQ10/CRingItem.h>
#include <NSCLDAQ10/CRingScalerItem.h>
#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ10/CRingTextItem.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>

#include <make_unique.h>

#include <string>
#include <stdexcept>
#include <chrono>

using namespace std;

namespace DAQ {
  namespace Transform {
    
    CTransform8p0to10p0::FinalType CTransform8p0to10p0::operator ()(const InitialType& item)
    {

      switch (item.getHeader().type) {
        case V8::SCALERBF:
        case V8::SNAPSCBF:
          return transformScaler(item);
          break;
        case V8::BEGRUNBF:
        case V8::ENDRUNBF:
        case V8::PAUSEBF:
        case V8::RESUMEBF:
          return transformControl(item);
          break;
        case V8::DATABF:
          return transformPhysicsEvent(item);
          break;
        case V8::STATEVARBF:
        case V8::RUNVARBF:
        case V8::PKTDOCBF:
        case V8::PARAMDESCRIP:
          return transformText(item);
          break;
      default:
          std::string errmsg("CTransform8p0to10p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.getHeader().type) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      return NSCLDAQ10::CRingItem(1);
    }
    

    NSCLDAQ10::CRingScalerItem CTransform8p0to10p0::transformScaler(const InitialType &item)
    {
      using namespace std::chrono;

      auto sclrBuf = V8::format_cast<V8::CScalerBuffer>(item);

      NSCLDAQ10::CRingScalerItem newItem(sclrBuf.getOffsetBegin(),
                                         sclrBuf.getOffsetEnd(),
                                         system_clock::to_time_t(system_clock::now()),
                                         sclrBuf.getScalers());

      return newItem;
    }

    NSCLDAQ10::CRingStateChangeItem CTransform8p0to10p0::transformControl(const InitialType &item)
    {
      auto ctlBuf = V8::format_cast<V8::CControlBuffer>(item);

      time_t tstamp      = convertToTime_t(ctlBuf.getTimeStruct());
      std::uint16_t run  = ctlBuf.getHeader().run;
      std::uint16_t type = ctlBuf.getHeader().type;

      NSCLDAQ10::CRingStateChangeItem v10item(mapControlType(type),
                                              run,
                                              ctlBuf.getOffset(),
                                              tstamp,
                                              ctlBuf.getTitle());

      return v10item;
    }

    NSCLDAQ10::CPhysicsEventItem CTransform8p0to10p0::transformPhysicsEvent(const InitialType &item)
    {
      m_physicsEvents.clear();

      auto evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(item);

      auto it = evtBuf.begin();
      auto end = evtBuf.end();
      while (it != end) {
        auto& pEvent = *it;

        transformOnePhysicsEvent(pEvent);
        ++it;
      }

      NSCLDAQ10::CPhysicsEventItem firstEvent( m_physicsEvents.front() );
      m_physicsEvents.erase(m_physicsEvents.begin());

      return firstEvent;
    }

    void CTransform8p0to10p0::transformOnePhysicsEvent(const std::shared_ptr<DAQ::V8::CPhysicsEvent>& pEvent)
    {
      // make sure that we construct a physics event big enough to handle any V8 buffer
      m_physicsEvents.emplace(m_physicsEvents.end(), NSCLDAQ10::PHYSICS_EVENT,
                              pEvent->getNTotalShorts()*sizeof(std::uint16_t));

      auto& v10Item = m_physicsEvents.back();
      char* pBody = reinterpret_cast<char*>(v10Item.getBodyPointer());
      auto v8Buffer = pEvent->getBuffer();
      pBody = std::copy(v8Buffer.begin(), v8Buffer.end(), pBody);
      v10Item.setBodyCursor(pBody);
      v10Item.updateSize();
    }

    NSCLDAQ10::CRingTextItem CTransform8p0to10p0::transformText(const InitialType &item)
    {
      auto textBuf = V8::format_cast<V8::CTextBuffer>(item);

      uint32_t v10type;
      std::uint16_t v8type = textBuf.getHeader().type;
      if (v8type == V8::STATEVARBF || v8type == V8::RUNVARBF) {
        v10type = NSCLDAQ10::MONITORED_VARIABLES;
      } else if (v8type == V8::PKTDOCBF) {
        v10type = NSCLDAQ10::PACKET_TYPES;
      } else {
        std::string errmsg("CTransform8p0to10p0::transformText() ");
        errmsg += "No known conversion of version 8 text type " + to_string(v8type) + " ";
        errmsg += "to a version 10 text type";
        throw std::runtime_error(errmsg);
      }

      NSCLDAQ10::CRingTextItem textItem(v10type, textBuf.getStrings());
      return textItem;
    }

    std::time_t CTransform8p0to10p0::convertToTime_t(const V8::bftime &tstruct) const
    {
      std::tm calTime;
      calTime.tm_mon  = tstruct.month;
      calTime.tm_mday = tstruct.day;
      calTime.tm_year = tstruct.year - 1900; // tm_year is number of years since 1900
      calTime.tm_hour = tstruct.hours;
      calTime.tm_min  = tstruct.min;
      calTime.tm_sec  = tstruct.sec;

      return std::mktime(&calTime);
    }

    std::uint16_t CTransform8p0to10p0::mapControlType(std::uint16_t type) const
    {
      std::uint16_t v10type;

      switch(type) {
        case V8::BEGRUNBF:
          v10type = NSCLDAQ10::BEGIN_RUN;
          break;
        case V8::ENDRUNBF:
          v10type = NSCLDAQ10::END_RUN;
          break;
        case V8::PAUSEBF:
          v10type = NSCLDAQ10::PAUSE_RUN;
          break;
        case V8::RESUMEBF:
          v10type = NSCLDAQ10::RESUME_RUN;
          break;
        default:
          throw std::runtime_error("CTransform8p0to10p0::mapControlType(std::uint16_t) unknown type provided");
          break;
      }

      return v10type;

    }
  } // namespace Transform
} // namespace DAQ
