#ifndef MAIN_H
#define MAIN_H

//#include <CTransform11p0to10p0.h>
//#include <CTransformMediator.h>
#include <memory>

#include <CTransformFactory.h>

class CBaseMediator;

class Main
{
private:
  CTransformFactory m_factory;
  std::unique_ptr<CBaseMediator> m_pMediator;

public:
    Main(int argc, char** argv);

    int run();

private:
    void setUpTransformFactory();
};

#endif // MAIN_H
