#include "CTransform10p0to8p0.h"
#include <DataFormatV10.h>
#include <NSCLDAQ10/CRingItem.h>
#include <NSCLDAQ8/CRawBuffer.h>
#include <NSCLDAQ10/CRingScalerItem.h>
#include <NSCLDAQ10/CRingTimestampedRunningScalerItem.h>
#include <NSCLDAQ8/CScalerBuffer.h>
#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ8/CControlBuffer.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ10/CRingPhysicsEventCountItem.h>
#include <NSCLDAQ10/CRingTextItem.h>
#include <NSCLDAQ10/CRingItemFactory.h>
#include <NSCLDAQ8/CTextBuffer.h>
#include <NSCLDAQ8/CVoidBuffer.h>
#include <NSCLDAQ8/format_cast.h>

#include <memory>

using namespace std;

namespace DAQ {
  namespace Transform {
    
    CTransform10p0to8p0::CTransform10p0to8p0()
      : m_run(0),
        m_nTriggersProcessed(0),
        m_samplingFactor(1.0),
        m_lastSequence(0),
        m_physicsBuffer(),
        m_textBuffers()
    {
       startNewPhysicsBuffer();
    }

    CTransform10p0to8p0::FinalType CTransform10p0to8p0::operator ()(const InitialType& item)
    {
      NSCLDAQ10::CRingItemFactory factory;
      std::unique_ptr<NSCLDAQ10::CRingItem> pItem(factory.createRingItem(item));

      switch (pItem->type()) {
        case NSCLDAQ10::INCREMENTAL_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformIncrScaler(*pItem));
          break;
        case NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformNonIncrScaler(*pItem));
          break;
        case NSCLDAQ10::BEGIN_RUN:
        case NSCLDAQ10::END_RUN:
        case NSCLDAQ10::PAUSE_RUN:
        case NSCLDAQ10::RESUME_RUN:
          return V8::format_cast<V8::CRawBuffer>(transformStateChange(*pItem));
          break;
        case NSCLDAQ10::PHYSICS_EVENT:
          return transformPhysicsEvent(*pItem);
          break;
        case NSCLDAQ10::MONITORED_VARIABLES:
        case NSCLDAQ10::PACKET_TYPES:
          return transformText(*pItem);
          break;
        case NSCLDAQ10::EVB_FRAGMENT:
        case NSCLDAQ10::EVB_UNKNOWN_PAYLOAD: // these do not transform.
          break;
        case NSCLDAQ10::PHYSICS_EVENT_COUNT:
          updateSamplingFactor(*pItem);
          break;
      default:
          std::string errmsg("CTransform10p0to8p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      Buffer::ByteBuffer buffer;
      buffer << V8::bheader();

      DAQ::V8::CRawBuffer voidBuffer;
      voidBuffer.setBuffer(buffer);
      return voidBuffer;
    }


    V8::CScalerBuffer CTransform10p0to8p0::transformIncrScaler(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const NSCLDAQ10::CRingScalerItem&>(item);

      V8::bheader header;
      header.type = V8::SCALERBF;
      header.nevt = v10item.getScalerCount();
      header.run  = m_run;
      header.seq  = computeSequence();
      V8::CScalerBuffer sclrBuf(header, v10item.getStartTime(), v10item.getEndTime(),
                                v10item.getScalers());
      return sclrBuf;
    }

    V8::CScalerBuffer CTransform10p0to8p0::transformNonIncrScaler(const InitialType &item)
    {

      auto& v10item = dynamic_cast<const NSCLDAQ10::CRingTimestampedRunningScalerItem&>(item);

      V8::bheader header;
      header.type = V8::SCALERBF;
      header.nevt = v10item.getScalerCount();
      header.run  = m_run;
      header.seq  = computeSequence();
      V8::CScalerBuffer sclrBuf(header, v10item.getOffsetStart(), v10item.getOffsetEnd(),
                                v10item.getScalers());
      return sclrBuf;

    }

    V8::CControlBuffer CTransform10p0to8p0::transformStateChange(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const NSCLDAQ10::CRingStateChangeItem&>(item);

      if (v10item.type() == NSCLDAQ10::BEGIN_RUN) {
        resetStatistics();
      }

      m_run = v10item.getRunNumber();

      V8::bheader header;
      header.type = mapControlType(v10item.type());
      header.run = m_run;
      header.seq = computeSequence();

      std::string title = v10item.getTitle();
      title.resize(80, ' ');
      title.at(79) = '\0';

      V8::CControlBuffer ctlBuf(header,
                                title,
                                v10item.getElapsedTime(),
                                V8::to_bftime(v10item.getTimestamp()));

      return ctlBuf;
    }

    V8::CRawBuffer CTransform10p0to8p0::transformPhysicsEvent(const InitialType &item)
    {
      const NSCLDAQ10::CPhysicsEventItem& v10item
                          = dynamic_cast<const NSCLDAQ10::CPhysicsEventItem&>(item);


      Buffer::ByteBuffer body = v10item.getBodyData();
      std::shared_ptr<V8::CPhysicsEvent> pEvent(new V8::CPhysicsEvent(body, v10item.mustSwap()));

      V8::CRawBuffer returnBuffer;
      if ( m_physicsBuffer.appendEvent(pEvent) ) {
        ++m_nTriggersProcessed;

        if (m_physicsBuffer.getNBytesFree() == 0) {
          returnBuffer = V8::format_cast<V8::CRawBuffer>(m_physicsBuffer);

          startNewPhysicsBuffer();
        } else {
          returnBuffer = V8::format_cast<V8::CRawBuffer>(V8::CVoidBuffer());
        }

      } else {
        returnBuffer = V8::format_cast<V8::CRawBuffer>(m_physicsBuffer);

        startNewPhysicsBuffer();
        m_physicsBuffer.appendEvent(pEvent);
        ++m_nTriggersProcessed;
      }

      return returnBuffer;
    }

    V8::CRawBuffer CTransform10p0to8p0::transformText(const InitialType &item)
    {
      const NSCLDAQ10::CRingTextItem& v10item = dynamic_cast<const NSCLDAQ10::CRingTextItem&>(item);

      auto strings = v10item.getStrings();

      appendNewTextBuffer(v10item.type());
      //for ( auto& str : strings ) {
      std::size_t nStr = strings.size();
      for (std::size_t i=0; i<nStr; ++i) {
        std::string& str = strings.at(i);

        auto& textBuf = m_textBuffers.back();

        if ( textBuf.appendString(str) ) { // insufficient space causes false to be returned
          if (textBuf.getNBytesFree() == 0) {
            appendNewTextBuffer(v10item.type());
          }

        } else {

          appendNewTextBuffer(item.type());
          m_textBuffers.back().appendString(str);
        }
      }

      const V8::CTextBuffer textBuf = m_textBuffers.front();
      m_textBuffers.erase(m_textBuffers.begin());

      V8::CRawBuffer rawBuf = V8::format_cast<V8::CRawBuffer>(textBuf);
      return rawBuf;
    }


    const V8::CPhysicsEventBuffer& CTransform10p0to8p0::getCurrentPhysicsBuffer() const
    {
      return m_physicsBuffer;
    }

    const std::vector<V8::CTextBuffer>& CTransform10p0to8p0::getStagedTextBuffers() const
    {
      return m_textBuffers;
    }

    void CTransform10p0to8p0::updateSamplingFactor(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const NSCLDAQ10::CRingPhysicsEventCountItem&>(item);
      double observedTriggers = static_cast<double>(m_nTriggersProcessed);
      double realTriggers     = static_cast<double>(v10item.getEventCount());
      if (realTriggers != 0) {
        m_samplingFactor = observedTriggers/realTriggers;
      } else {
        m_samplingFactor = 1.0; // realTriggers=0 means that we have no info to compute something else
      }
    }

    std::uint16_t CTransform10p0to8p0::mapControlType(std::uint16_t type) const
    {
      std::uint16_t v8type;

      switch(type) {
        case NSCLDAQ10::BEGIN_RUN:
          v8type = V8::BEGRUNBF;
          break;
        case NSCLDAQ10::END_RUN:
          v8type = V8::ENDRUNBF;
          break;
        case NSCLDAQ10::PAUSE_RUN:
          v8type = V8::PAUSEBF;
          break;
        case NSCLDAQ10::RESUME_RUN:
          v8type = V8::RESUMEBF;
          break;
        default:
          throw std::runtime_error("CTransform10p0to8p0::mapControlType(std::uint16_t) unknown type provided");
          break;
      }

      return v8type;

    }

    std::uint16_t CTransform10p0to8p0::mapTextType(std::uint16_t type) const
    {
      std::uint16_t v8type;

      switch(type) {
        case NSCLDAQ10::MONITORED_VARIABLES:
          v8type = V8::RUNVARBF;
          break;
        case NSCLDAQ10::PACKET_TYPES:
          v8type = V8::PKTDOCBF;
          break;
        default:
          throw std::runtime_error("CTransform10p0to8p0::mapTextType(std::uint16_t) unknown type provided");
          break;
      }

      return v8type;

    }
    
    void CTransform10p0to8p0::startNewPhysicsBuffer()
    {
      m_lastSequence = computeSequence();

      V8::bheader header;
      header.type = V8::DATABF;
      header.nevt = 0;
      header.run = m_run;
      header.seq = computeSequence();

      m_physicsBuffer = V8::CPhysicsEventBuffer(header,  Buffer::ByteBuffer({}));
    }

  std::uint32_t CTransform10p0to8p0::computeSequence() const
  {
    return m_nTriggersProcessed/m_samplingFactor;
  }

  void CTransform10p0to8p0::appendNewTextBuffer(std::uint16_t type) {
    V8::bheader header;
    header.type = mapTextType(type);
    header.run  = m_run;
    header.seq  = computeSequence();

    V8::CTextBuffer buffer(header, {});
    m_textBuffers.push_back(buffer);
  }

  void CTransform10p0to8p0::resetStatistics()
  {
    m_samplingFactor = 1.0;
    m_nTriggersProcessed = 0;
    m_lastSequence = 0;
  }
} // namespace Transform
} // namespace DAQ
