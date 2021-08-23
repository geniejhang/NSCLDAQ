/** @file:  scaledown
 *  @brief: reduces sampling rate of events of interest by a user selected factor
 */

#include "scaledownFunctions.h"
// standard runtime headers:
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <string>

// NSCLDAQ headers:
#include <CDataSource.h>                    
#include <CDataSink.h>                      
#include <CRingItem.h>                              
#include <Exception.h>  

/**
 *  This program transfers data between DataSources while reducing sampling rate of specified events.
 *  Both the initial and final DataSource can be either a RingBuffer or File (local or remote).
 *
 *    Program accepts three parameters: URI of DataSource, URI of DataSink, and scale factor
 */

int main(int argc, char** argv) {
    // Make sure we have the correct number of command line parameters.
    if (!argcCheck(argc)) {
        usage(std::cerr, "Incorrect number of command line parameters.");
    }

    // Helper functions process cmd line args to make source, sink, and reduction factor.
    CDataSource* pSrc = nullptr;
    CDataSink* pSink = nullptr;
    try {
        pSrc = createSource(argv[1]);
        pSink = createSink(argv[2]);
    } catch (CException& e) {
        if (pSrc == nullptr) {
            std::cerr << "Failed to open DataSource: ";
        } else {
            std::cerr << "Failed to make sink: ";
        }
        usage(std::cerr, e.ReasonText());
    }
    // Deletes dynamically allocated source once out of scope.
    std::unique_ptr<CDataSource> src(pSrc);
    std::unique_ptr<CDataSink> sink(pSink);

    int factor;
    try {
        factor = convertFactor(argv[3]);
    } catch (std::invalid_argument& e) {
        std::cerr << "Your factor input: " << argv[3] << std::endl;
        usage(std::cerr, "Unable to convert factor input to integer");
    } catch (std::runtime_error& e) {
        usage(std::cerr, e.what());
    }

    // The loop below consumes items from the ring buffer until all are used up
    CRingItem* pItem;
    int count = 0; // Used to track how many events of interest we have encountered
    while (pItem = src->getItem()) {
        std::unique_ptr<CRingItem> item(pItem);
        reduceSampling(*sink, *item, factor, count);
    }

    // We can only fall through here for file data sources... normal exit
    std::exit(EXIT_SUCCESS);
}
