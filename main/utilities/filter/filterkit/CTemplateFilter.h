#ifndef V12_CTEMPLATEFILTER_H
#define V12_CTEMPLATEFILTER_H

#include <V12/CFilter.h>
#include <V12/CPhysicsEventItem.h>


/**! \class CTemplateFilter
    Here is a sample implementation of a filter to append a reversed copy of the
    data in a physics event to its body. This is for illustration purposes only.

    See the documentation for the V12::CFilter base class for the virtually declared
    methods available for dealing with non-physics events. The user has access
    to all of the different ring item types.
*/
class CTemplateFilter : public DAQ::V12::CFilter
{
  public:
    /**! Virtual copy constructor
        Implementation of this is mandatory!
    */
    virtual DAQ::V12::CFilterUPtr clone() const;

    /**! A sample filter for handling physics events

        The handlePhysicsEventItem method will be called for every physics event item. It will produce
        a ring item whose body is double the size of the original item,
        with the first half being
        the original data and the second half being the data in reversed order.
        This filter is unlikely to have any real use but is defined to be
        illustrative of how to manipulate the data of a ring item.

        @param pItem a pointer to the raw physics event item to process
        @return the resulting ring item from this filter.
    */
    virtual DAQ::V12::CPhysicsEventItemPtr
    handlePhysicsEventItem(DAQ::V12::CPhysicsEventItemPtr pItem);
};

#endif // CTEMPLATEFILTER_H
