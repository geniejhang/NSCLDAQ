/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

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

// forward definitions:



/*! 
   Abstract base class that defines an interface that all dumper data sources 
   must meet.  Data sources provide data that can be dumped in a formatted
   way to the dumper's stdout.
   */

namespace NSCLDAQ 
{

  template<class T> class DataSource
  {
    // constructors and canonicals.  In general most canonicals are not allowed.
    //

    public:
      DataSource() = default;
      virtual ~DataSource() {};

    private:
      DataSource(const DataSource& rhs);
      DataSource& operator=(const DataSource& rhs);
      int operator==(const DataSource& rhs) const;
      int operator!=(const DataSource& rhs) const;
    public:

      // This method must be implemented in concrete
      // sublcasses.  It returns the next item from the'
      // source the dumper deserves to get.
      //
      virtual T* getItem() = 0;
  };

} // end of NSCLDAQ namespace

#endif
