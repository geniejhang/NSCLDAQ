#include "CPhysicsEventBuffer.h"
#include <ByteOrder.h>
#include <ByteBuffer.h>
#include <CStandardBodyParser.h>
#include <CRawBuffer.h>

namespace DAQ {
  namespace V8 {
    
    CPhysicsEvent::CPhysicsEvent(const Buffer::ByteBuffer &data, bool needsSwap)
      : m_needsSwap(needsSwap),
        m_buffer(data) {}

    CPhysicsEvent::CPhysicsEvent(Buffer::ByteBuffer&& data, bool needsSwap)
      : m_needsSwap(needsSwap),
        m_buffer( move(data) ) {}

    CPhysicsEvent::CPhysicsEvent(const CPhysicsEvent& rhs)
      : m_needsSwap(rhs.m_needsSwap),
        m_buffer(rhs.m_buffer) {}

    CPhysicsEvent::~CPhysicsEvent() {}

    CPhysicsEvent& CPhysicsEvent::operator=(const CPhysicsEvent& rhs)
    {
      if (this != &rhs) {
        m_needsSwap = rhs.m_needsSwap;
        m_buffer    = rhs.m_buffer;
      }
      return *this;
    }

    std::size_t CPhysicsEvent::getNTotalShorts() const {
      return *(m_buffer.begin());
    }

    CPhysicsEvent::iterator CPhysicsEvent::begin() const {
      return iterator(m_buffer.begin(), BO::CByteSwapper(m_needsSwap));
    }

    CPhysicsEvent::iterator CPhysicsEvent::end() const {
      return iterator(m_buffer.end(), BO::CByteSwapper(m_needsSwap));
    }

    ////////////////////////////////////////////////////////////////////////////

    CPhysicsEventBuffer::CPhysicsEventBuffer(const bheader &header,
                                             const std::vector<std::uint16_t>& body)
      : m_header(header),
        m_body()
    {
      Buffer::ByteBuffer buffer;
      buffer << body;
      parseBodyData(buffer.begin(), buffer.end());
    }

    CPhysicsEventBuffer::CPhysicsEventBuffer(const CRawBuffer &rawBuffer)
      : m_header(rawBuffer.getHeader()),
        m_body()
    {
      std::size_t hdrSize = 16*sizeof(std::uint16_t);
      auto buf = rawBuffer.getBuffer();
      parseBodyData(buf.begin()+hdrSize, buf.end());
    }

    CPhysicsEventBuffer::CPhysicsEventBuffer(const CPhysicsEventBuffer& rhs)
      : m_header(rhs.m_header),
        m_body()
    {
      // deep copy
      for (auto pEvt : rhs.m_body) {
        m_body.push_back(std::shared_ptr<CPhysicsEvent>(new CPhysicsEvent(*pEvt)));
      }
    }

    CPhysicsEventBuffer::~CPhysicsEventBuffer() {}

    CPhysicsEventBuffer& CPhysicsEventBuffer::operator=(const CPhysicsEventBuffer& rhs)
    {
      if (this != &rhs) {
        m_header = rhs.m_header;

        // deep copy
        std::vector<std::shared_ptr<CPhysicsEvent> > new_body;
        for (auto& pEvt : rhs.m_body) {
          new_body.push_back(std::shared_ptr<CPhysicsEvent>(new CPhysicsEvent(*pEvt)));
        }
        m_body = new_body;
      }
      return *this;
    }

    void CPhysicsEventBuffer::parseBodyData(Buffer::ByteBuffer::const_iterator beg,
                                            Buffer::ByteBuffer::const_iterator end)
    {
      if (m_header.buffmt == StandardVsn) {
        parseStandardBody(beg, end);
      } else {
        throw std::runtime_error("Only buffer version 5 is supported");
      }
    }

    void CPhysicsEventBuffer::parseStandardBody(Buffer::ByteBuffer::const_iterator beg,
                                                Buffer::ByteBuffer::const_iterator end)
    {
      CStandardBodyParser parser;
      Buffer::BufferPtr<uint16_t> begPtr(beg, m_header.mustSwap());
      Buffer::BufferPtr<uint16_t> endPtr(end, m_header.mustSwap());

      m_body = parser(m_header.nevt, begPtr, endPtr);
    }

    bheader CPhysicsEventBuffer::getHeader() const {
      return m_header;
    }
    
    CPhysicsEventBuffer::iterator CPhysicsEventBuffer::begin() 
    {
      return m_body.begin();
    }

    CPhysicsEventBuffer::const_iterator CPhysicsEventBuffer::begin() const
    {
      return m_body.begin();
    }

    CPhysicsEventBuffer::iterator CPhysicsEventBuffer::end() 
    {
      return m_body.end();
    }

    CPhysicsEventBuffer::const_iterator CPhysicsEventBuffer::end() const
    {
      return m_body.end();
    }

    void CPhysicsEventBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      Buffer::ByteBuffer newbuf;

      newbuf << m_header;

      for (auto& pEvent : m_body) {
        newbuf << pEvent->getBuffer();
      }

      buffer.setBuffer(newbuf);

     }
  } // namespace V8
} // namespace DAQ
