#include "C10p0to8p0Mediator.h"
#include <NSCLDAQ8/DataFormatV8.h>
#include <NSCLDAQ10/DataFormatV10.h>
#include <NSCLDAQ8/format_cast.h>
#include <NSCLDAQ10/CRingItem.h>
#include <RingIOV10.h>
#include <BufferIOV8.h>

namespace DAQ {
  namespace Transform {
    

    std::unique_ptr<CBaseMediator> C10p0to8p0MediatorCreator::operator ()() const {
      return std::unique_ptr<CBaseMediator>(new C10p0to8p0Mediator());
    }


    C10p0to8p0Mediator::C10p0to8p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(move(source), move(sink)),
        m_transform()
    {
    }

    void C10p0to8p0Mediator::mainLoop()
    {

      int count=0;
      while (processOne()) {
        ++count;
      }

    }

    void C10p0to8p0Mediator::outputExtraTextBuffers(CDataSink& sink)
    {
      auto& buffers = m_transform.getStagedTextBuffers();
      for (auto & buffer : buffers) {
        sink << (V8::format_cast<V8::CRawBuffer>(buffer));
      }
      m_transform.clearStagedTextBuffers();
    }


    bool C10p0to8p0Mediator::processOne() {

      CDataSource& source = *getDataSource();
      CDataSink& sink = *getDataSink();

      NSCLDAQ10::CRingItem item1(NSCLDAQ10::VOID);
      source >> item1;

      if (source.eof()) {
        return false;
      }

      try {

        if ( typeDemandsFlush(item1.type()) && dataToFlush() ) {
          auto rawBuffer = V8::format_cast<V8::CRawBuffer>(m_transform.getCurrentPhysicsBuffer());
          sink << rawBuffer;

          m_transform.startNewPhysicsBuffer();
        }

        V8::CRawBuffer item2 = m_transform(item1);

        if (item2.getHeader().type == V8::RUNVARBF
            || item2.getHeader().type == V8::PKTDOCBF
            || item2.getHeader().type == V8::STATEVARBF
            || item2.getHeader().type == V8::PARAMDESCRIP ) {

          outputExtraTextBuffers(sink);
        }

        if (item2.getHeader().type != V8::VOID) {
          sink << item2;
        }
        //    }
      } catch (std::exception& exc) {
        std::cout << exc.what() << std::endl;
      } catch (...) {
        std::cout << "Caught an error" << std::endl;
      }

      return !source.eof();
    }

    bool C10p0to8p0Mediator::typeDemandsFlush(std::uint32_t v10type) const
    {
      return (v10type != NSCLDAQ10::PHYSICS_EVENT
          && v10type != NSCLDAQ10::EVB_FRAGMENT
          && v10type != NSCLDAQ10::EVB_UNKNOWN_PAYLOAD
          && v10type != NSCLDAQ10::PHYSICS_EVENT_COUNT);
    }

    bool C10p0to8p0Mediator::dataToFlush() const
    {
      std::size_t nEvents = m_transform.getCurrentPhysicsBuffer().size();
      return (nEvents > 0);
    }

  } // namespace Transform
} // namespace DAQ
