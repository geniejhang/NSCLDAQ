/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/


#ifndef DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
#define DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransformFactory.h>
#include <CTransformMediator.h>
#include <CTransform10p0to11p0.h>
#include <memory>



namespace DAQ {
  namespace Transform {
    
    class C10p0to11p0Mediator;

    /*!
     * \brief The C10p0to11p0MediatorCreator class
     */
    class C10p0to11p0MediatorCreator : public CTransformCreator {
    public:
      std::unique_ptr<CBaseMediator> operator()() const;
    };



    /*! \brief Decorate a 10 -> 11 transform mediator
     *
     * The only unique thing this does is spit out a data format item
     * before entering the main loop. It is just a decorator pattern on top
     * of a standard CTransformMediator.
     *
     */
    class C10p0to11p0Mediator : public CTransformMediator<CTransform10p0to11p0>
    {

    public:
      /*!
       * \brief C10p0to11p0Mediator
       *
       * Simply passes on the sink and source to the actual transform mediator.
       *
       * \param source data source
       * \param sink   data sink
       */
      C10p0to11p0Mediator(std::shared_ptr<CDataSource> source = std::shared_ptr<CDataSource>(),
                         std::shared_ptr<CDataSink> sink = std::shared_ptr<CDataSink>());

      /*!
       * \brief mainLoop
       *
       *  OUtputs a ring data format item and then calls the main loop of the m_mediator
       */
      virtual void mainLoop();

      virtual void initialize() { CTransformMediator::initialize();}
      virtual void finalize() { CTransformMediator::finalize();}

    private:

      /*!
       * \brief Send a ring format item to the sink
       */
      void outputRingFormat();
    };
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
