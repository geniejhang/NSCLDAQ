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
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;

public:
    /*!
     * \brief Constructor
     *
     * \param nMicroseconds - length of timeout in units of us
     *
     */
    CTimeout(long nMicroseconds);

    /*!
     * \brief Construct from a duration
     * \param duration  any duration value
     *
     * \code
     *  using namespace std::chrono;
     *
     *  CTimeout t0(milliseconds(124));
     *  CTimeout t1(hours(1));
     *  CTimeout t2(nanoseconds(12343234));
     *  CTimeout t3(seconds(3));
     *  CTimeout t4(microseconds(12312312));
     *
     * \endcode
     */
    CTimeout(const std::chrono::high_resolution_clock::duration& duration);
    CTimeout(const CTimeout& rhs) = default;
    CTimeout(CTimeout&& rhs) = default;
    CTimeout& operator=(const CTimeout&) = default;

    /*!
     * \brief getTotalTime
     *
     * \return the total length of the timeout (in std::chrono::nanoseconds)
     *
     * If the length of the timeout is zero, a function should assume
     * polling functionality.
     *
     * Also, you can convert to any timebase you want by using std::chrono::duration_cast.
     * \code
     *
     *  using namespace std::chrono;
     *
     *  nanoseconds nanos = timeout.getTotalTime();
     *  auto millis = duration_cast<milliseconds>(nanos);
     * \endcode
     */
    std::chrono::nanoseconds getTotalTime() const;


    /*!
     * \brief isPoll
     * \retval true if period is 0 seconds
     * \retval false otherwise
     */
    bool isPoll() const {
        return (m_start == m_end);
    }

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
     * \brief getRemainingTime in nanoseconds
     *
     * \return returns remaining nanoseconds if not expired
     * \retval 0 if expired
     *
     * The return value can be manipulated like with getTotalTime()
     */
    std::chrono::nanoseconds getRemainingTime() const;

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
