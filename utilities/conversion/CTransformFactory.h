#ifndef CTRANSFORMFACTORY_H
#define CTRANSFORMFACTORY_H

#include <memory>

class CTransformFactory
{
public:
    CTransformFactory();

    void addCreator(std::unique_ptr<CTransformCreator> pCreator);
};

#endif // CTRANSFORMFACTORY_H
