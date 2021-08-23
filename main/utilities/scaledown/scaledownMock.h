#ifndef SCALEDOWN_MOCK
#define SCALEDOWN_MOCK

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <CDataSourceFactory.h>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CRingItem.h>
#include "scaledownFunctions.h"

class ScaledownMock {
    private:
        std::vector<std::string> argv;  // vector simulates argv arguments
        // tempDir defines where temp files are written
        std::string tempDir = "/tmp/", inTemplate = "input.XXXXXX", outTemplate = "output.XXXXXX";
        std::string srcString, sinkString; // string format of unique temp files
        char* srcFilename; 
        char* sinkFilename;
        int srcFd, sinkFd; // File descriptors for source and sink
        CDataSink* sink; // sink used for reduced sampling
        int reductionFactor; // factor used for reduced sampling
    public:
        ScaledownMock() {
            // Construction creates file source and sink based on template
            srcFilename = new char[(tempDir + inTemplate).size()];
            sinkFilename = new char[(tempDir + outTemplate).size()];
            srcFd = makeTemp(tempDir + inTemplate, srcFilename);
            sinkFd = makeTemp(tempDir + outTemplate, sinkFilename);
            srcString = srcFilename;
            sinkString = sinkFilename;
            // Instantiate simulated command line args
            argv = {"scaledown.cpp", "file://" + srcString, "file://" + sinkString, "1"}; // initializes psuedo argv
        }

        // Removes temp source and sink for teardown
        void cleanup() {
            if (unlink(srcFilename) == -1 || unlink(sinkFilename) == -1) {
                throw("Unable to remove temp files\n");
            }
        }

        // Make temp file based on directory template. Store result in char* "nameVar"
        int makeTemp(std::string template_, char*& nameVar) {
            int fd;
            strcpy(nameVar, (template_).c_str());
            if ((fd = mkstemp(nameVar)) == -1) {
                throw("mkstemp error");
            }
            return fd;
        }

        /// Sets up sink and factor for sampling test. Clears current data in sink
        void instantiateVars() {
            std::ofstream ofs;
            ofs.open(sinkString, std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            sink = createSink(argv[2]);
            reductionFactor = convertFactor(argv[3]);
        }

        /**
        * testSampling.
        *    This method passes CRingItems to sink while scaling down transfer rate.
        *    Primary goal is to test reduceSampling from scaledownFunctions.cpp
        *
        *  @param factor - factor of transfer reduction.
        *  @param phyEvents - number of physics events that can be passed to sink (given factor is 1).
        *  @param otherEvents - number of non-physics evens passed to sink.
        *  @param distribute - Determines delivery method of events to sink. Created two methods
        *                      to strengthen tests (in case one method falsely passed tests)
        *                * Distributed insert: non-physics events are evenly distributed throughout phys events
        *                * Block insert: All physics events added at once followd by all other events.     
        */

        void testSampling(const int& factor, const int& phyEvents, const int & otherEvents, bool distribute=true) {
            if (otherEvents < 1) {
                distribute = false;
            }

            int count = 0;
            reduceSampling(*sink, CRingItem(BEGIN_RUN), factor, count);
            if (distribute) {
                distributedInsert(factor, phyEvents, otherEvents, count);
            }   else {
                blockInsert(factor, phyEvents, otherEvents, count);
            }

            reduceSampling(*sink, CRingItem(END_RUN), factor, count);
        }

        void distributedInsert(const int& factor, const int& phyEvents, const int & otherEvents, int& count) {
            int i, j, events_per_cycle, remainder;
            events_per_cycle = phyEvents / otherEvents;
            remainder = phyEvents % otherEvents;
            for (i = 0; i < otherEvents; ++i) {
                for (j = 0; j < events_per_cycle; ++j) {
                    reduceSampling(*sink, CRingItem(PHYSICS_EVENT), factor, count);
                }
                reduceSampling(*sink, CRingItem(PERIODIC_SCALERS), factor, count);
            }
            for (i = 0; i < remainder; ++i) {
                reduceSampling(*sink, CRingItem(PHYSICS_EVENT), factor, count);
            }
        }

        void blockInsert(const int& factor, const int& phyEvents, const int & otherEvents, int& count) {
            int i;
            for (i = 0; i < phyEvents; ++i) {
                reduceSampling(*sink, CRingItem(PHYSICS_EVENT), factor, count);
            }
            for (i = 0; i < otherEvents; ++i) {
                reduceSampling(*sink, CRingItem(PERIODIC_SCALERS), factor, count);
            }
        }

        // Returns number of PHYSICS_EVENT in sink. Total is cumulative if not cleared.
        int countSinkEvents() {
            CDataSource* pDataSource = createSource(argv[2]);
            std::unique_ptr<CDataSource> source(pDataSource);
            CRingItem* pItem;
            int count = 0;
            while(pItem = source->getItem()) {
                std::unique_ptr<CRingItem> item(pItem);
                if (item->type() == PHYSICS_EVENT) {
                    ++count;
                }
            }
            return count; // SINK MUST BE FILE so that it can fall out of while loop
        }
};

#endif
