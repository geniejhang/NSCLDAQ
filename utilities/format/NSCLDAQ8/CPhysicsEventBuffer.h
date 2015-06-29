#ifndef DAQ_V8_CPHYSICSEVENTBUFFER_H
#define DAQ_V8_CPHYSICSEVENTBUFFER_H

#include <CV8Buffer.h>
#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <DataFormatV8.h>


#include <cstdint>
#include <memory>

namespace DAQ {
  namespace V8 {

    class CRawBuffer;

    class CPhysicsEvent
    {
    public:
      using iterator       = DAQ::Buffer::BufferPtr<std::uint16_t>;

    private:
      bool                m_needsSwap;
      Buffer::ByteBuffer  m_buffer;

    public:
      CPhysicsEvent(const Buffer::ByteBuffer& data, bool needsSwap);
      CPhysicsEvent(Buffer::ByteBuffer&& data, bool needsSwap);

      /*!
       * \brief Copy constructor - copies deeply
       * \param rhs  - the event to copy from
       */
      CPhysicsEvent(const CPhysicsEvent& rhs);
      ~CPhysicsEvent();

      /*!
       * \brief Assignment operator
       *
       * This does a deep copy of the target object.
       *
       * \param rhs - object to copy
       * \return reference to this
       */
      CPhysicsEvent& operator=(const CPhysicsEvent& rhs);

      /*!
       * \brief Returns the total number of shorts in body
       *
       *  Different versions of the buffers follow different conventions
       *  for specifying the number of shorts in the body.
       *
       * \return
       */
      std::size_t getNTotalShorts() const;

      iterator begin() const;
      iterator end() const;

      Buffer::ByteBuffer& getBuffer() { return m_buffer;}
      const Buffer::ByteBuffer& getBuffer() const { return m_buffer;}
    };


    /*!
     * \brief Representation of the a DATABF type buffer
     *
     * This is intended to be used as a read only buffer. It supports
     * the establishment of its structure upon construction but then
     * you are mainly expected to only read from it. The main utilities
     * provided by this class is the ability to iterate over the events in
     * the body and then also to retrieve the buffer header. You can gain
     * read-only access to the data in the events through the CPhysicsEvent
     * class.
     *
     * Only standard buffer versions are supported and the version of the
     * 8.0 buffer that did not use inclusive word counts is not supported as
     * well. There is no way to know how to tell the difference with regard
     * to the latter usage of the data format.
     */
    class CPhysicsEventBuffer : public CV8Buffer
    {
    public:
      using Event          = typename std::shared_ptr<CPhysicsEvent>;
      using Body           = typename std::vector<Event >;
      using iterator       = typename Body::iterator;
      using const_iterator = typename Body::const_iterator;

    private:
      bheader m_header;
      Body    m_body;

    public:
      // Canonical methods
      CPhysicsEventBuffer() : m_header(), m_body() {}
      CPhysicsEventBuffer(const bheader& header,
                          const std::vector<std::uint16_t>& body);
      CPhysicsEventBuffer(const CRawBuffer& rawBuffer);
      CPhysicsEventBuffer(const CPhysicsEventBuffer& rhs);

      CPhysicsEventBuffer& operator=(const CPhysicsEventBuffer& rhs);
      ~CPhysicsEventBuffer();

      bheader getHeader() const;
      BufferTypes type() const { return DATABF; }

      // Access to the events
      iterator       begin();
      const_iterator begin() const;

      iterator       end();
      const_iterator end() const;

      std::size_t size() const { return m_body.size(); }
      Event at(std::size_t index) { return m_body.at(index); }


      void toRawBuffer(CRawBuffer &buffer) const;

     private:
      void parseBodyData(Buffer::ByteBuffer::const_iterator beg,
                         Buffer::ByteBuffer::const_iterator end);
      void parseStandardBody(Buffer::ByteBuffer::const_iterator beg,
                             Buffer::ByteBuffer::const_iterator end);
    };
    


  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CPHYSICSEVENTBUFFER_H
