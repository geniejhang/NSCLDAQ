#include "Main.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include "CTransformFactory.h"
#include "CTransform11p0to10p0.h"
#include "CTransform10p0to11p0.h"
#include "CTransformMediator.h"

using namespace std;

Main::Main(int argc, char **argv)
  : m_pMediator()
{
  CDataSourceFactory sourceFactory;
  unique_ptr<CDataSource> pSource( sourceFactory.makeSource("file://./testout.evt", {}, {}) );

  CDataSinkFactory factory;
  unique_ptr<CDataSink> pSink( factory.makeSink("file://./testout2.evt") );

  setUpTransformFactory();

  m_pMediator = xformFactory.create(10, 11);

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
  CTransformFactory xformFactory;
  xformFactory.setCreator( 10, 11,
                           unique_ptr<CTransformCreator>(new CGenericCreator<CTransform10p0to11p0>()));
  xformFactory.setCreator( 11, 10,
                           unique_ptr<CTransformCreator>(new CGenericCreator<CTransform11p0to10p0>()));

}


int main (int argc, char** argv) 
{
  Main theMain(argc, argv);

  return theMain.run();
}
