#ifndef _H_EasySocketEngine
#define _H_EasySocketEngine

#include "EasySocket.h"
#include "SocketPipe.h"

namespace WONMisc {

typedef list<SocketPipe*> PipeList;

class EasySocketEngine {
private:
	
	PipeList mPipeList;
	CRITICAL_SECTION mCrit;
	static unsigned int __stdcall mThreadRoutine( void* param );
	HANDLE mStopEvent;
	HANDLE mCompletedPipeEvent;
	HANDLE mAbortGetCompletedPipeEvent;
	HANDLE mThread;
	int mThreadID;
	unsigned int mPumpLoopCount; // times that Pump will go around before exitting if there are commands left to be processed
	unsigned int mPostProcessingSleepPeriod; // time to sleep after pumping in mThreadRoutine 

public:

	EasySocketEngine(unsigned int thePumpLoopCount =1, unsigned int thePostProcessingSleepPeriod =20, bool pumpAutomatically =true);
	virtual ~EasySocketEngine(void);

	static ES_ErrorType startWinsock(void);
	static ES_ErrorType stopWinsock(void);
	long GetNumPipes(void);
	SocketPipe* GetNthPipe(long theIndex); // zero based index
	SocketPipe* GetErrorPipe(void);
	SocketPipe* GetCompletedPipe(DWORD theTimeout =INFINITE);
	void AbortGetCompletedPipe(void);
	void AddPipe(SocketPipe* thePipeP);
	void RemovePipe(SocketPipe* thePipeP);
	void PurgePipe(SocketPipe* theSocketPipeP);
	void KillPipe(SocketPipe* theSocketPipeP);
	
	static unsigned int Pump(EasySocketEngine* theEngineP);
};

}; // end namespace WONMisc

#endif // _H_EasySocketEngine
