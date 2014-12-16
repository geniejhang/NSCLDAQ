

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>
#include <CControlModule.h>
#include <CMockVMUSB.h>
#include <CLoggingReadoutList.h>

#define private public
#define protected public
#include <CWienerMDGG16.h>
#undef protected
#undef private

using namespace std;

class CWienerMDGG16Tests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CWienerMDGG16Tests);
    CPPUNIT_TEST(addWriteLogicalORMaskAB_0);
    CPPUNIT_TEST(addWriteLogicalORMaskCD_0);
    CPPUNIT_TEST(addReadLogicalORMaskAB_0);
    CPPUNIT_TEST(addReadLogicalORMaskCD_0);
    CPPUNIT_TEST(addWriteECLOutput_0);
    CPPUNIT_TEST(addReadECLOutput_0);
    CPPUNIT_TEST_SUITE_END();


  public:
    void setUp() {
    }
    void tearDown() {
    }

  protected:
    void addWriteLogicalORMaskAB_0();
    void addWriteLogicalORMaskCD_0();
    void addReadLogicalORMaskAB_0();
    void addReadLogicalORMaskCD_0();
    void addWriteECLOutput_0();
    void addReadECLOutput_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CWienerMDGG16Tests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CWienerMDGG16Tests::addWriteLogicalORMaskAB_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addWriteLogicalORMaskAB(list, 0xffff);

  vector<string> expected(1);
  expected[0] = string("addWrite32 fff000b8 39 65535");

  CPPUNIT_ASSERT(expected == list.getLog());
}


void CWienerMDGG16Tests::addWriteLogicalORMaskCD_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addWriteLogicalORMaskCD(list, 0xffff);

  vector<string> expected(1);
  expected[0] = string("addWrite32 fff000bc 39 65535");

  //print_vectors(expected, list.getLog());
  CPPUNIT_ASSERT(expected == list.getLog());
}

void CWienerMDGG16Tests::addReadLogicalORMaskAB_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addReadLogicalORMaskAB(list);

  vector<string> expected(1);
  expected[0] = string("addRead32 fff000b8 39");

  //print_vectors(expected, list.getLog());
  CPPUNIT_ASSERT(expected == list.getLog());
}


void CWienerMDGG16Tests::addReadLogicalORMaskCD_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addReadLogicalORMaskCD(list);

  vector<string> expected(1);
  expected[0] = string("addRead32 fff000bc 39");

  //print_vectors(expected, list.getLog());
  CPPUNIT_ASSERT(expected == list.getLog());
}


void CWienerMDGG16Tests::addWriteECLOutput_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addWriteECLOutput(list,0xff);

  vector<string> expected(1);
  expected[0] = string("addWrite32 fff0000c 39 255");

  //print_vectors(expected, list.getLog());
  CPPUNIT_ASSERT(expected == list.getLog());
}

void CWienerMDGG16Tests::addReadECLOutput_0() {
  CWienerMDGG16 dev;
  dev.setBase(0xfff00000);

  CLoggingReadoutList list;
  dev.addReadECLOutput(list);

  vector<string> expected(1);
  expected[0] = string("addRead32 fff0000c 39");

  //print_vectors(expected, list.getLog());
  CPPUNIT_ASSERT(expected == list.getLog());
}
