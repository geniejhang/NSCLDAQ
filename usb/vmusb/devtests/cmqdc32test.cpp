
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CReadoutModule.h>
#include <CMockVMUSB.h>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>

#define private public
#define protected public
#include <CMQDC32RdoHdwr.h>
#undef protected
#undef private


// Fool the linker
//namespace Globals {
//  ::CConfiguration* pConfig;
//}

using namespace std;

class cmqdc32test : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(cmqdc32test);
  CPPUNIT_TEST( initialize_0 );
  CPPUNIT_TEST_SUITE_END();


private:
  CMQDC32RdoHdwr*  m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pConfig = new CReadoutModule("test", CMQDC32RdoHdwr());
    m_pModule = static_cast<CMQDC32RdoHdwr*>(m_pConfig->getHardwarePointer());
  }
  void tearDown() {
    delete m_pConfig;
  }
protected:
  void initialize_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cmqdc32test);

 
// On creating a module, attaching it to a configuration should
// cause the right configuration entries and defaults to be made.
//
// Readout list executed immediately should return no data since there are
// no triggers.
//
void cmqdc32test::initialize_0()
{
  std::string baseString = "0x80000000";
  m_pConfig->configure("-base", baseString);
  m_pConfig->configure("-ipl", "1");
  m_pConfig->configure("-vector", "0");
  m_pConfig->configure("-multievent", "on");
  m_pConfig->configure("-irqthreshold", "1");
  m_pConfig->configure("-gatemode", "common");
  m_pConfig->configure("-multlowerlimit0", "0");
  m_pConfig->configure("-multlowerlimit1", "0");
  m_pConfig->configure("-multupperlimit0", "32");
  m_pConfig->configure("-multupperlimit1", "16");
  m_pConfig->configure("-pulser", "on");

  CMockVMUSB ctlr;
  ctlr.addReturnDatum(1);
  ctlr.addReturnDatum(1);
  m_pModule->Initialize(ctlr);

  std::vector<std::string> ops = ctlr.getOperationRecord();
  cout.flags(ios::hex);
  copy(ops.begin(), ops.end(), ostream_iterator<std::string>(cout,"\n"));
  cout.flags(ios::dec);


  CPPUNIT_ASSERT_EQUAL(std::string("writeEventsPerBuffer(0x00000014)"), ops.at(0));

}
