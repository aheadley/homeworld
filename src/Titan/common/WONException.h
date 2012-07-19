#ifndef _WONException_H
#define _WONException_H

// WONException

// Defines Exception base class for WON exceptions.  All WON exceptions
// should derive from this class.

// Also defines MemoryException which is thrown by the installed new_handler
// if memory is exhausted.  A new_handler is installed by WONException when
// its static Init method is called.

// Exception Codes and standard text are defined in the WONExceptCodes.h header.

// **NOTE**
// Always catch WONExceptions by REFERENCE.  This will improve performance and
// limit ownership and exception logging issues.
// **NOTE**


#include <sstream>
#include "STRING"
#include <exception>

#ifndef _WON_EXCEPTION_SOURCE
#include "WONExceptCodes.h"
#endif

// WONException in WONEx namespace
namespace WONCommon
{

// Exception base class.  All WON exceptions, except MemoryException are
// derived from this class
class WONException : public exception
{
public:
    // Types
    enum ExState {
        Continuing,    // Source continues
        StateChanged,  // Source continues, but it's state was changed
        Critical       // Source is dead/corrupt (default)
    };

    // Constructors/Destructor
    // Usage: WONException anEx(code, __FILE__, __LINE__ [,text]);
    explicit WONException(int theCode=0, int theLine=0, const char* theFileP=NULL,
                          const char* addTextP=NULL) throw();

    WONException(const WONException& theExR) throw();
    virtual ~WONException(void) throw();

    // Assignment
    WONException& operator=(const WONException& theExR) throw();

    // Build from another WONException
    // Copies code, line, file, and text.  Disables logging in source exception
    void BuildFrom(const WONException& theExR) throw();

    // Fetch exception text
    virtual const char* what(void) const throw();

    // Throw this exception, reset line and file is desired.
    virtual void Raise(int theLine=0, const char* theFileP=NULL);

    // Get the exception stream
    std::ostream& GetStream(void) throw();

    // Log to event log or not
    bool         GetLog(void) const throw();
    virtual void SetLog(bool theFlag) throw();

    // Get/set exception state
    ExState      GetState(void) const throw();
    virtual void SetState(ExState theState) throw();

    // Access to data
    int                GetCode(void) const throw();
    const std::string& GetFile(void) const throw();
    int                GetLine(void) const throw();

    // Static init method, also sets _new_handler.  Call once!
    static void Init(void) throw();

protected:
    ExState           mState;   // Exception state (default=Critical)
    int               mCode;    // Exception code
    std::string       mFile;    // File where exception occured.
    int               mLine;    // Line where exception occured
    std::stringstream mStream;  // Exception buffer (stream)

private:
    bool                mLogIt;  // Log to event log?
    mutable std::string mWhat;   // Buffer used by what method
};


// Exception class for out of memory
// Should be derived from bad_alloc, but this class doesn't seem to exist
// in VC5.
// This class is used by the installed new_handler when no memory is
// available.  The new_handler uses a static instance so no memory is
// allocated.
class MemoryException : public exception
{
public:
    // Constructors/Destructor
    MemoryException(void) throw();
    MemoryException(const MemoryException& theExR) throw();
    virtual ~MemoryException(void) throw();

    // Assignment
    MemoryException& operator=(const MemoryException& theExR) throw();

    // Fetch exception text
    virtual const char* what(void) const throw();

    // Throw this exception
    virtual void Raise(void);

    // Build the error text
    virtual void BuildText(void) throw();

    // Log to the event log
    void Log(void) const throw();

private:
    std::string mText;  // Error text

    // Class constants
    static const unsigned long EX_CODE;  // MemoryException code

};


// Inlines
inline std::ostream&
WONException::GetStream(void) throw()
{ mWhat.erase();  return mStream; }

inline bool
WONException::GetLog(void) const throw()
{ return mLogIt; }

inline WONException::ExState
WONException::GetState(void) const throw()
{ return mState; }

inline int
WONException::GetCode(void) const throw()
{ return mCode; }

inline const std::string&
WONException::GetFile(void) const throw()
{ return mFile; }

inline int
WONException::GetLine(void) const throw()
{ return mLine; }

inline
MemoryException::MemoryException(void) throw() :
    exception(),
    mText()
{}

inline
MemoryException::MemoryException(const MemoryException& theExR) throw() :
    exception(theExR),
    mText(theExR.mText)
{}

inline
MemoryException::~MemoryException(void) throw()
{}

inline MemoryException&
MemoryException::operator=(const MemoryException& theExR) throw()
{ exception::operator=(theExR);  mText = theExR.mText;  return *this; }

};  // namespace WONEx


// ** Output Operators **
std::ostream&
operator<<(std::ostream& os, WONCommon::WONException::ExState theState);

inline std::ostream&
operator<<(std::ostream& os, const WONCommon::WONException& theExR)
{ os << theExR.what();  return os; }


#endif