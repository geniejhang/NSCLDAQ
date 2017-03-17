#ifndef CTEMPLATEFILTER_H
#define CTEMPLATEFILTER_H

#include <V12/CFilter.h>
#include <V12/CPhysicsEventItem.h>


/**! \class CTemplateFilter
    Here is a sample implementation of a filter to append a reversed copy of the
    data in physics event to its body. This is for illustration purposes.

    See the documentation for the CFilter base class for the virtually declared
    methods available for dealing with non-physics events. The user has access
    to all of the different ring item types. In fact, it is not necessary for
    the user to return the same type of ring item from method as it received.
*/
class CTemplateFilter : public DAQ::V12::CFilter
{
  public:
    /**! Virtual copy constructor
        DO NOT FORGET THIS!
    */
    virtual DAQ::V12::CFilterUPtr clone() const;

    /**! A sample filter for handling physics events

        This filter will be called for every physics event item. It will produce
        a ring item double the size of the original item with the first half being
        the original data and the second half being the data in reversed order.
        This filter is unlikely to have any real use but is defined to be
        illustrative of how to manipulate the data of a ring item.

        For some precautionary measures, it also demonstrates how to compute whether
        the new data is going to overflow the available space in the filtered item.
        Reaching the storage capacity is an unlikely occurence because by default a
        ring item has 8192 bytes of storage space.

        @param pItem a pointer to the raw physics event item to process
        @return the resulting ring item from this filter. This can be the same item
                pointed to by pItem or a newly allocated one. Can be any derived
                type of ring item.
    */
    virtual DAQ::V12::CPhysicsEventItemPtr
    handlePhysicsEventItem(DAQ::V12::CPhysicsEventItemPtr pItem);
};

#endif // CTEMPLATEFILTER_H
