//
// Test recursive mutex:
//
// Assumptions:
//   Buffers tagged type 2 are event buffers.
//   Buffers tagged type 3 are control buffers.
//   We want them all.. unsampled for now.
//

#include <iostream.h>
#include <iomanip.h>

#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#include <CThreadRecursiveMutex.h>

#ifndef LOOPCOUNT
#define LOOPCOUNT 1000
#endif

#ifndef THRDCOUNT
#define THRDCOUNT 10
#endif

CThreadRecursiveMutex TestMutex;

class Islocked: public DAQThread
{
  int operator() (int argc, char** argv) {
    for(int i = 0; i < 10000; i++) {
      if(TestMutex.isLocked()) {
	cerr << "IsLocked - locked\n";
      }
      else {
	cerr << "IsLocked - not locked\n";
      }
      cerr.flush();
    }
    return 0;

  }
};

class Unlockalltest : public DAQThread
{
  int operator() (int argc, char** argv) {

    // parameter 0 is really the nesting depth.
    // parameter 1 is the loop count.

    int depth = (int)argv[0];
    int count = (int)argv[1];
    daqthread_t tid = daqthread_self();
    for(int i = 0; i < count ; i++) {
      for(int j = 0; j < depth; j++) {
	TestMutex.Lock();
	cerr << hex << tid << dec
	     << " Locked at depth " << j << endl;
	cerr.flush();
      }
      cerr << "Unlocking all at once" << endl;
      cerr.flush();
      TestMutex.UnLockCompletely();
    }
    return 0;
  }
};

class TryTest : public DAQThread
{
  int operator() (int argc, char** argv) {

    // argc  is really the loop count.

    daqthread_t tid = daqthread_self();

    while(argc) {
      if(TestMutex.TryLock() == 0) {
	cerr << " TryTest (" << hex << tid << dec << ") locked ...";
	cerr << " TryTest (" << hex << tid << dec << ") unlocked\n";
	cerr.flush();
	TestMutex.UnLock();
	argc--;
      }
      else {
	assert(errno = EAGAIN);
      }
    }
  }
};

class NestTest : public DAQThread
{
  int operator() (int argc, char** argv) {

    // parameter 0 is really the nesting depth.
    // parameter 1 is the loop count.

    int depth = (int)argv[0];
    int count = (int)argv[1];
    daqthread_t tid = daqthread_self();
    for(int i = 0; i < count ; i++) {
      for(int j = 0; j < depth; j++) {
	TestMutex.Lock();
	cerr << hex << tid << dec
	     << " Locked at depth " << j << endl;
	cerr.flush();
      }
      for(int j = depth-1; j >=  0; j--) {
	cerr << hex << tid << dec
	     << " Unlocking at depth " << j << endl;
	cerr.flush();
	TestMutex.UnLock();
      }
    }
    return 0;
  }
};

/*===================================================================*/
class DAQMutex : public DAQROCNode {
  int operator()(int argc,char **argv) {
    int args[] = {0, LOOPCOUNT};
    TryTest     tryer;
    DAQThreadId tids[THRDCOUNT];
    NestTest* objPtrs[THRDCOUNT];
    NestTest* pThread;
    DAQStatus    stat;
      
    DAQThreadId Tryid = daq_dispatcher.Dispatch(tryer,LOOPCOUNT);
    for(int i = 1; i <= THRDCOUNT; i++) {
      args[0] = i;
      pThread = new NestTest;
      tids[i-1]    = daq_dispatcher.Dispatch(*pThread, 2,(char**) args);
      objPtrs[i-1] = pThread;
    }
    Islocked    islocktest;
    DAQThreadId islockId = daq_dispatcher.Dispatch(islocktest);

    Unlockalltest alltest;
    DAQThreadId   allid  = daq_dispatcher.Dispatch(alltest,2, (char**)args);

    //  Wait for the threads to exit.
    
    for(int i = 0; i < THRDCOUNT; i++) {
      Join(tids[i], &stat);
      cerr << "Thread id " << hex << tids[i] << dec << " Exited status: "
	   << stat.GetStatusCode() << endl;;
    }

    Join(Tryid, &stat);
    cerr << "Tryer exited with " << stat.GetStatusCode() << endl;

    Join(allid, &stat);
    cerr << "Unlockalltest exited\n";
    Join(islockId, &stat);
    cerr << "Islock test exited\n";

  } 
};

DAQMutex mydaq;




