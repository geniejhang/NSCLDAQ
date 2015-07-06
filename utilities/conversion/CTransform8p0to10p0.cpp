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
          errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
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
      return NSCLDAQ10::CRingItem(NSCLDAQ10::BEGIN_RUN);
    }

    NSCLDAQ10::CPhysicsEventItem CTransform8p0to10p0::transformPhysicsEvent(const InitialType &item)
    {
      auto evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(item);

      // make sure that we construct a physics event big enough to handle any V8 buffer
      NSCLDAQ10::CPhysicsEventItem v10item(NSCLDAQ10::PHYSICS_EVENT, V8::gBufferSize);

      char* pBody = reinterpret_cast<char*>(v10item.getBodyPointer());
      auto v8Buffer = evtBuf.at(0)->getBuffer();
      pBody = std::copy(v8Buffer.begin(), v8Buffer.end(), pBody);
      v10item.setBodyCursor(pBody);
      v10item.updateSize();
      return v10item;
    }

    NSCLDAQ10::CRingTextItem CTransform8p0to10p0::transformText(const InitialType &item)
    {
      auto textBuf = V8::format_cast<V8::CTextBuffer>(item);

      uint32_t v10type;
      V8::BufferTypes v8type = textBuf.getHeader().type;
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
  } // namespace Transform
} // namespace DAQ
