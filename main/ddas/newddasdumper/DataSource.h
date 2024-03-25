/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef DATASOURCE_H
#define DATASOURCE_H

/** 
 * @file  DataSource.h
 * @brief Works with factories to provide a data source for undifferentiated 
 * ring items.
 */

class CRingItem;
class RingItemFactoryBase;

/**
 * @class DataSource
 * Pure abstract data source which uses a factory's ring item getters to
 * provide ring item from a data source. Since the factory provides this,
 * we'll need concrete classes:
 * - FdDataSource: give data from a file descriptor.
 * - StreamDataSource: give data from a stream.
 * @note Neither of these data sources supports reading directly from a ring 
 * buffer, as the format library is unaware of those NSCLDAQ classes. To read 
 * data from a ringbuffer you can create a file descriptor data source and 
 * read data from stdin i.e. ringselector | ddasdumper -.
 */

class DataSource {
protected:
    RingItemFactoryBase* m_pFactory; //!< Pointer to our ring item factory.
    
public:
    /** @brief Constructor. */
    DataSource(RingItemFactoryBase* pFactory);
    /** @brief Destructor. */
    virtual ~DataSource();
    /** 
     * @brief Pure-virtual method to access a ring item from the data source. 
     * Must be implemented in derived classes.
     * @return Pointer to the next ring item from the source.
     */
    virtual CRingItem* getItem() = 0;
    /** @brief Set a new factory. */
    void setFactory(RingItemFactoryBase* pFactory);
};


#endif
