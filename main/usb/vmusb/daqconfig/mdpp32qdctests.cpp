// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <string>


#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CConfiguration.h"


namespace Globals {
  extern CConfiguration* pConfig;
  extern unsigned        scalerPeriod;
};

class MDPP32QDCTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MDPP32QDCTests);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    ::Globals::pConfig = new CConfiguration;
  }
  void tearDown() {
    delete ::Globals::pConfig;
    ::Globals::pConfig = 0;
  }
protected:
  void create();
  void config();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MDPP32QDCTests);

void MDPP32QDCTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mdpp32qdc create testing");
  std::string configString = pInterp->GlobalEval("mdpp32qdc cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void MDPP32QDCTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mdpp32qdc create testing -base 0x12340000");
  pInterp->GlobalEval("mdpp32qdc config testing -pulseramplitude 1000");
  std::string configString = pInterp->GlobalEval("mdpp32qdc cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString)); 
  EQ(std::string("1000"), getConfigVal(*pInterp, "-pulseramplitude", configString));
}
