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
 * @file  RingItemProcessor.h
 * @brief Define a type-independent processing for NSCLDAQ ring items.
 */

#ifndef RINGITEMPROCESSOR_H
#define RINGITEMPROCESSOR_H

class CDataSink;
class CRingScalerItem;
class CRingStateChangeItem;
class CRingTextItem;
class CPhysicsEventItem;
class CRingPhysicsEventCountItem;
class CDataFormatItem;
class CGlomParameters;
class CAbnormalEndItem;
class CRingItem;

/**
 * @class RingItemProcessor
 * @brief Supports type-independent ring item processing.
 * 
 * @details
 * This is a pretty simple class consisting of one method for each type of 
 * ring item we want to process. The assumption is that users of the 
 * ddasdumper program want to convert PHYSICS_EVENT ring items into some 
 * output file format (a "data sink"). ddasdumper uses data sinks derived 
 * from the CDataSink abstract base classand uses the derived class' 
 * `putItem()` method to write ring items to the sink. In this way, the 
 * RingItemProcessor remains agnostic about the output data format, everything 
 * is encapsulated in the sink.
 *
 * @note It may be beneficial to abstract things at the processor level at 
 * some point. One could make this class contain virutal methods which do a 
 * textual dump of ring item types as default behavior. Derived classes 
 * implement what they want (including, probably, a sink).
 *
 * @note Could implement this as a plugin library and allow users to define 
 * their own output data format.
 */

class RingItemProcessor
{
private:
    CDataSink* m_pSink; //!< Data sink for output.
    
public:
    /** @brief Constructor. */
    RingItemProcessor();
    /** @brief Construct with a sink. */
    RingItemProcessor(CDataSink* pSink);
    /** @brief Destructor. */
    ~RingItemProcessor() {};

    /**
     * @brief Set a data sink.
     * @param pSink Pointer ot the data sink.
     * @note 
     */
    void setSink(CDataSink* pSink) { m_pSink = pSink; }
  
public:
    /**
     * @brief Output an abbreviated scaler dump to stdout.
     * @param item Reference to the scaler ring item to process.
     */
    void processScalerItem(CRingScalerItem& item);
    /**
     * @brief Output a state change item to stdout.
     * @param item Reference to the state change item.
     */
    void processStateChangeItem(CRingStateChangeItem& item);
    /**
     * @brief Output a text item to stdout. 
     * @param item References the CRingTextItem we got.
     */
    void processTextItem(CRingTextItem& item);
    /**
     * @brief Output a physics event item to the data sink.
     * @param item Reference the physics event item.
     */
    void processPhysicsEventItem(CPhysicsEventItem& item);
    /**
     * @brief Output an event count item to stdout.
     * @param item References the CPhysicsEventCountItem being dumped.
     */
    void processPhysicsEventCountItem(CRingPhysicsEventCountItem& item);
    /**
     * @brief Output the ring item format to stdout.
     * @param item References the format item.
     */
    void processFormatItem(CDataFormatItem& item);
    /**
     * @brief Output a glom parameters item to stdout. 
     *  @param item References the glom parameter record. 
     */    
    void processGlomParameters(CGlomParameters& item);
    /**
     * @brief Output an abnormal end run item to stdout.
     * @param item References the abnormal end run item.
     */
    void processAbnormalEndItem(CAbnormalEndItem& item);
    /**
     * @brief Output a ring item with unknown type to stdout.
     * @param item References the ring item for the event.
     */
    void processUnknownItemType(CRingItem& item);
    
};

#endif
