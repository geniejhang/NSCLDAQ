/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#ifndef CBASEMEDIATOR_H
#define CBASEMEDIATOR_H

#include <memory>
#include <CDataSource.h>
#include <CDataSink.h>

class CBaseMediator
{
  protected:
    std::shared_ptr<DAQ::CDataSource> m_pSource; //!< the source
    std::shared_ptr<DAQ::CDataSink>   m_pSink; //!< the sink

  public:
    // The constructor
    CBaseMediator(std::shared_ptr<DAQ::CDataSource> pSource = std::shared_ptr<DAQ::CDataSource>(),
                  std::shared_ptr<DAQ::CDataSink> pSink     = std::shared_ptr<DAQ::CDataSink>());

    virtual ~CBaseMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CBaseMediator(const CBaseMediator&) = delete;
    CBaseMediator& operator=(const CBaseMediator&) = delete;

  public:

    /**! The main loop
    *   
    *  This is to be defined by the derived class.
    *
    */
    virtual void mainLoop() = 0;

    /**! Initialize procedure 
     *
     *  Depending on the mediator, this may look different.
     *
     */
    virtual void initialize() = 0;

    /**! Finalization procedure.
     *
     *  Depending on the mediator, this may look different.
     *
     */
    virtual void finalize() = 0;

    /**! Set the source

      \param source the new source

    */
    virtual std::shared_ptr<DAQ::CDataSource>
    setDataSource( std::shared_ptr<DAQ::CDataSource> source)
    {
        auto pOld = m_pSource;
        m_pSource = source;
        return pOld;
    }

    /**! Set the sink
      \param sink the new sink
    */
    virtual std::shared_ptr<DAQ::CDataSink>
    setDataSink(std::shared_ptr<DAQ::CDataSink> pSink)
    {
        auto pOld = m_pSink;
        m_pSink = pSink;
        return pOld;
    }

    /**! Access to the source 
    */
    virtual std::shared_ptr<DAQ::CDataSource> getDataSource() { return m_pSource;}

    /**! Access to the sink 
    */
    virtual std::shared_ptr<DAQ::CDataSink> getDataSink() { return m_pSink;}

};

#endif
