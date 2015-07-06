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
                                             const std::vector<std::uint16_t>& body,
                                             bool mustSwap)
      : m_header(header),
        m_body(),
        m_mustSwap(mustSwap)
    {
      Buffer::ByteBuffer buffer;
      buffer << body;
      parseBodyData(buffer.begin(), buffer.end());
    }

    CPhysicsEventBuffer::CPhysicsEventBuffer(const CRawBuffer &rawBuffer)
      : m_header(rawBuffer.getHeader()),
        m_body(),
        m_mustSwap(rawBuffer.bufferNeedsSwap())
    {
      if (m_header.type != DATABF) {
        std::string errmsg = "CPhysicsEventBuffer::CPhysicsEventBuffer(CRawBuffer const&) ";
        errmsg += "Buffer is not of type DATABF!";
        throw std::runtime_error(errmsg);
      }

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
      Buffer::BufferPtr<uint16_t> begPtr(beg, m_mustSwap);
      Buffer::BufferPtr<uint16_t> endPtr(end, m_mustSwap);

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

    // because I don't know how to properly swap the body of a physics event, I
    // have to send the entire buffer back unswapped.
    void CPhysicsEventBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      Buffer::ByteBuffer newbuf;

      bheader header = m_header;
//      std::uint16_t nShorts = 16; // size of bheader
//      for (auto& pEvent : m_body) {
//        nShorts += pEvent->getNTotalShorts();
//      }
//      header.nwds = nShorts;
//      header.nevt = m_body.size();

      if (m_mustSwap) {
        swapBytesOfHeaderInPlace(header);
      }
      newbuf << header;

      for (auto& pEvent : m_body) {
        newbuf << pEvent->getBuffer();
      }

      buffer.setBuffer(newbuf);

     }

    void CPhysicsEventBuffer::swapBytesOfHeaderInPlace(bheader &header) const
    {
      BO::swapBytes(header.nwds);
      BO::swapBytes(header.type);
      BO::swapBytes(header.cks);
      BO::swapBytes(header.run);
      BO::swapBytes(header.seq);
      BO::swapBytes(header.nevt);
      BO::swapBytes(header.nlam);
      BO::swapBytes(header.cpu);
      BO::swapBytes(header.nbit);
      BO::swapBytes(header.buffmt);
      BO::swapBytes(header.ssignature);
      BO::swapBytes(header.lsignature);
      BO::swapBytes(header.unused[0]);
      BO::swapBytes(header.unused[1]);
    }

  } // namespace V8
} // namespace DAQ
