/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

// Class: CScalerClient                     //ANSI C++
//

//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved .h
//

#ifndef __SCALERCONSUMER_H  //Required for current class
#define __SCALERCONSUMER_H

                               //Required for base classes
#ifndef __CONSUMER_H
#include <Consumer.h>
#endif
                               
                               //Required for 1:1 association classes
#ifndef __TCLSERVERCONNECTION_H
#include <TclServerConnection.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#define __STL_VECTOR
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif



                                                               
class CScalerClient  : public CConsumer        
{                       
			
  int m_eConnectionState; //        
  TclServerConnection* m_Connection; //1:1 association object data member      
  STD(vector)<long> m_vTotals;	// Run totals for scalers.
  STD(vector)<long> m_vIncrements;	// Run increments for scalers.
  bool         m_fDefaultSource; // True if the data source is default.
							       

public:

   // Constructors and other cannonical operations:

  CScalerClient ()    : 
    m_eConnectionState(0),
    m_Connection(0),
    m_fDefaultSource(true)
  { 
  } 
  ~ CScalerClient ( )  // Destructor 
  {
    if(m_eConnectionState) {
      delete m_Connection;
    }  
  }
   //Copy constructor 
private:
  CScalerClient (const CScalerClient& aCScalerClient );
  CScalerClient& operator= (const CScalerClient& aCScalerClient);
  int operator== (const CScalerClient& aCScalerClient) const;
public:
	

public:
  
  virtual   void 
  OnConnection (TcpClientConnection& rConnection)    ;
  virtual   void 
  OnDisconnected (TcpClientConnection& rConnection)    ;
  
  static    void 
  ConnectionRelay (TcpClientConnection& rConnection, void* pObject)    ;
  static  void 
  DisconnectRelay (TcpClientConnection& rConnection, void* pObject)    ;

  virtual   int operator() (int argc, char** pargv)    ;
 
protected:

  virtual   void OnScalerBuffer (CNSCLScalerBuffer& rScalerBuffer)   ;
  virtual   void OnBeginBuffer (CNSCLStateChangeBuffer& rBuffer)   ;
  virtual   void OnEndBuffer (CNSCLStateChangeBuffer& rBuffer)   ;
  virtual   void OnPauseBuffer (CNSCLStateChangeBuffer& rBuffer)   ;
  virtual   void OnResumeBuffer (CNSCLStateChangeBuffer& rBuffer)   ;

  void  ClearScalers();
  void  UpdateRunState(DAQRunState eNewState);
  void  UpdateRunTitle(const char* pNewTitle);
  void  UpdateRunNumber(int        nNewRun);
  void  UpdateScalers(STD(vector)<ULong_t>& rScalers, Bool_t isSnapshot);
  void  CreateArrays(int nScalers);
private:
  static STD(string) GetRemoteHost(int nArgs, char** pArgs);
  static int    GetRemotePort(int nArgs, char** pArgs);
  STD(string) GetDataSourceURL(int nargs, char** pargs);
  static Bool_t GetSwitchParameter(STD(string)& rValue,
				   const char* pSwitch, int nArgs, char** pArgs);
  static void Usage();
  static void WarnDefaultSource();
};

#endif
