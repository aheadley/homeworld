#ifndef _H_SocketPipe
#define _H_SocketPipe

#include "PipeCmd.h"
#include "LIST"
//#include "windows.h"

namespace WONMisc {

typedef list<PipeCmd*> PipeCmdQueue;

class SocketPipe {
private:
    PipeCmdQueue mOutCmds;
    PipeCmdQueue mInCmds;
    CRITICAL_SECTION mOutCrit;
    CRITICAL_SECTION mInCrit;
    PipeCmdQueue::iterator mCurrOutCmd;
    PipeCmdQueue::iterator mCurrInCmd;
    int mLabel;

    bool HasCompletedCmds(  const PipeCmdQueue& theQueueR,
                            CRITICAL_SECTION& theCritR );

    long GetNumCompletedCmds(   const PipeCmdQueue& theQueueR,
                                CRITICAL_SECTION& theCritR );

    bool HasPendingCmds(const PipeCmdQueue& theQueueR,
                        CRITICAL_SECTION& theCritR );

    long GetNumPendingCmds( const PipeCmdQueue& theQueueR,
                            CRITICAL_SECTION& theCritR );

    PipeCmd* RemoveCmd( PipeCmdQueue& theQueueR,
                        CRITICAL_SECTION& theCritR );

    PipeCmd* RemoveCompletedCmd(PipeCmdQueue& theQueueR,
                                CRITICAL_SECTION& theCritR );

    void AddCmd(PipeCmd* thePipeCmdP,
                PipeCmdQueue& theQueueR,
                CRITICAL_SECTION& theCritR );

    bool ProcessCmd(PipeCmd* thePipeCmdP); // returns true if completed the command
    void MarkInError();

    SocketPipe(EasySocket* theEasySocketP);
public:
    EasySocket* mEasySocketP; // access with caution
    ES_ErrorType mError;
    bool mInErrorState;
    HANDLE mCompletedPipeEvent;
    
    SocketPipe(void);
    virtual ~SocketPipe(void);

    void SetCompletedPipeEventH(HANDLE theCompletedPipeEventH);

    bool HasCompletedIncomingCmds(void);
    bool HasCompletedOutgoingCmds(void);
    bool HasCompletedCmds(void);
    long GetNumCompletedIncomingCmds(void);
    long GetNumCompletedOutgoingCmds(void);

    bool HasPendingIncomingCmds(void);
    bool HasPendingOutgoingCmds(void);
    long GetNumPendingIncomingCmds(void);
    long GetNumPendingOutgoingCmds(void);

    PipeCmd* RemoveCompletedIncomingCmd(void);
    PipeCmd* RemoveCompletedOutgoingCmd(void);
    PipeCmd* RemoveCompletedCmd(void);

    PipeCmd* RemoveIncomingCmd(void);
    PipeCmd* RemoveOutgoingCmd(void);

    void AddIncomingCmd(PipeCmd* thePipeCmdP);
    void AddOutgoingCmd(PipeCmd* thePipeCmdP);

    void Pump(void);

    void ClearErrorState(   bool forceCompleteCurrentInCommand,
                            bool forceCompleteCurrentOutCommand);

    int GetLabel(void);
    void SetLabel(int theLabel);
};

}; // end namespace WONMisc

#endif // _H_SocketPipe