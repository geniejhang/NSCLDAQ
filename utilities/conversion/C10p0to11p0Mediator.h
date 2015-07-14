#ifndef DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
#define DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransformFactory.h>
#include <CTransformMediator.h>
#include <CTransform10p0to11p0.h>
#include <memory>

namespace DAQ {
  namespace Transform {
    
    class C10p0to11p0Mediator;

    /*!
     * \brief The C10p0to11p0MediatorCreator class
     */
    class C10p0to11p0MediatorCreator : public CTransformCreator {
    public:
      std::unique_ptr<CBaseMediator> operator()() const;
    };

    /*! \brief Decorate a 10 -> 11 transform mediator
     *
     * The only unique thing this does is spit out a data format item
     * before entering the main loop.
     *
     */
    class C10p0to11p0Mediator : public CBaseMediator
    {
      CTransformMediator<CTransform10p0to11p0> m_mediator;

    public:
      C10p0to11p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      virtual void mainLoop();

      virtual void initialize() { m_mediator.initialize();}
      virtual void finalize() { m_mediator.finalize();}

      virtual CDataSource* getDataSource() { return m_mediator.getDataSource(); }
      virtual CDataSink*   getDataSink()   { return m_mediator.getDataSink(); }

      virtual void setDataSource(std::unique_ptr<CDataSource> &pSource)
      { m_mediator.setDataSource(pSource); }

      virtual void setDataSink(std::unique_ptr<CDataSink> &pSink)
      { m_mediator.setDataSink(pSink); }

    private:
      void outputRingFormat();
    };
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
