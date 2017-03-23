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


#include <CTemplateFilter.h>
#include <make_unique.h>     // required for DAQ::make_unique() function
#include <ByteBuffer.h>

#include <cstdint>

using namespace DAQ::V12;

/*! Virtual copy constructor
 *
 * \returns newly allocated copy of this object wrapped in a std::unique_ptr.
*/
CFilterUPtr CTemplateFilter::clone() const {

    // There are multiple ways to implement this method and this
    // is just one simple example. You should just
    // replace the "CTemplateFilter" with the name of your class.
    //
    // Three other ways to implement this would be :
    //
    //  return CFilterUPtr(new CTemplateFilter(*this));
    //
    //  return std::unique_ptr<CTemplateFilter>(new CTemplateFilter(*this))
    //
    //  return CTemplateFilterUPtr(new CTemplateFilter(*this));
    //
    //  these are all equivalent in functionality. The key is to create a
    // std::unique_ptr. The std::unique_ptr is just a "smart pointer"
    // that defines single owner semantics. By returning a CFilterUPtr,
    // (i.e. std::unique_ptr<CTemplateFilter>), we are clearly stating
    // that the caller is passed ownership of the newly allocated object.
    return DAQ::make_unique<CTemplateFilter>(*this);
}


/*! A sample filter for handling physics events

        This filter will be called for every physics event item. It will produce
        a ring item double the size of the original item with the first half being
        the original data and the second half being the data in reversed order.
        This is unlikely to have any real use but is defined to be
        illustrative of how to manipulate the data of a ring item.

        To be more illustrative, I will show you three equivalent ways.
        The first demonstrates the recommended method using the insertion operators
        (i.e. "<<" ). The second is probably more common to see
        in the wild. It makes use of std::vector<uint8_t>::push_back() to fill the body.
        The third is the most elegant and simple (only a few lines of real work). Though elegant and simple,
        it makes use of the slightly more advanced concept of an iterator (some object
        that iterates through a sequence).

        @param pItem    a shared pointer to the raw physics event item to process

        @return shared_ptr of a CPhysicsEventItem (i.e. CPhysicsEventItemPtr). If you
                do not wish to output a ring item, return nullptr or CPhysicsEventPtr().
    */
DAQ::V12::CPhysicsEventItemPtr
CTemplateFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    using DAQ::Buffer::ByteBuffer; // a.k.a. std::vector<uint8_t>

    // Create a copy of the original item to manipulate as a shared_ptr
    // (a.k.a. CPhysicsEventItemPtr). This is unnecessary
    // but allows one to safely abort filtering and return the original ring
    // item. The CPhysicsEventItemPtr is a std::shared_ptr, another smart pointer
    // type. You do not need to worry too much about how it works, but you should
    // know that it behaves just like a normal pointer and you do not need to
    // delete it at the end. It will delete the memory you see new'ed into existence
    // when it goes out of scope.
    CPhysicsEventItemPtr pFiltItem(new CPhysicsEventItem(*pItem));

    // At this point, both pFiltItem and pItem have identical data in their bodies.

    // Get a reference to the body of the item that was passed in and also the reference
    // to the body of the newly created item.
    ByteBuffer& oldBody = pItem->getBody();
    ByteBuffer& newBody = pFiltItem->getBody();


    // Simple optimization for subsequent examples:
    //----------------------------------------------
    // Reserve the amount of space that will be ultimately be used. This step
    // is not required, because the ByteBuffer will automatically reallocate as
    // necessary. However, it is always faster to allocate once rather than
    // multiple times. Because we know how much we need, let's reserve the space.
    newBody.reserve(oldBody.size()*2);


    // Example #1 - Using the << operators
    //------------------------------------------------------------------------

    // clear the body for the sake of consistency with examples 2 and 3.
    newBody.clear();

    // insert body data in normal order
    size_t bodySize = oldBody.size();
    for (size_t i=0; i<bodySize; ++i) {
        newBody << oldBody[i];
    }

    // insert body data in reverse order
    for (size_t i=0; i<bodySize; ++i) {
        newBody << oldBody[ (bodySize-1) - i ];
    }

    // As a note about these insertion operators, they work for uint8_t, uint16_t,
    // uint32_t, uint64_t, int8_t, int16_t, int32_t, and int64_t. You are strongly
    // encouraged to use these because they properly serialize the data for you
    // into a stream of bytes, which is what the body of the physics event is.




    // Example #3 - Using push_back method
    //------------------------------------------------------------------------
    // remember that a ByteBuffer is just a std::vector<uint8_t>. You can do anything
    // to it that you can do with a vector.

    // first let's clear our new body so we start from scratch.
    newBody.clear();

    // insert body data in normal order
    size_t bodySize = oldBody.size();
    for (size_t i=0; i<bodySize; ++i) {
        newBody.push_back(oldBody[i]);
    }

    // append body data in reverse order
    for (size_t i=0; i<bodySize; ++i) {
        newBody.push_back(oldBody[ (bodySize-1) - i ]);
    }




    // Example #3 - Using insert and iterators
    //------------------------------------------------------------------------

    // clear body
    newBody.clear();

    // Here we are appending the entire body in normal order and then in reverse
    // order. This is accomplished by inserting new data at the end of the new body.
    // The first time it happens, we use normal iterators to fill the body in normal
    // order. The second time around reverse" iterators are used. These reverse
    // iterators will iterate from the end to the beginning instead of from the
    // beginning to the end like a normal iterator.
    newBody.insert(newBody.end(), oldBody.begin(), oldBody.end());
    newBody.insert(newBody.end(), oldBody.rbegin(), oldBody.rend());



    // if you want to discard an event,
    // return CPhysicsEventItemPtr()

    // otherwise, return the item
    return pFiltItem;
}

