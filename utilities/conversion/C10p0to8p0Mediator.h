#ifndef DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H
#define DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransformFactory.h>
#include <CTransform10p0to8p0.h>
#include <memory>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace Transform {
    class C10p0to8p0Mediator;

    /*!
     * \brief The C10p0to8p0MediatorCreator class
     */
    class C10p0to8p0MediatorCreator : public CTransformCreator {
    public:
      std::unique_ptr<CBaseMediator> operator()() const;
    };

    /*!
     *
     */
    class C10p0to8p0Mediator : public CBaseMediator
    {
      CTransform10p0to8p0 m_transform;

    public:
      C10p0to8p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      void initialize() {}

      void mainLoop();

      void finalize() {}

      bool processOne();

      void outputExtraTextBuffers(CDataSink& sink);
    private:
      bool typeDemandsFlush(std::uint32_t v10type) const;
      bool dataToFlush() const;
    };
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H
