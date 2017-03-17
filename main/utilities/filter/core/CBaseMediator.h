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



#ifndef DAQ_CBASEMEDIATOR_H
#define DAQ_CBASEMEDIATOR_H

#include <CDataSource.h>
#include <CDataSink.h>

#include <memory>

namespace DAQ {

class CBaseMediator
{
  protected:
    CDataSourcePtr m_pSource; //!< the source
    CDataSinkPtr   m_pSink; //!< the sink

  public:
    // The constructor
    CBaseMediator(CDataSourcePtr pSource = CDataSourcePtr(),
                  CDataSinkPtr pSink     = CDataSinkPtr());

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
    virtual CDataSourcePtr
    setDataSource( CDataSourcePtr source)
    {
        auto pOld = m_pSource;
        m_pSource = source;
        return pOld;
    }

    /**! Set the sink
      \param sink the new sink
    */
    virtual CDataSinkPtr
    setDataSink(CDataSinkPtr pSink)
    {
        auto pOld = m_pSink;
        m_pSink = pSink;
        return pOld;
    }

    /**! Access to the source 
    */
    virtual CDataSourcePtr getDataSource() { return m_pSource;}

    /**! Access to the sink 
    */
    virtual CDataSinkPtr getDataSink() { return m_pSink;}

};

} // end DAQ
#endif
