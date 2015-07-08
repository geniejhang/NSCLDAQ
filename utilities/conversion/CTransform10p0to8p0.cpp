#include "CTransform10p0to8p0.h"
#include <DataFormatV10.h>
#include <NSCLDAQ10/CRingItem.h>
#include <NSCLDAQ8/CRawBuffer.h>
#include <NSCLDAQ10/CRingScalerItem.h>
#include <NSCLDAQ8/CScalerBuffer.h>
#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ8/CControlBuffer.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ8/CPhysicsEventBuffer.h>
#include <NSCLDAQ10/CRingTextItem.h>
#include <NSCLDAQ8/CTextBuffer.h>
#include <NSCLDAQ8/format_cast.h>

namespace DAQ {
  namespace Transform {
    
    CTransform10p0to8p0::CTransform10p0to8p0()
    {
    }

    CTransform10p0to8p0::FinalType CTransform10p0to8p0::operator ()(const InitialType& item)
    {

      switch (item.type()) {
        case NSCLDAQ10::INCREMENTAL_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformIncrScaler(item));
          break;
        case NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformNonIncrScaler(item));
          break;
        case NSCLDAQ10::BEGIN_RUN:
        case NSCLDAQ10::END_RUN:
        case NSCLDAQ10::PAUSE_RUN:
        case NSCLDAQ10::RESUME_RUN:
          return V8::format_cast<V8::CRawBuffer>(transformStateChange(item));
          break;
        case NSCLDAQ10::PHYSICS_EVENT:
          return V8::format_cast<V8::CRawBuffer>(transformPhysicsEvent(item));
          break;
        case NSCLDAQ10::MONITORED_VARIABLES:
        case NSCLDAQ10::PACKET_TYPES:
          return V8::format_cast<V8::CRawBuffer>(transformText(item));
          break;
        case NSCLDAQ10::EVB_FRAGMENT:
        case NSCLDAQ10::EVB_UNKNOWN_PAYLOAD: // these do not transform.
          break;
      default:
          std::string errmsg("CTransform10p0to8p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      V8::bheader header;
      header.nwds = 16;
      header.nevt = 0;
      header.type = V8::VOID;
      header.ssignature = V8::BOM16;
      header.lsignature = V8::BOM32;
      Buffer::ByteBuffer buffer;
      buffer << header;
      DAQ::V8::CRawBuffer voidBuffer;
      voidBuffer.setBuffer(buffer);
      return voidBuffer;
    }


    V8::CScalerBuffer CTransform10p0to8p0::transformIncrScaler(const InitialType &item)
    {
      return V8::CScalerBuffer();
    }

    V8::CScalerBuffer CTransform10p0to8p0::transformNonIncrScaler(const InitialType &item)
    {
      return V8::CScalerBuffer();
    }

    V8::CControlBuffer CTransform10p0to8p0::transformStateChange(const InitialType &item)
    {
      return V8::CControlBuffer();
    }

    V8::CPhysicsEventBuffer CTransform10p0to8p0::transformPhysicsEvent(const InitialType &item)
    {
      return V8::CPhysicsEventBuffer();
    }

    V8::CTextBuffer CTransform10p0to8p0::transformText(const InitialType &item)
    {
      const NSCLDAQ10::CRingTextItem& v10item = dynamic_cast<const NSCLDAQ10::CRingTextItem&>(item);

      V8::bheader header;
      header.nwds = v10item.size()/sizeof(uint16_t);
      header.type = mapTextType(item.type());
      header.nevt = v10item.getStringCount();
      header.buffmt = V8::StandardVsn;
      header.ssignature = V8::BOM16;
      header.lsignature = V8::BOM32;
      header.run = m_run;
      header.seq = m_seq;
      header.cpu = 0;
      header.nbit = 0;
      header.nlam = 0;
      header.cks = 0;
      header.unused[0] = 0;
      header.unused[1] = 0;


      return V8::CTextBuffer(header, v10item.getStrings());
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
    
  } // namespace Transform
} // namespace DAQ
