/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


#ifndef CTIMEOUT_H
#define CTIMEOUT_H

#include <chrono>

namespace DAQ {

/*!
 * \brief The CTimeout class
 *
 * This is simple class to encapsulate the logic required for
 * evaluating if a timeout has expired. One can create it and
 * then evaluate at any point whether it has expired.
 *
 */
class CTimeout
{
private:
    double                                         m_nSeconds;
    std::chrono::high_resolution_clock::time_point m_start;

public:
    /*!
     * \brief Constructor
     *
     * \param nSeconds - length of timeout in units of seconds
     *
     */
    CTimeout(double nSeconds);
    CTimeout(const CTimeout& rhs) = default;
    CTimeout(CTimeout&& rhs) = default;
    CTimeout& operator=(const CTimeout&) = default;

    /*!
     * \brief getRemainingSeconds
     *
     * This retrieves the amount of time that remains before expiration.
     *
     * \retval 0 - timeout has elapsed
     * \retval >0 - otherwise
     */
    double getRemainingSeconds() const;

    /*!
     * \brief expired
     *
     * Checks whether the timeout has expired
     *
     * \return bool
     * \retval false - not expired
     * \retval ture  - expired
     */
    bool expired() const;

    /*!
     * \brief reset
     *
     * Resets the start time to the present.
     */
    void reset();
};

} // end DAQ

#endif // CTIMEOUT_H
