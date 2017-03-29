/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTclRingCommand.h
# @brief  Define the 'ring' command which gives limited access to ring data.
# @author <fox@nscl.msu.edu>
*/


#ifndef CTCLRINGCOMMAND_H
#define CTCLRINGCOMMAND_H

#include <V12/CRingItem.h>

#include <TCLObjectProcessor.h>
#include <map>

#include <TCLObject.h>
#include <CDataSource.h>

class CTCLInterpreter;

namespace DAQ {

class CDataSourcePredicate;
class CTimeout;

namespace V12 {

/**
 * @class CTclRingCommand
 *
 * This class implements the ring command.  The ring command allows
 * Tcl scripts to establish a connection to an NSCL Ring buffer and to
 * get limited data types from that ringbuffer.  As this is part of the
 * scaler display program, the types of data that are accepted from the ring
 * are only:
 *
 * *  The state change types.
 * *  PERIODIC_SCALERS
 *
 * This can only be used for data in version 11.0 format.
 *
 *  Command format:
 *  \verbatim
 *     ring attach ringname
 *     ring detach ringname
 *     ring get ringname ?acceptable-types?
 * \endverbatim
 */
class CTclRingCommand : public CTCLObjectProcessor
{
private:
    std::map<std::string, CDataSourcePtr> m_attachedRings;
    
public:
    CTclRingCommand(CTCLInterpreter& interp);
    virtual ~CTclRingCommand();
    
    virtual int operator()(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    
protected:
    void attach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void detach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Local utilities.
private:
    void formatHeaderInfo(CRingItemPtr p, CTCLObject &result);
    CTCLObject formatStateChangeItem(CTCLInterpreter& interp, CRingItemPtr pItem);
    CTCLObject formatScalerItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatStringItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatFormatItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatEvent(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatTriggerCount(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatGlomParams(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatAbnormalEnd(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);
    CTCLObject formatComposite(CTCLInterpreter& interp, CRingItemPtr pSpecificItem);

    CTCLObject dispatch(CRingItemPtr pSpecificItem, CTCLInterpreter& interp);

    CRingItemPtr getFromRing(CDataSource& ring, CDataSourcePredicate& predicate,
                           const CTimeout& timeout);
    CRingItemPtr getFromRing(CDataSource& ring, const CTimeout& timeout);

};

} // end V12
} // end DAQ

#endif
