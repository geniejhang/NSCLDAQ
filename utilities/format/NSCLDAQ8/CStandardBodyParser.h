#ifndef DAQ_V8_CSTANDARDBODYPARSER_H
#define DAQ_V8_CSTANDARDBODYPARSER_H

#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <CPhysicsEventBodyParser.h>

#include <memory>
#include <vector>
#include <utility>
#include <cstdint>

namespace DAQ {
  namespace V8 {

    class CStandardBodyParser : public CPhysicsEventBodyParser
    {
    public:
      std::vector<std::shared_ptr<CPhysicsEvent> >
      operator()(Buffer::BufferPtr<std::uint16_t> pos,
                 Buffer::BufferPtr<std::uint16_t> end);

       std::pair<std::shared_ptr<CPhysicsEvent>, Buffer::BufferPtr<std::uint16_t> >
        parseOne(Buffer::BufferPtr<std::uint16_t> beg,
                 Buffer::BufferPtr<std::uint16_t> deadend);
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CSTANDARDBODYPARSER_H
