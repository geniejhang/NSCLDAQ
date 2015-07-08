#ifndef DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H
#define DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransform8p0to10p0.h>
#include <CTransformFactory.h>

#include <memory>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace Transform {

    class C8p0to10p0Mediator;

    /*!
     * \brief The C8p0to10p0MediatorCreator class
     */
    class C8p0to10p0MediatorCreator : public CTransformCreator {
    public:
      std::unique_ptr<CBaseMediator> operator()() const;
    };

    /*!
     *
     */
    class C8p0to10p0Mediator : public CBaseMediator
    {
      CTransform8p0to10p0 m_transform;

    public:
      C8p0to10p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      void initialize() {}

      void mainLoop();

      void finalize() {}

      bool processOne();
    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H
