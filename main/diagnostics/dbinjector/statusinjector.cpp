#include "CParameters.h"
#include <iostream>
#include <zmq.hpp>
#include <CMultiAggregator.h>
#include <CStatusSubscription.h>
#include <CStatusDb.h>
#include <CSqlite.h>
#include <CSqliteTransaction.h>
#include <string>
#include <memory>
#include <thread>
#include <nsclzmq.h>

static void
aggregatorThread(CMultiAggregator& agg)
{
    agg();
}

int main(int argc, char** argv)
{
    // Parse the parameters.
    
    CParameters params(argc, argv);
    std::string service = params.service();
    std::string database = params.filename();
    
    // Create/run the multi node aggregation thread.   Fetch the URI
    // we need to subscribe to:
    
    CMultiAggregator aggregationThread(service.c_str(), 10);
    new std::thread(aggregatorThread, std::ref(aggregationThread));
    
    std::string subscribeTo = aggregationThread.getPublisherURI();
    
    // Connect to the subscription service from the multi aggregator and
    // make our subscriptions.  We want everything.
    
    ZmqSocket&  statusSocket( *(ZmqObjectFactory::createSocket(ZMQ_SUB)));
    statusSocket->connect(subscribeTo.c_str());
    CStatusSubscription subs(statusSocket);
    
    // Note empty type and severity lists subscribe to all.
    
    CStatusSubscription::RequestedTypes      types;
    CStatusSubscription::RequestedSeverities sevs;
    subs.subscribe(types, sevs);
    
    // Open the status database file:
    
    CStatusDb db(database.c_str(), CSqlite::readwrite | CSqlite::create);
    
    // Main loop.  Process messages  we're going to process them in batches
    // until we can't read any more without blocking.  The batch of
    // messages will be processed in a save point to improve the
    // efficiency of sqlite.
    
    while (1) {
        std::vector<zmq::message_t*> message;
        CStatusDefinitions::readMessage(message, statusSocket);
        
        // The block below is a bit awkward because we don't want to start
        // the savepoint until we have that first message in hand.  That
        // keeps the database unlocked as long as possible.
        
        {
            uint32_t eventFlag(0);
            size_t   evflagSize(sizeof(eventFlag));
            
            std::unique_ptr<CSqliteSavePoint> savept(
                db.savepoint("batchinject"));            // Begin 'transaction'.
                
            db.insert(message);
            CStatusDefinitions::freeMessage(message);
            
            statusSocket->getsockopt(ZMQ_EVENTS, &eventFlag, &evflagSize);
            while (eventFlag & ZMQ_POLLIN) {
                
                CStatusDefinitions::readMessage(message, statusSocket);
                db.insert(message);
                CStatusDefinitions::freeMessage(message);
                
                statusSocket->getsockopt(ZMQ_EVENTS, &eventFlag, &evflagSize);
            }
            
        }                                       // Savepoint commits here.
    }
}
