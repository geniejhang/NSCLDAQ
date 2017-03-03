/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";


#include <cppunit/extensions/HelperMacros.h>
#include <Asserts.h>

#include "CFilterVersionAbstractionFactory.h"

namespace DAQ {
class CDataSource;
class CDataSink;
}

using namespace DAQ;


class CFakeVersionAbstractionCreator;
using CFakeVersionAbstractionCreatorPtr = std::shared_ptr<CFakeVersionAbstractionCreator>;

class CFakeVersionAbstraction : public DAQ::CFilterVersionAbstraction
{
public:
    void readDatum(CDataSource& source) {}
    void processDatum() {}
    void outputDatum(CDataSink& sink) {}
    uint32_t getDatumType() const { return 0; }
    void cleanUp() {};
};

class CFakeVersionAbstractionCreator : public DAQ::CFilterVersionAbstractionCreator
{
    CFilterVersionAbstractionUPtr create() const {
        return CFilterVersionAbstractionUPtr(new CFakeVersionAbstraction);
    }
};


// A test suite
class CFilterVersionAbstractionFactoryTest : public CppUnit::TestFixture
{
  private:

  public:

    CPPUNIT_TEST_SUITE( CFilterVersionAbstractionFactoryTest );
    CPPUNIT_TEST ( addGetCreator_0 );
    CPPUNIT_TEST ( getCreator_0 );
    CPPUNIT_TEST ( create_0 );
    CPPUNIT_TEST ( create_1 );
    CPPUNIT_TEST_SUITE_END();

    void setUp()
    {}

    void tearDown()
    {}

    void addGetCreator_0()
    {
        CFilterVersionAbstractionCreatorPtr pCreator;
        CFilterVersionAbstractionFactory factory;

        factory.addCreator(100, pCreator);

        auto pFoundCreator = factory.getCreator(100);

        EQMSG("added and retrieved creators are same",
              pCreator.get(), pFoundCreator.get());
    }


    void getCreator_0()
    {
        CFilterVersionAbstractionFactory factory;

        auto pFoundCreator = factory.getCreator(100);

        EQMSG("nullptr is essentially returned when creator not found",
              CFilterVersionAbstractionFactory::CreatorPtr(nullptr),
              pFoundCreator);
    }

    void create_0()
    {
        CFakeVersionAbstractionCreatorPtr pCreator;
        CFilterVersionAbstractionFactory factory;

        factory.addCreator(100, pCreator);

        auto pAbstraction = factory.create(100);

        ASSERTMSG("Ensure that we can create a type", pAbstraction);
    }

    void create_1()
    {
        CFilterVersionAbstractionFactory factory;

        CPPUNIT_ASSERT_THROW_MESSAGE(
                    "creating item from creator that does not exists = fail",
                    factory.create(100),
                    std::out_of_range);

    }

};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFilterVersionAbstractionFactoryTest );

