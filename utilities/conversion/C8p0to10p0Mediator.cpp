#include "C8p0to10p0Mediator.h"
#include <NSCLDAQ10/CRingItem.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ8/CRawBuffer.h>
#include <CDataSource.h>
#include <CDataSink.h>
#include <RingIOV10.h>
#include <BufferIOV8.h>
#include <exception>
#include <DataFormatV8.h>
#include <DataFormatV10.h>

namespace DAQ {
  namespace Transform {
    

    std::unique_ptr<CBaseMediator> C8p0to10p0MediatorCreator::operator ()() const {
      return std::unique_ptr<CBaseMediator>(new C8p0to10p0Mediator());
    }


    C8p0to10p0Mediator::C8p0to10p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(move(source), move(sink)),
        m_transform()
    {
    }

    void C8p0to10p0Mediator::mainLoop()
    {

      int count=0;
      while (processOne()) {
        ++count;
      }

    }

    bool C8p0to10p0Mediator::processOne() {

      CDataSource& source = *getDataSource();
      CDataSink& sink = *getDataSink();

      V8::CRawBuffer item1;
      source >> item1;
      if (source.eof()) {
        return false;
      }
      //      updatePredicate();

      try {
        //    if ((*m_pPredicate)()) {

        ::DAQ::V10::CRingItem item2 = m_transform(item1);

        if (item2.type() == DAQ::V10::PHYSICS_EVENT) {
          auto& events = m_transform.getRemainingEvents();
          for (auto & event : events) {
            sink << DAQ::V10::CRingItem(event);
          }
        }
        if (item2.type() != 0) {
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

  } // namespace Transform
} // namespace DAQ
