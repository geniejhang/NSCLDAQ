/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef DAQ_CPREDICATE_H
#define DAQ_CPREDICATE_H

#include <memory>
#include <CPredicatedMediator.h>

namespace DAQ {

class CPredicate;

// Userful typedefs
using CPredicatePtr = std::shared_ptr<CPredicate>;

/*!
 * \brief The CPredicate class
 *
 * The CPredicate class is for use in a CPredicatedMediator (i.e. CFilterMediator).
 * The predicates are used to determine whether or not the filter program should
 * continue processing.
 *
 * Predicate operations return a specific action: CONTINUE, ABORT, or SKIP.
 *
 * The order that the methods will be called in is:
 * 1. preInputUpdate
 * 2. postInputUpdate
 * 3. preOutputUpdate
 * 4. postOutputUpdate
 */
class CPredicate
{
public:
    /*!
     * \brief Evaluate prior to reading a data item
     *
     * \param transform     the mediator
     *
     * \return the action to be taken
     */
    virtual CPredicatedMediator::Action preInputUpdate(CPredicatedMediator& transform) = 0;

    /*!
     * \brief Evaluate after reading a data item
     *
     * \param transform     the mediator
     *
     * \return the action to be taken
     */
    virtual CPredicatedMediator::Action postInputUpdate(CPredicatedMediator& transform,
                                                       int type) = 0;

    /*!
     * \brief Evaluate prior to writing a data item
     *
     * \param transform     the mediator
     *
     * \return the action to be taken
     */
    virtual CPredicatedMediator::Action preOutputUpdate(CPredicatedMediator& transform,
                                                       int type) = 0;

    /*!
     * \brief Evaluate prior to reading a data item
     *
     * \param transform     the mediator
     *
     * \return the action to be taken
     */
    virtual CPredicatedMediator::Action postOutputUpdate(CPredicatedMediator& transform,
                                                        int type) = 0;

    /*!
     * \brief Resets to an initial state
     */
    virtual void reset() = 0;

};

} // end DAQ

#endif
