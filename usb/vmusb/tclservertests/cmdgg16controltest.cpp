

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
#include <CMDGG16Control.h>
#undef protected
#undef private

using namespace std;

class CMDGG16ControlTests : public CppUnit::TestFixture {
  private:
    unique_ptr<CControlModule> m_pMod;

  public:
    CPPUNIT_TEST_SUITE(CMDGG16ControlTests);
    CPPUNIT_TEST(onAttach_0);

    CPPUNIT_TEST(initialize_0);
    CPPUNIT_TEST(initialize_1);

    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST(set_1);
    CPPUNIT_TEST(set_2);

    CPPUNIT_TEST(get_0);
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);

    CPPUNIT_TEST(readConfig_0);

    CPPUNIT_TEST_SUITE_END();


  public:
    void setUp() {
      // create the control hardware
      unique_ptr<CControlHardware> hdwr(new WienerMDGG16::CControlHdwr);

      // create control module and pass ownership of hardware to the 
      // CControlModule
      m_pMod.reset(new CControlModule("test", move(hdwr)) );
      m_pMod->configure("-base","0xff000000");
    }
    void tearDown() {
    }

  protected:
    void onAttach_0();
    void initialize_0();
    void initialize_1();

    void set_0();
    void set_1();
    void set_2();

    void get_0();
    void get_1();
    void get_2();

    void readConfig_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMDGG16ControlTests);


class FileJanitor {
  std::string m_path;
  
  public:
  FileJanitor(std::string path) : m_path(path) {}
  FileJanitor(const FileJanitor&) = delete;
  ~FileJanitor() {
    std::remove(m_path.c_str());
  }
};

void generateTestConfigFile (std::string path) {
  std::ofstream f(path.c_str());
  f << "or_a " << 255 << endl;
  f << "or_b " << 254 << endl;
  f << "or_c " << 253 << endl;
  f << "or_d " << 252 << endl;
  f.close();
}

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CMDGG16ControlTests::onAttach_0() {
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-base"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-mode"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_a"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_b"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_c"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_d"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-configfile"));
}

void CMDGG16ControlTests::initialize_0() {
  m_pMod->configure("-mode","explicit");
 
 
  m_pMod->configure("-or_a","255");
  m_pMod->configure("-or_b","0");
  m_pMod->configure("-or_c","1");
  m_pMod->configure("-or_d","2");

  CMockVMUSB ctlr;
  m_pMod->Initialize(ctlr);

  vector<string> expected = {
    "executeList::begin",
    "addWrite32 ff00000c 39 858993459", // (i.e. 0x33333333) for ECL outs
    "addWrite32 ff0000d0 39 1717973520", // (i.e. 0x66660000) for NIM outs
    "addWrite32 ff0000b8 39 255",
    "addWrite32 ff0000bc 39 131073",
    "executeList::end"};

  auto record = ctlr.getOperationRecord();

  //print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
  
}


void CMDGG16ControlTests::initialize_1() {
  generateTestConfigFile(".testfile.txt");
  FileJanitor janitor(".testfile.txt"); // to cleanup

  m_pMod->configure("-mode","file");
  m_pMod->configure("-configfile",".testfile.txt");

  CMockVMUSB ctlr;
  m_pMod->Initialize(ctlr);

  vector<string> expected = {
    "executeList::begin",
    "addWrite32 ff00000c 39 858993459", // (i.e. 0x33333333)
    "addWrite32 ff0000d0 39 1717973520", // (i.e. 0x66660000) for NIM outs
    "addWrite32 ff0000b8 39 16646399",
    "addWrite32 ff0000bc 39 16515325",
    "executeList::end"};

  auto record = ctlr.getOperationRecord();

  //print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
  
}

void CMDGG16ControlTests::set_0() 
{
  CMockVMUSB ctlr;

  m_pMod->Set(ctlr,"or_ab", "0xa0a0");

  vector<string> expected = {"executeList::begin",
                             "addWrite32 ff0000b8 39 41120",
                             "executeList::end"};

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected, record);
  
  CPPUNIT_ASSERT(expected == record);
}


void CMDGG16ControlTests::set_1() 
{
  CMockVMUSB ctlr;

  m_pMod->Set(ctlr,"or_cd", "0xa0a0");

  vector<string> expected = {"executeList::begin",
                             "addWrite32 ff0000bc 39 41120",
                             "executeList::end"};

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected, record);
  
  CPPUNIT_ASSERT(expected == record);
}


void CMDGG16ControlTests::set_2() 
{
  CMockVMUSB ctlr;


  CPPUNIT_ASSERT_THROW( m_pMod->Set(ctlr, "invalidparam", "0"),
                       std::string );

}


void CMDGG16ControlTests::get_0() 
{
  CMockVMUSB ctlr;

  std::string result = m_pMod->Get(ctlr,"or_ab");

  vector<string> expected = {"executeList::begin",
                             "addRead32 ff0000b8 39",
                             "executeList::end"};

  CPPUNIT_ASSERT(expected == ctlr.getOperationRecord());
}


void CMDGG16ControlTests::get_1() 
{
  CMockVMUSB ctlr;

  std::string result = m_pMod->Get(ctlr,"or_cd");

  vector<string> expected = {"executeList::begin",
                             "addRead32 ff0000bc 39",
                             "executeList::end"};

  CPPUNIT_ASSERT(expected == ctlr.getOperationRecord());
}

void CMDGG16ControlTests::get_2() 
{
  CMockVMUSB ctlr;

  CPPUNIT_ASSERT_THROW( m_pMod->Get(ctlr,"invalid param"),
                        std::string);
}
void CMDGG16ControlTests::readConfig_0() {

  generateTestConfigFile(".testfile.txt");

  // should delete test file when it goes out of scope
  FileJanitor janitor(".testfile.txt");

  using namespace WienerMDGG16;
  CControlHdwrState state = ConfigFileReader().parse(".testfile.txt");

  CPPUNIT_ASSERT_EQUAL(uint32_t(255), state.or_a);
  CPPUNIT_ASSERT_EQUAL(uint32_t(254), state.or_b);
  CPPUNIT_ASSERT_EQUAL(uint32_t(253), state.or_c);
  CPPUNIT_ASSERT_EQUAL(uint32_t(252), state.or_d);

}

