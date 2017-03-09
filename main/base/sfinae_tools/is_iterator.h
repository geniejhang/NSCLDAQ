#ifndef DAQ_IS_ITERATOR_H
#define DAQ_IS_ITERATOR_H

#include <utility>
#include <iterator>

namespace DAQ {

/*!  TMP function is_iterator<typename T>
 *
 * To give credit where credit is due, this code was taken from an answer to
 * a stack exchange question. It was offered by user ForEveR at the site:
 * http://stackoverflow.com/questions/12032771/how-to-check-if-an-arbitrary-type-is-an-iterator
 *
 * The is_iterator struct is just some template metaprogramming magic based on SFINAE.
 *
 * If the template parameter that has has a valid value_type when passed to std::iterator_traits<T>,
 * its value value will be true. Otherwise it will return false.
 *
 * \code
 *
 *  template<class T>
 *  void identifyIterator(T obj) {
 *      if (DAQ::is_iterator< T >::value ) {
 *          std::cout << "Type is an iterator" << std::endl;
 *      } else {
 *          std::cout << "Type is NOT an iterator" << std::endl;
 *      }
 * }
 *
 * std::vector<int>::iterator it;
 * double d;
 * int i;
 * int* pI;
 *
 * identifyIterator(it);  // prints "Type is an iterator"
 * identifyIterator(d);   // prints "Type is NOT an iterator"
 * identifyIterator(i);   // prints "Type is NOT an iterator"
 * identifyIterator(pI);  // prints "Type is NOT an iterator"
 * \endcode
 */

template<typename T, typename = void>
struct is_iterator
{
   static constexpr bool value = false;
};

template<typename T>
struct is_iterator<T,
                   typename std::enable_if<
                                           ! std::is_same<
                                                        typename std::iterator_traits<T>::value_type,
                                                        void
                                                        >::value
                                          >::type>
{
   static constexpr bool value = true;
};


} // end DAQ

#endif // DAQ_IS_ITERATOR_H
