#ifndef FORMAT_CAST_H
#define FORMAT_CAST_H

#include <CRawBuffer.h>
#include <CScalerBuffer.h>
#include <CPhysicsEventBuffer.h>
#include <CV8Buffer.h>

#include <typeinfo>

namespace DAQ
{
  namespace V8 {

    /*! \brief Cast operator for casting to or from a CRawBuffer (i.e. GENERIC)
     *
     */
    template<class T> T format_cast(const CV8Buffer& anyBuffer)
    {
      T newItem;

      if ((newItem.type() == GENERIC) ) {

        CRawBuffer buffer;
        buffer.setHeader(anyBuffer.getHeader());
        buffer.getBuffer() << anyBuffer;
        return buffer;

      } else {
        if (anyBuffer.type() == GENERIC) {

          CRawBuffer& rawBuffer = dynamic_cast<CRawBuffer&>(anyBuffer);

          if (newItem.type() == SCALERBF) {
            return CScalerBuffer(rawBuffer);
          } else if (newItem.type() == DATABF) {
            return CPhysicsEventBuffer(buffer);
          }
        } else {
          // cannot cast from a specific buffer to another specific buffer
          throw std::bad_cast();
        }
      }



      return new_item;
    }
  }
}

#endif // FORMAT_CAST_H
