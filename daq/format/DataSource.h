/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCLDAQ Development Group
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __DATASOURCE_H
#define __DATASOURCE_H


namespace NSCLDAQ 
{

  /*! 
    Abstract base class that defines an interface that all data sources 
    must meet. Data source provide a standardized interface by which complete
    data elements can be read. The class is a template because it implements a 
    generic method for extracting data, but also specializations for the specific
    data formats known to NSCLDAQ. At the moment, the only specialization provided
    is the CRingItem, which works for the 11.0 data format.. 
    */
  template<class T> class DataSource
  {

    public:
      /*! \brief Default constructor is used. 
       *
       * Note that this class cannot actually be constructed.
       */ 
      DataSource() = default;

      /*! \brief Virtual destructor
       *
       * This is a no-op because there is no resources managed
       * by the base class.
       */
      virtual ~DataSource() {};

    private:
      // These are not well defined operations and are thus not allowed.
      DataSource(const DataSource& rhs);
      DataSource& operator=(const DataSource& rhs);
      int operator==(const DataSource& rhs) const;
      int operator!=(const DataSource& rhs) const;
      
    public:

      /*! \brief All derived classes must implement how to extract a data element
       *
       * The pointer returned is to reference allocated memory that must
       * be released by the caller. Ownership of the object is passed to the
       * caller. 
       *
       */
      virtual T* getItem() = 0;
  };

} // end of NSCLDAQ namespace

#endif
