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

#ifndef MAKEUNIQUE_H
#define MAKEUNIQUE_H

#include <memory>
#include <utility>

namespace DAQ {

/**! \brief Utility to turn an pointer into a unique ptr
 *
 *  This serves to stem the gap between the time when the method
 *  is not a part of the stl implementations yet. It is to be included
 *  in C++14, but is not yet in C++11. This should eventually be
 *  replaced by the C++14 version.
 *
 *  The returned unique_ptr will own the object pointed to by 
 *  the argument. It is the callers responsibility to make sure that 
 *  no other objects own it and try to delete it.
 *
 * The implementation here is taken directly from the sample implementation
 * provided here http://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique
 * on 3/8/2017
 *
 * \param pObject   pointer to be transformed into a unique_ptr
 *
 * \return std::unique_ptr<T> that owns object passed in
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // end DAQ
#endif
