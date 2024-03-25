/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Jeromy Tompkins
	     Aaron Chester
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file DDASRootEvent.h
 * @brief Defines a class to encapsulate the information in a built DDAS event.
 */ 

#ifndef DDASEVENT_H
#define DDASEVENT_H

#include <vector>

#include <TObject.h>

class DDASRootHit;

/**
 * @addtogroup libddaschannel libddaschannel.so
 * @brief DDAS data format for ROOT I/O e.g. produced by the ddasdumper 
 * program.
 * @{
 */

/** 
 * @class DDASRootEvent DDASRootEvent.h
 * @brief Encapsulates a Built DDAS event 
 *
 * @details
 * Any data that was written to disk downstream of the NSCLDAQ event
 * builder will have a "built" structure. What that means is that the body
 * of the physics event item will contain data from more than one DDAS 
 * event. The DDASRootEvent class represents this type of data. It provides
 * access to the events that make it up through the DDASRootHit objects
 * it owns and then also provides some useful methods for getting
 * data from the event as a whole.
 */
class DDASRootEvent : public TObject
{
private:
    /** Extensible array of primitive DDASRootHit objects. */
    std::vector<DDASRootHit*> m_data;

public:
    /** @brief Default constructor. */
    DDASRootEvent();
    /** @brief Copy constructor. */
    DDASRootEvent(const DDASRootEvent& obj);
    /** 
     * @brief Assignment operator.
     * @return Reference to the object.
     */
    DDASRootEvent& operator=(const DDASRootEvent& obj);
    /** @brief Destructor. */
    ~DDASRootEvent();
    /** 
     * @brief Access internal, extensible array of channel data.
     * @return Reference to the data.
     */ 
    std::vector<DDASRootHit*>& GetData() { return m_data;}
    /**
     * @brief Return the number of hits in this event.
     * @return The number of hits in the event (size of the event vector).
     */
    UInt_t GetNEvents() const { return m_data.size(); }
    /** 
     * @brief Append channel data to event.
     * @param channel Pointer to a DDASRootHit object.
     */
    void AddChannelData(DDASRootHit* channel);
    /** 
     * @brief Get timestamp of first channel datum.
     * @return Timestamp of the first element in the data vector.
     */
    Double_t GetFirstTime() const;
    /** 
     * @brief  Get timestamp of last channel datum.
     * @return Timestamp of the last element in the data vector.
     */
    Double_t GetLastTime() const;        
    /** @brief Get time difference between first and last channel data */
    Double_t GetTimeWidth() const;
    /** @brief Clear data vector and reset event */ 
    void Reset();

    ClassDef(DDASRootEvent, 1);
};

/** @} */

#endif
