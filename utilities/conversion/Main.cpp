#include "Main.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include "CTransformFactory.h"
#include "C8p0to10p0Mediator.h"
#include "C10p0to8p0Mediator.h"
#include "CTransform11p0to10p0.h"
#include "CTransform10p0to11p0.h"
#include "CTransformMediator.h"

using namespace std;
using namespace DAQ::Transform;

namespace DAQ {
  namespace V8 {
    std::size_t gBufferSize = 8192;
  }
}

Main::Main(int argc, char **argv)
  : m_pMediator()
{
  CDataSourceFactory sourceFactory;
  unique_ptr<CDataSource> pSource( sourceFactory.makeSource("file://./testin10.evt", {}, {}) );

  CDataSinkFactory factory;
  unique_ptr<CDataSink> pSink( factory.makeSink("file://./testout10p8.evt") );

  setUpTransformFactory();

  m_pMediator = m_factory.create(10, 8);

  m_pMediator->setDataSource(pSource);
  m_pMediator->setDataSink(pSink);
}


int Main::run()
{
  m_pMediator->initialize();

  m_pMediator->mainLoop();

  m_pMediator->finalize();

  return 0;
}

void Main::setUpTransformFactory()
{
  m_factory.setCreator(  8, 10,
                         unique_ptr<CTransformCreator>(new C8p0to10p0MediatorCreator()));
  m_factory.setCreator(  10, 8,
                         unique_ptr<CTransformCreator>(new C10p0to8p0MediatorCreator()));
  m_factory.setCreator( 10, 11,
                           unique_ptr<CTransformCreator>(new CGenericCreator<CTransform10p0to11p0>()));
  m_factory.setCreator( 11, 10,
                           unique_ptr<CTransformCreator>(new CGenericCreator<CTransform11p0to10p0>()));

}


int main (int argc, char** argv) 
{
  Main theMain(argc, argv);

  return theMain.run();
}
