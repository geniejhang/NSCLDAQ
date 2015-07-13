#ifndef MAIN_H
#define MAIN_H

#include <CTransformFactory.h>
#include <format_converter_options.h>

#include <memory>
#include <utility>

class CBaseMediator;

class Main
{
private:
  cmdLineOpts                       m_argsInfo;
  DAQ::Transform::CTransformFactory m_factory;
  std::unique_ptr<CBaseMediator>    m_pMediator;

public:
    Main(int argc, char** argv);

    int run();

private:
    std::unique_ptr<CDataSource> createSource() const;
    std::unique_ptr<CDataSink> createSink() const;
    void setUpTransformFactory();
    std::pair<int, int> parseInOutVersions() const;

};

#endif // MAIN_H
