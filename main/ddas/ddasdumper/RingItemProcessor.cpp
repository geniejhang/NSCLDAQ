/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins
	     Aaron Chester
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** 
 * @file  RingItemProcessor.cpp
 * @brief Implementation of type-independent ring item processing.
 */

#include "RingItemProcessor.h"

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <ctime>

#include <CDataSink.h>
#include <CRingItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingStateChangeItem.h>
#include <CPhysicsEventItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CAbnormalEndItem.h>

// Map of timestamp policy codes to strings:

static std::map<CGlomParameters::TimestampPolicy, std::string> glomPolicyMap = {
    {CGlomParameters::first, "first"},
    {CGlomParameters::last, "last"},
    {CGlomParameters::average, "average"}
};

/**
 * @details
 * Construction without a data sink (nullptr). A sink must be set using 
 * the `setSink()` method if the class is constructed in this manner.
 */
RingItemProcessor::RingItemProcessor() : m_pSink(nullptr)
{}

/**
 * @details
 * The caller is responsible for handling cleanup of the sink.
 */
RingItemProcessor::RingItemProcessor(CDataSink* pSink) : m_pSink(pSink)
{}

/**
 * @details
 * Get scaler information from the item: timestamp, channel scaler values.
 */
void
RingItemProcessor::processScalerItem(CRingScalerItem& item)
{
    time_t ts = item.getTimestamp();
    std::cout << "Scaler item recorded " << ctime(&ts) << std::endl;
    for (size_t i = 0; i < item.getScalerCount(); i++) {
        std::cout << "Channel " << i << " had "
		  << item.getScaler(i) << " counts\n";
    }
}

/**
 * @details
 * We're just going to do a partial dump: item type, timestamp, run number,
 * title, elapsed time into the run at which the state change occured.
 */
void
RingItemProcessor::processStateChangeItem(CRingStateChangeItem& item)
{
    time_t tm = item.getTimestamp();
    std::cout << item.typeName() << " item recorded"
	      << " for run " << item.getRunNumber()
	      << " source ID " << item.getSourceId() << std::endl;
    std::cout << "Title: " << item.getTitle() << std::endl;
    std::cout << "Occured at: " << std::ctime(&tm) << " "
	      << item.getElapsedTime() << " sec. into the run\n";
}

/**
 * @details
 * Text items are items that contain documentation information in the form 
 * of strings. The currently defined text items are:
 *   - PACKET_TYPE - which contain documentation of any data packets that 
 *     might be present. This is used by the SBS readout framework.
 *   - MONITORED_VARIABLES - used by all frameworks to give the values of 
 *     tcl variables that are being injected during the run or are constant 
 *     throughout the run.
 * Again we format a dump of the item.
 */
void
RingItemProcessor::processTextItem(CRingTextItem& item)
{
    time_t tm = item.getTimestamp();
    std::cout << item.typeName() << " item recorded at "
	      << std::ctime(&tm) << " " << item.computeElapsedTime()
	      << " seconds into the run\n";
    std::cout << "Here are the recorded strings: \n";
    
    std::vector<std::string> strings = item.getStrings();
    for (size_t i = 0; i < strings.size(); i++) {
        std::cout << i << ": '" << strings[i] << "'\n";
    }
}

/**
 * @details
 * We want to write these to disk. Let the sink handle that by calling its 
 * `putItem()` method. Unpacking the data into whatever output data structure
 * is being used is the responsibility of the sink.
 */
void
RingItemProcessor::processPhysicsEventItem(CPhysicsEventItem& item)
{
    m_pSink->putItem(item);
}

/**
 * @details
 * Event count items are used to describe, for a given data source, the number 
 * of triggers that occured since the last instance of that item. This can be
 * used both to determine the rough event rate as well as the fraction of data
 * analyzed (more complicated for built events) in a program sampling physics
 * events. We'll dump out information about the item.
 */
void
RingItemProcessor::processPhysicsEventCountItem(
    CRingPhysicsEventCountItem& item
    )
{
    time_t tm = item.getTimestamp();
    std::cout << "Event count item";
    if (item.hasBodyHeader()) {
        std::cout << " from source id: " << item.getSourceId();
    }
    std::cout << std::endl;
    std::cout << "Emitted at: " << std::ctime(&tm) << " "
	      << item.computeElapsedTime() << " seconds into the run\n";
    std::cout << item.getEventCount() << " events since last one\n";
}

/**
 * @details
 * NSCLDAQ runs have, as their first record an event format record that 
 * indicates hat the data format (11.0, 12.0, etc.)
 */
void
RingItemProcessor::processFormatItem(CDataFormatItem& item)
{
    std::cout << "Data format is for: "
	      << item.getMajor() << "." << item.getMinor() << std::endl;
}

/**
 * @details
 * When the data source is the output of an event building pipeline, the glom 
 * stage of that pipeline will insert a glom parameters record into the output
 * data. This record type will indicate whether or not glom is building events
 * (or acting in passthrough mode) and the coincidence interval in clock ticks
 * used when in build mode, as well as how the timestamp is computed from the 
 * fragments that make up each event.
 */
void
RingItemProcessor::processGlomParameters(CGlomParameters& item)
{
    std::cout << "Event built data. Glom is: ";
    if (item.isBuilding()) {
        std::cout << "building with coincidece interval: "
		  << item.coincidenceTicks() << std::endl;
        std::cout << "Timestamp policy: "
		  << glomPolicyMap[item.timestampPolicy()] << std::endl;
    } else {
        std::cout << "operating in passthrough (non-building) mode\n";
    }
}

/**
 * @details
 * Rely on the item's `toString()` method to tell us what we need to know.
 */
void
RingItemProcessor::processAbnormalEndItem(CAbnormalEndItem& item)
{
    std::cout << item.toString() << std::endl;
    std::cout << "Run ended abnormally!" << std::endl;
}

/**
 * @details 
 * Process a ring item with an  unknown item type. This can happen if we're 
 * seeing a ring item that we've not specified a handler for (unlikely) or the
 * item types have expanded but the data format is the same (possible) or the
 * user has defined and is using their own ring item type. We'll just dump the
 * item.
 */
void
RingItemProcessor::processUnknownItemType(CRingItem& item)
{
    std::cout << item.toString() << std::endl;
}
