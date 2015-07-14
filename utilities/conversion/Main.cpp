#include "Main.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include "CTransformFactory.h"
#include "C8p0to10p0Mediator.h"
#include "C10p0to8p0Mediator.h"
#include "C10p0to11p0Mediator.h"
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
  : m_pMediator(),
    m_argsInfo()
{

  cmdline_parser(argc, argv, &m_argsInfo);

  auto pSource = createSource();
  auto pSink = createSink();


  auto transformSpec = parseInOutVersions();
  setUpTransformFactory();

  m_pMediator = m_factory.create(transformSpec.first, transformSpec.second);
  m_pMediator->setDataSource(pSource);
  m_pMediator->setDataSink(pSink);
}


unique_ptr<CDataSource> Main::createSource() const
{
  CDataSourceFactory sourceFactory;
  unique_ptr<CDataSource> pSource( sourceFactory.makeSource(m_argsInfo.source_arg, {}, {}) );

  return move(pSource);
}

unique_ptr<CDataSink> Main::createSink() const
{
  CDataSinkFactory factory;
  unique_ptr<CDataSink> pSink( factory.makeSink(m_argsInfo.sink_arg) );

  return move(pSink);
}

std::pair<int, int> Main::parseInOutVersions() const
{
  int inputVsn  = std::atoi(cmdline_parser_input_version_values[m_argsInfo.input_version_arg]);
  int outputVsn = std::atoi(cmdline_parser_output_version_values[m_argsInfo.output_version_arg]);

  return std::make_pair(inputVsn, outputVsn);
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
                           unique_ptr<CTransformCreator>(new C10p0to11p0MediatorCreator()));
  m_factory.setCreator( 11, 10,
                           unique_ptr<CTransformCreator>(new CGenericCreator<CTransform11p0to10p0>()));

}


int main (int argc, char** argv) 
{
  Main theMain(argc, argv);

  return theMain.run();
}
