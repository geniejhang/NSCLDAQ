/** @file:  scaledown
 *  @brief: Unit tests for reduce sampling script "scaledown.cpp"
 */

// NSCLDAQ headers:
#include <CDataSource.h>              
#include <CDataSourceFactory.h>       
#include <CDataSink.h>                
#include <CDataSinkFactory.h>
#include <CRingBuffer.h>         
#include <CRingItem.h>                
#include <DataFormat.h>                
#include <Exception.h>

// Custom/CppUnit headers
#define private public  // Trick allows us to temporarily define private methods as public
#include "scaledownMock.h"
#undef private
#include "scaledownFunctions.h"
#include "Asserts.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h> 
#include <cppunit/TestAssert.h>
#include <cppunit/Asserter.h>

// Standard headers:
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <string>
using std::string;
#include <stdio.h>
#include <cstring>

class scaledownTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(scaledownTests);
    CPPUNIT_TEST(fewArgs);
    CPPUNIT_TEST(manyArgs);
    CPPUNIT_TEST(checkSource);
    CPPUNIT_TEST(checkSink);
    CPPUNIT_TEST(checkFactor);
    CPPUNIT_TEST(normalScaledown);
    CPPUNIT_TEST(abnormalScaledown);
    CPPUNIT_TEST_SUITE_END();
    private:
        string ring_name = "TestRing";
        ScaledownMock* env;
    public:
        void setUp() {
            try {
                env = new ScaledownMock;
            } catch (...) {
                std::cerr << "Error making new ScaledownMock\n";
                std::exit(EXIT_FAILURE);
            }
        }    
        void tearDown() {
            try{
                env->cleanup(); // Removes temporary files created during construction
            } catch (string& s) {
                std::cerr << s;
                std::exit(EXIT_FAILURE);
            }
            delete env;
        }
    protected:
        void fewArgs(); // Tests outcome of too few cmd line args
        void manyArgs(); // Tests outcome of too many cmd line args
        void checkSource(); // Tests outcomes of attempting to make sources
        void checkSink(); // Tests outcomes of attempting to make sinks
        void checkFactor(); // Tests outcome of converting factor from string to int
        void normalScaledown(); // Tests reduceSampling with normal parameters
        void abnormalScaledown(); // Tests reduceSampling with abnormal parameters
};

CPPUNIT_TEST_SUITE_REGISTRATION(scaledownTests);

void scaledownTests::fewArgs() {
    std::vector<string> args = env->argv;
    ASSERTMSG("normal args", argcCheck(args.size()));
    args.pop_back();
    ASSERTMSG("too few args", !argcCheck(args.size()));
    ASSERTMSG("one arg", !argcCheck(1)); // argc must always be at least 1
}

void scaledownTests::manyArgs() {
    std::vector<string> args = env->argv;
    ASSERTMSG("normal args", argcCheck(args.size()));
    args.push_back("another arg");
    ASSERTMSG("one extra arg", !argcCheck(args.size()));
    ASSERTMSG("many extra args", !argcCheck(1000)); //simulates many args
}

void scaledownTests::checkSource() {
    try {
        CDataSource* a = createSource(env->argv[1]); // valid file source
        a = createSource("tcp://localhost/" + ring_name); // valid ring source
    } catch (...) {
        FAIL("valid source failure");
    }
    EXCEPTION("program name can't be source", createSource(env->argv[0]), CException);
    EXCEPTION("nonexistant ring", createSource("tcp://localhost/invalidring"), CException);
    EXCEPTION("nonexistant file", createSource("file://" + env->tempDir + "nonexistant_file"), CException);
    EXCEPTION("valid ring with invalid format", createSource("localhost/" + ring_name), CException);
    EXCEPTION("valid file with invalid format", createSource(env->srcString), CException);
}

void scaledownTests::checkSink() {
    try {
        CDataSink* b = createSink(env->argv[2]); // valid file sink
        b = createSink("tcp://localhost/" + ring_name); //valid ring sink
    } catch (...) {
        FAIL("valid sink failure");
    }
    EXCEPTION("program name can't be source", createSource(env->argv[0]), CException);
    EXCEPTION("nonexistant ring", createSource("tcp://localhost/myfakering"), CException);
    EXCEPTION("nonexistant file", createSource("file://" + env->tempDir + "fakesink.txt"), CException);
    EXCEPTION("valid ring with invalid format", createSource("localhost/" + ring_name), CException);
    EXCEPTION("valid file with invalid format", createSource(env->sinkString), CException);
}

void scaledownTests::checkFactor() {
    EQMSG("valid factor", 1, convertFactor(env->argv[3]));
    EQMSG("valid large factor", 999999, convertFactor("999999"));
    EXCEPTION("factor must be >= 1", convertFactor("0"), std::runtime_error);
    EXCEPTION("negative factor", convertFactor("-5"), std::invalid_argument);
    EXCEPTION("decimal factor", convertFactor("3.6"), std::invalid_argument);
    EXCEPTION("invalid symbols", convertFactor("7**5#4"), std::invalid_argument);
    EXCEPTION("not number", convertFactor("five"), std::invalid_argument);
}

void scaledownTests::normalScaledown() {
    try {
        env->instantiateVars(); // Sets up sink and factor mock vars and clears current data in sink
    } catch (...) {
        FAIL("instantiation failure");
    }
    int factor = env->reductionFactor;
    env->testSampling(factor, 500, 50, false); // recall that ceil(physEvents / factor) are transferred to sink
    EQMSG("half samples no distribution", 500, env->countSinkEvents()); // countSinkEvents counts all phys events in sink (cumulative)
    factor = 2;
    env->testSampling(factor, 5000, 777, true);
    EQMSG("half samples with distribution", 3000, env->countSinkEvents());
    factor = 5;
    env->testSampling(factor, 12345, 9384, false);
    EQMSG("many physics events small factor", 5469, env->countSinkEvents());
    factor = 42;
    env->testSampling(factor, 98765, 0, true);
    EQMSG("many physics events moderate factor", 7821, env->countSinkEvents());
    factor = 1024;
    env->testSampling(factor, 68423, 99999, false);
    EQMSG("more non-physics events large factor", 7888, env->countSinkEvents());
}

void scaledownTests::abnormalScaledown() {
    try {
        env->instantiateVars();
    } catch (...) {
        FAIL("instantiation failure");
    }
    int factor = env->reductionFactor;
    env->testSampling(factor, 0, 0, false);
    EQMSG("no data transferred", 0, env->countSinkEvents());
    env->testSampling(factor, 0, 500, true);
    EQMSG("only nonimportant events", 0, env->countSinkEvents());
    env->testSampling(factor, 10, 0, false);
    EQMSG("only physics events", 10, env->countSinkEvents());
    factor = 10000;
    env->testSampling(factor, 5642, 743, true);
    EQMSG("factor greater than events", 11, env->countSinkEvents());
    env->testSampling(factor, 10000, 44, false);
    EQMSG("factor equal to events", 12, env->countSinkEvents());
}
