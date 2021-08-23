// NSCLDAQ headers:
#include <CDataSource.h>              
#include <CDataSourceFactory.h>       
#include <CDataSink.h>                
#include <CDataSinkFactory.h>         
#include <CRingItem.h>                
#include <DataFormat.h>                
#include <Exception.h>                

// standard runtime headers:
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <string>
#include <algorithm>

/**
 * Usage:
 *    This outputs an error message that shows how the program should be used
 *     and exits using std::exit().
 *
 * @param o   - references the stream on which the error message is printed.
 * @param msg - Error message that precedes the usage information.
 */

void usage(std::ostream& o, const char* msg) {
    o << msg << std::endl;
    o << "Usage:\n";
    o << "  scaledown input-uri output-uri factor-int\n";
    o << "      input-uri - The file: or tcp: URI that describes where data comes from\n";
    o << "                   Note that the special value '-' makes the source get data from\n";
    o << "                   standard input.\n";
    o << "      output-uri - The file: or tcp: URI that describes where data will be written\n";
    o << "                   If the URI is a tcp: uri, the host part of the URI must either be\n";
    o << "                   empty or 'localhost\n";
    o << "                   Note that the special valu '-' makes the source put data to\n";
    o << "                   standard output\n";
    o << "      factor-string - The factor that details how events of interest will be reduced\n";
    o << "                   1 out of every X events will be transferred where X represents the factor\n";
    o << "                   Note that factor must be convertable to int and be greater than or equal to 1\n";
    std::exit(EXIT_FAILURE);
}

// Used to check proper number of command line args are input
bool argcCheck (int args) {
    return args == 4;
}

// Create the data source. Also specify item types that will be sampled and skipped
CDataSource* createSource(std::string uri) {
    std::vector<std::uint16_t> sample;  
    std::vector<std::uint16_t> exclude; 
    CDataSource* pDataSource;
    pDataSource = CDataSourceFactory::makeSource(uri, sample, exclude);
    return pDataSource;
}

// Create an output DataSink
CDataSink* createSink(std::string uri) {
    CDataSink* pDataSink;
    CDataSinkFactory factory;
    pDataSink = factory.makeSink(uri);
    return pDataSink;
}

// Converts reduction factor string to int and checks validity
int convertFactor(std::string factor) {
    std::string input = factor;
    int reduction_factor;
    if (std::find_if(input.begin(), input.end(), [] (char c) {return !std::isdigit(c);}) != input.end()) {
        throw std::invalid_argument("Input contains non-numeric chars"); //Ex: input of "7**8"
    }
    reduction_factor = std::stoi(input);
    if (reduction_factor < 1) {
        throw std::runtime_error("Factor must be int greater than or equal to 1");
    }
    return reduction_factor;
}

/**
 * reduceSampling.
 *    Modify this to put whatever event of interest you would like to reduce.
 *    In this case, we reduced the rate of PHYSICS_EVENTS that were 
 *    transferred to the output data source.
 *
 *  @param sink - references data sink to which data is written
 *  @param item - references the ring item we got.
 *  @param factor - only one of every X events of interest are transferred where 'X' is factor
 *  @param count - tracks how many events of interest have been processed.
 */

void reduceSampling(CDataSink& sink, CRingItem item, const int& factor, int& count) {
    if (item.type() != PHYSICS_EVENT) {  // All items that are not physics events auto sent to output
        sink.putItem(item);
    } else {
        if (count % factor == 0) {
            sink.putItem(item);
        }
        ++count;  // Count increases with each physics event encountere.
    }
}