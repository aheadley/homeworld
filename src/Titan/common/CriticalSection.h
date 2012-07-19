#ifndef _CRITICALSECTION_H
#define _CRITICALSECTION_H

// CriticalSection

// Classes for managing Windows critical sections.

// CriticalSection, a class that manages the CRITICAL_SECTION
// structure.  This class will initializes and deletes the section automatically
// and has enter/try/leave methods.

// AutoCrit, an exception safe class for performing enter/try/leave operations
// on a CritcalSection object.  AutoCrit will enter a CriticalSection on
// construction and leave a CriticalSection when it goes out of scope.

// **NOTE**
// Win95 has no TryEnterCriticalSection.  Ergo the TryEnter functionality is only
// enabled for NT4 builds.  You must define _WIN32_WINNT = 0x0400 before compiling
// to enable the TryEnter functionality.  Calling TryEnter under a Win95 build will
// perform an Enter().

// **NOTE** These classes are implemented inline.


#ifdef _LINUX
#include "linuxGlue.h"
#elif defined(WIN32)
#include <windows.h>
#else
#error unknown platform
#endif

//  In WONCommon namespace
namespace WONCommon
{

class CriticalSection
{
public:
	// Constructor / Destructor
	CriticalSection();
	~CriticalSection();

	// Enter the critical section
	void Enter();

	// Leave the critical section
	void Leave();

	// Try to enter critical section
	// Returns true if entered, false if not.
	// ** This does a blocking Enter() under Win95! **
	bool TryEnter();

private:
	CRITICAL_SECTION mCrit;    // The Critical Section

	// Disable Copy and Assignment
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);
};


class AutoCrit
{
public:
	// Constructor / Destructor
	explicit AutoCrit(CriticalSection& theCrit, bool enterNow=true);
	~AutoCrit();

	// Enter the critical section
	void Enter();

	// Leave the critical section
	void Leave();

	// Try to enter critical section
	// Returns true if entered, false if not.
	// ** This does a blocking Enter() under Win95! **
	bool TryEnter();

private:
	CriticalSection& mCritR;    // The CriticalSection
	unsigned int     mEnterCt;  // How many times entered?

	// Disable default ctor, copy, and assignment
	AutoCrit();
	AutoCrit(const AutoCrit&);
	AutoCrit& operator=(const AutoCrit&);
};


// Inlines
inline
CriticalSection::CriticalSection()
{ InitializeCriticalSection(&mCrit); }

inline
CriticalSection::~CriticalSection()
{ DeleteCriticalSection(&mCrit); }

inline void
CriticalSection::Enter()
{ EnterCriticalSection(&mCrit); }

inline void
CriticalSection::Leave()
{ LeaveCriticalSection(&mCrit); }

inline bool
CriticalSection::TryEnter()
{
#if (_WIN32_WINNT >= 0x0400)
	return (TryEnterCriticalSection(&mCrit) != 0);
#else
	Enter();  return true;
#endif
}

inline
AutoCrit::AutoCrit(CriticalSection& theCrit, bool enterNow) :
	mCritR(theCrit),
	mEnterCt(0)
{ if (enterNow) Enter(); }

inline
AutoCrit::~AutoCrit()
{ while (mEnterCt > 0) { mCritR.Leave();  mEnterCt--; } }

inline void
AutoCrit::Enter()
{ mCritR.Enter();  mEnterCt++; }

inline void
AutoCrit::Leave()
{ if (mEnterCt > 0) { mCritR.Leave();  mEnterCt--; } }

inline bool
AutoCrit::TryEnter()
{ bool aRet = mCritR.TryEnter();  if (aRet) mEnterCt++;  return aRet; }

};  //namespace WONCommon

#endif