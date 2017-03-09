#ifndef DAQ_IS_BYTE_ITERATOR_H
#define DAQ_IS_BYTE_ITERATOR_H

#include <utility>
#include <iterator>

#include <is_iterator.h>

namespace DAQ {

template<class T, typename = void>
struct is_byte_iterator {
    static const bool value = false;
};

template<typename T>
struct is_byte_iterator<T,
                        typename std::enable_if<
                                                is_iterator<T>::value
                                                &&
                                                ( sizeof(typename std::iterator_traits<T>::value_type) == 1 ) >::type
                        >
{
    static const bool value = true;
};

}
#endif // DAQ_IS_BYTE_ITERATOR_H
