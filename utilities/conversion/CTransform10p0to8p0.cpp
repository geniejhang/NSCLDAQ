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

namespace DAQ {
  namespace Transform {
    
    CTransform10p0to8p0::CTransform10p0to8p0()
    {
    }

    CTransform10p0to8p0::FinalType CTransform10p0to8p0::operator ()(const InitialType& item)
    {

      switch (item.type()) {
        case NSCLDAQ10::INCREMENTAL_SCALER:
          return transformIncrScaler(item);
          break;
        case NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS:
          return transformNonIncrScaler(item);
          break;
        case NSCLDAQ10::BEGIN_RUN:
        case NSCLDAQ10::END_RUN:
        case NSCLDAQ10::PAUSE_RUN:
        case NSCLDAQ10::RESUME_RUN:
          return transformStateChange(item);
          break;
        case NSCLDAQ10::PHYSICS_EVENT:
          return transformPhysicsEvent(item);
          break;
        case NSCLDAQ10::MONITORED_VARIABLES:
        case NSCLDAQ10::PACKET_TYPES:
          return transformText(item);
          break;
        case NSCLDAQ10::EVB_FRAGMENT:
        case NSCLDAQ10::EVB_UNKNOWN_PAYLOAD: // these do not transform.
          break;
      default:
          std::string errmsg("CTransform10p0to8p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.getHeader().type) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      return NSCLDAQ10::CRingItem(0);
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
      return V8::CTextBuffer();
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
    
  } // namespace Transform
} // namespace DAQ
