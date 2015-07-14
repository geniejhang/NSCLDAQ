#ifndef DAQ_V8_CPHYSICSEVENTBODYPARSER_H
#define DAQ_V8_CPHYSICSEVENTBODYPARSER_H

#include <V8/CPhysicsEventBuffer.h>
#include <ByteBuffer.h>
#include <memory>

namespace DAQ {
  namespace V8 {
    
    class CPhysicsEventBodyParser
    {
    public:
      virtual std::vector<std::shared_ptr<CPhysicsEvent> >
      operator()(std::size_t nEvents,
                 DAQ::Buffer::BufferPtr<std::uint16_t> beg,
                 DAQ::Buffer::BufferPtr<std::uint16_t> end) = 0;
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CPHYSICSEVENTBODYPARSER_H
