/******************************************************************************/
/*                                                                            */
/*     !  N O T I C E  !  N O T I C E  !  N O T I C E  !  N O T I C E  !      */
/*                                                                            */
/*             ©1998 Sierra On-Line, Inc.  All Rights Reserved.               */
/*                     U.S. and foreign patents pending.                      */
/*                                                                            */
/*                          THIS SOFTWARE BELONGS TO                          */
/*                            Sierra On-Line, Inc.                            */
/*     IT IS CONSIDERED A TRADE SECRET AND IS NOT TO BE DIVULGED OR USED      */
/*        BY PARTIES WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM         */
/*                            Sierra On-Line, Inc.                            */
/*                       3380 146th Place SE, Suite 300                       */
/*                          Bellevue, WA  98007-6472                          */
/*                                206 649-9800                                */
/*                                                                            */
/*              Federal law provides severe civil penalties for               */
/*                   misuse or violation of trade secrets.                    */
/*                                                                            */
/******************************************************************************/
// Threadbase.h - nice encapsulation for threads
//
//////////////////////////////////////////////////////////////////////
// 1/12/98 - Initial code from Dynamix, no changes have been made
// 1/26/98 - Greg Hoglund, added Rehup event

#ifndef _THREADBASE_H_
#define _THREADBASE_H_

#include "STRING"
#include <windows.h>

// WON Namespace
namespace WONCommon {

class ThreadBase
{
private:
   HANDLE hThread;         // Thread associated with the object
   HANDLE hStop;           // EventObject to signal a desire to stop the thread
   HANDLE hRehup;          // EventObject to signal a rehup of the threadprocess()
   HANDLE hExceptionNotify;        // Will be signalled when this thread catches an exception

   int    threadId;        // needed by the _beginthreadex() function
   int    priority;
   int    mLastError;

   std::string mName;  // Name of thread (for debugging)

   static unsigned int __stdcall ThreadRoutine( void* param );

   //Disable Copy and assignment
   ThreadBase(const ThreadBase&);
   ThreadBase& operator=(const ThreadBase&);

protected:
   //override this function: follow the example loop or risk deadlock!!!
   virtual int threadProcess();

public:
   explicit ThreadBase(const char* theName=NULL);
   virtual ~ThreadBase();

   virtual void startThread();
   virtual void stopThread();

   bool IsRunning() const { return (hThread != 0); }

   HANDLE getHandle();
   int  getId() const;
   int  getPriority() const;
   void setPriority(int priority);
   HANDLE getStopEvent();
   HANDLE getRehupEvent();
   int  getLastError() const { return mLastError; }
   const std::string& getName() const;

   HANDLE   getExceptionNotifyEvent() { return(hExceptionNotify); }
   void     setExceptionNotifyEvent(HANDLE theEvent) { hExceptionNotify = theEvent; }
};


inline HANDLE ThreadBase::getHandle()
{
   return ( hThread );
}

inline int ThreadBase::getId() const
{
    return threadId;
}

inline HANDLE ThreadBase::getStopEvent()
{
   return hStop;
}

inline HANDLE ThreadBase::getRehupEvent()
{
    return hRehup;
}

inline void ThreadBase::setPriority(int p)
{
   priority = p;
   if (hThread)
       SetThreadPriority(hThread, priority);
}

inline int ThreadBase::getPriority() const
{
   return (priority);
}

inline const std::string& ThreadBase::getName() const
{
   return (mName);
}

};  // Namespace WON

#endif //_THREADBASE_H_
