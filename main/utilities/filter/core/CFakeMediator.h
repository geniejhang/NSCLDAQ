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

#ifndef DAQ_CFAKEMEDIATOR_H
#define DAQ_CFAKEMEDIATOR_H

#include <CFilterMediator.h>

#include <vector>
#include <string>

namespace DAQ {

class CFakeMediator;
using CFakeMediatorPtr = std::shared_ptr<CFakeMediator>;

class CFakeMediator : public CFilterMediator {
  private:
    std::vector<std::string> m_log;
  
  public:
    CFakeMediator(): CFilterMediator(), m_log() {}
    void mainLoop() {
      m_log.push_back("mainLoop");
    }

    void initialize() {
      m_log.push_back("initialize");
    }

    void finalize() {
      m_log.push_back("finalize");
    }

    std::vector<std::string> getLog () const { return m_log;}
};

} // end DAQ
#endif
