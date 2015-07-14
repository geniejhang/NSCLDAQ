#include "C10p0to11p0Mediator.h"
#include <CDataSink.h>
#include <V11/CDataFormatItem.h>

namespace DAQ {
  namespace Transform {
    
    std::unique_ptr<CBaseMediator> C10p0to11p0MediatorCreator::operator ()() const {
      return std::unique_ptr<CBaseMediator>(new C10p0to11p0Mediator());
    }


    C10p0to11p0Mediator::C10p0to11p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(std::unique_ptr<CDataSource>(), std::unique_ptr<CDataSink>()),
        m_mediator(std::move(source), std::move(sink))
    {
    }

    void C10p0to11p0Mediator::mainLoop()
    {

      outputRingFormat();

      m_mediator.mainLoop();
    }

    void C10p0to11p0Mediator::outputRingFormat()
    {
      CDataSink& sink = *m_mediator.getDataSink();
      sink << V11::CDataFormatItem();
    }

  } // namespace Transform
} // namespace DAQ
