#include <spectrodaq.h>
#include <SpectroFramework.h>
#include <list>
#include <string>


typedef list<CEvent*> EventList;

// Reaper objects are timed events which delete dead process objects:
//
class Reaper : public CTimerEvent
{
private:
  EventList m_DeletePending;
public:
  Reaper(const char* pName);

  void QueueEvent(CEvent* pEvent);
  CEvent* DeQueueEvent();

  virtual void OnTimer();
};
// Implementation of Reaper:

Reaper::Reaper(const char* pname) :
  CTimerEvent(pname, 1000, true) {}

void 
Reaper::QueueEvent(CEvent* pEvent)
{
  CApplicationSerializer::getInstance()->Lock(); // Don't assume this is done
  m_DeletePending.push_back(pEvent);             // in an event context. 
  CApplicationSerializer::getInstance()->UnLock();
}
CEvent* 
Reaper::DeQueueEvent()		// Returns NULL if empty queue or front not yet
{				// inactive... assumed to run locked.
  if(m_DeletePending.empty()) return (CEvent*)NULL;

  CEvent* pItem = m_DeletePending.front();
  if(pItem->isActive()) {
    m_DeletePending.pop_front();
    return pItem;
  }
  else {
    return (CEvent*)NULL;
  }
}
void 
Reaper::OnTimer()
{
  CEvent* pEvent;
  while(pEvent = DeQueueEvent()) {
    delete pEvent;
  }
}

// Server instance. Echoes client requests on client channel until
// client exits.. at exit time, disables self and enters the object
// on the delete pending queue of a reaper.

class EchoServer : public CServerInstance
{
  Reaper& m_GrimReaper;
public:
  EchoServer(CSocket* pSocket, Reaper& pReapme);
  void OnRequest(CSocket* pSocket);
};

EchoServer::EchoServer(CSocket* pSocket, Reaper& rReapme) :
  CServerInstance(pSocket),
  m_GrimReaper(rReapme) {}

void
EchoServer::OnRequest(CSocket* pSocket) {
  char buffer[1024];
  int nread = pSocket->Read(buffer, sizeof(buffer)-1);
  if(nread <= 0) {		// Client exited or other error...
    Shutdown();			// Shutdown our part of the connection.
    Disable();			// Schedule thread exit and
    m_GrimReaper.QueueEvent(this); // Object deletion.
  } else {			// Data available.
    pSocket->Write(buffer, nread);
  }
}


// Server listener.  Only new functionality is the OnConnection
// which creates a new server instance thread.

class EchoListener : public CServerConnectionEvent
{
  Reaper& m_GrimReaper;
public:
  EchoListener(const char* pName, const string& rservice, Reaper& rReaper);
  virtual void OnConnection(CSocket* pSocket);
};

EchoListener::EchoListener(const char* pName, const string& rservice,
			   Reaper& rReaper) :
  CServerConnectionEvent(pName, rservice),
  m_GrimReaper(rReaper)
{}
void
EchoListener::OnConnection(CSocket* pSocket)
{
  EchoServer* pServer = new EchoServer(pSocket, m_GrimReaper);
  pServer->Enable();
}


class MyApp : public DAQROCNode
{
protected:
  int operator()(int argc, char** pargv);

};
int
MyApp::operator()(int argc, char** pargv)
{
  Reaper theReaper("GrimReaper");
  theReaper.Enable();		// Start off the grim reaper.

  EchoListener Listen("EchoListen", string("2048"), theReaper);
  Listen.Enable();

  DAQThreadId id = Listen.getThreadId();
  Join(id);
  
};

MyApp theApplication;
