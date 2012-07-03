/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CEVENTORDERCLIENT_H
#define __CEVENTORDERCLIENT_H

#ifndef  __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

class CEventOrderClient {
private:
  std::string m_host;		// Host running the event builder.
  uint16_t    m_port;		// port on which the event builder is running.
  
  // construction/destruction/canonicals
public:
  CEventOrderClient(std::string host, uint16_t port);
  virtual ~CEventOrderClient();

private:
  CEventOrderClient(const CEventOrderClient&);
  CEventOrderClient& operator=(const CEventOrderClient&);
  int operator==(const CEventOrderClient&) const;
  int operator!=(const CEventOrderClient&) const;
  

  // Static members:

  static uint16_t Lookup(std::string host);

  // Object operations:

  void Connect();

};


#endif
