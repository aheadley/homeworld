#ifndef _H_PipeCmd
#define _H_PipeCmd

#include "EasySocket.h"

namespace WONMisc {

class SocketPipe;
class EasySocketEngine;

////////////////////

enum PipeCmdType {
	pctNone,
	pctOpen,
	pctBind,
	pctClose,
	pctCloseNow,
	pctShutdown,
	pctListen,
	pctAccept,
	pctConnect,
	pctSmartConnect,
	pctSend,
	pctSendTo,
	pctBroadcast,
	pctRecv,
	pctRecvPrefixed,
	pctRecvFrom,
	pctCallBack,
	pctSetEvent,
	pctWaitForEvent,
	pctTimer,
	pctNoOpPayload
};


////////////////////

class PipeCmd {
private:
	friend class SocketPipe;
	bool mProcessing;

protected:
	PipeCmdType mType; // internal use

public:

	bool mDeleteOnCompletion; // input
	bool mCompleted; // output
	ES_ErrorType mResult; // output

	PipeCmd(PipeCmdType theType,
			bool deleteOnCompletion);

	virtual ~PipeCmd(void);

	PipeCmdType GetType(void) const;
};

////////////////////

class OpenCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	EasySocket::SocketType mSocketType; // input

	OpenCmd(EasySocket::SocketType theSocketType,
			bool deleteOnCompletion = true);
};

////////////////////

class BindCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	unsigned short mPort; // input/output.  If you set mPort to 0 as input, check the output for the actual assigned port
	bool mAllowReuse; // input

	BindCmd(unsigned short thePort,
			bool allowReuse = false,
			bool deleteOnCompletion = true);
};

////////////////////

class CloseCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:

	bool mSetLinger; // input.  EasySockets don't linger by default, but set this to true to use the mLinger and mLingerSeconds args
	bool mLinger; // input
	unsigned short mLingerSeconds; // input

	CloseCmd(	bool setLinger = false,
				bool linger = false,
				unsigned short theLingerSeconds = 0,
				bool deleteOnCompletion = true );
};

////////////////////

class CloseNowCmd : public CloseCmd {
public:

	CloseNowCmd(bool setLinger = false,
				bool linger = false,
				unsigned short theLingerSeconds = 0,
				bool deleteOnCompletion = true );
};

////////////////////

class ShutdownCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	long mHow; // input, 0 for receives, 1 for sends, and 2 for both

	ShutdownCmd(long theHow = 2,
				bool deleteOnCompletion = true);
};

////////////////////

class ListenCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	long mBacklog; // input

	ListenCmd(	long theBacklog = 5,
				bool deleteOnCompletion = true);
};

////////////////////

class AcceptCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	EasySocketEngine* mSocketEngineP; // input
	bool mAddToEngine; // input
	SocketPipe* mAcceptedPipeP; // output
	SOCKADDR mSockAddr; // output

	AcceptCmd(	EasySocketEngine* theSocketEngineP,
				bool addToEngine,
				bool deleteOnCompletion ); // not setting any defaults here.  Make people make a concious decision about how to use this command
};

////////////////////

class ConnectCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	SOCKADDR mSockAddr; // input

	ConnectCmd(	const SOCKADDR& theDestSockAddr,
				bool deleteOnCompletion ); // not setting any defaults here.  Make people make a concious decision about how to use this command
};	

////////////////////

// See the "patented BrianR" connect method in EasySocket.h to understand what this command does
class SmartConnectCmd : public PipeCmd {
private:
	friend class SocketPipe;
	long mAttemptIndex; // internal use
	long mStartTime; // internal use

public:
	SOCKADDR mSockAddr; // input
	long mWaitTime; // input
	long mNumTries; // input

	SmartConnectCmd(const SOCKADDR& theDestSockAddr,
					long theWaitTime,
					long theNumTries,
					bool deleteOnCompletion ); // not setting any defaults here.  Make people make a concious decision about how to use this command
};	

////////////////////

class BufferCmd : public PipeCmd {
public:
	char* mBufferP; // input
	long mBufferLength; // input
	bool mOwnsBuffer; // input.  will delete buffer if deleted

	BufferCmd(	char* theBufferP,
				long theBufferLength,
				bool ownsBuffer,
				bool deleteOnCompletion); // not setting any defaults here.  Make people make a concious decision about how to use this command
	virtual ~BufferCmd(void);  // to facilitate deletion of buffers
};

////////////////////

class SendCmd : public BufferCmd {
private:
	friend class SocketPipe;
	long mSentSoFar; // internal use

public:
	SendCmd(char* theBufferP,
			long theBufferLength,
			bool ownsBuffer,
			bool deleteOnCompletion); // not setting any defaults here.  Make people make a concious decision about how to use this command
};

////////////////////

class SendToCmd : public SendCmd {
private:
	friend class SocketPipe;

public:
	SOCKADDR mSockAddr; // input

	SendToCmd(	const SOCKADDR& theDestSockAddr,
				char* theBufferP,
				long theBufferLength,
				bool ownsBuffer,
				bool deleteOnCompletion); // not setting any defaults here.  Make people make a concious decision about how to use this command
};
	
////////////////////

class BroadcastCmd : public SendCmd {
private:
	friend class SocketPipe;
	unsigned short mPort;

public:

	BroadcastCmd(	unsigned short theDestPort,
					char* theBufferP,
					long theBufferLength,
					bool ownsBuffer,
					bool deleteOnCompletion); // not setting any defaults here.  Make people make a concious decision about how to use this command
};
	
////////////////////

class RecvCmd : public BufferCmd {
private:
	friend class SocketPipe;

public:
	long mBytesReceived; // output
	long mBytesToReceive; // input

	RecvCmd(char* theBufferP,
			long theBytesToReceive,
			bool ownsBuffer,
			bool deleteOnCompletion );
};

////////////////////

enum RecvLengthPrefixType {
	ptByte,
	ptUnsignedByte,
	ptShort,
	ptUnsignedShort,
	ptLong,
	ptUnsignedLong
};

////////////////////

// useful for receiving length prefixed messages
// I wonder if there ought to be a max receivable
// length arg

class RecvPrefCmd : public BufferCmd {
private:
	friend class SocketPipe;
	char mPrefixLength; // internal use
	long mBytesToReceive;
	bool mReceivedLength;

public:
	RecvLengthPrefixType mPrefixType; // input
	long mBytesReceived; // output

	RecvPrefCmd(bool ownsBuffer,
				RecvLengthPrefixType thePrefixType,
				bool deleteOnCompletion );
};

////////////////////

class RecvFromCmd : public BufferCmd {
private:
	friend class SocketPipe;

public:
	SOCKADDR mSockAddr; // output
	long mBytesReceived; // output

	RecvFromCmd(char* theBufferP,
				long theBufferLength,
				bool ownsBuffer,
				bool deleteOnCompletion );
};

////////////////////

/*
// this would probably be a useful cmd, but has not been implemented yet.
// there might be a need for variations on this wherein the length ptr
// is a char* or a short* or a uchar* or a ushort*

class RecvLengthPtrCmd : public PipeCmd {
	char* mBuffer; // input/output
	long* mBytesToReceiveP; // input
	long mBytesReceived; // output
};
*/

////////////////////

typedef void (*CallBackCmdPtr)(void* theParamP);

class CallBackCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	CallBackCmdPtr mCallBackCmdPtr; // input
	void* mCallBackParamP; // input

	CallBackCmd(CallBackCmdPtr theCallBackCmdPtr,
				void* theCallBackParamP,
				bool deleteOnCompletion = true);
};


////////////////////

class SetEventCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	HANDLE mEvent; // input

	SetEventCmd(HANDLE theEvent,
				bool deleteOnCompletion = true);
};

////////////////////

class WaitForEventCmd : public PipeCmd {
private:
	friend class SocketPipe;

public:
	HANDLE mEvent; // input
	bool mResetEvent; // input

	WaitForEventCmd(HANDLE theEvent,
					bool resetEvent,
					bool deleteOnCompletion = true);
};

////////////////////

class TimerCmd : public PipeCmd {
private:
	friend class SocketPipe;
	unsigned long mStartTime; // internal use

public:
	unsigned long mMillisecondsToWait; // input

	TimerCmd(	unsigned long theMillisecondsToWait,
				bool deleteOnCompletion = true);
};

////////////////////

class NoOpPayloadCmd : public PipeCmd {
private:
	friend class SocketPipe;
	unsigned long mPayload;
public:
	NoOpPayloadCmd(unsigned long thePayloadP, bool deleteOnCompletion =true);
	unsigned long GetPayload() const { return mPayload; }
};

// Do I need a generic wait for object?
// What about mutexes, semaphores, etc?
// I think events are good enough to start with.

}; // end namespace WONMisc

#endif // _H_PipeCmd
