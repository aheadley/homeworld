#ifndef _EVENT_LOG_H_INCLUDED_
#define _EVENT_LOG_H_INCLUDED_

// EventLog

// Class that encapsulates writing messages to the EventLog.  Uses C++
// streams fill buffer to be logged.  A global instance of this class should be
// created with the initialixzation constructor and maintained for the life of
// the application.  Other instances may be created very cheaply,
// used and then discarded.

// Note that if the connection to the Event log cannot be created, log
// attempts are written to the LASTCHANCE_FILE.  If writes to this file
// fail, the log message is discarded.  Use the IsOK method to see if a
// connection to the actual event log exists or not (if you care).

// Note that event logs are an NT thing and under 95, all logs will go to
// the LASTCHANCE_FILE: C:\temp\WONEventLog.log

// **NOTE**
// Win95 has no event log.  Ergo, logging to the system event log is only enabled
// for NT4.  You must define _WIN32_WINNT = 0x0400 before compiling EventLog.cpp
// to enable logging to the system event log.
// **NOTE**


#include <sstream>
#include "STRING"
#include <windows.h>
#include "CriticalSection.h"

// EventLog in WON namespace
namespace WONCommon
{

// Forwards
class MemoryException;

class EventLog
{
public:
    // Types
    enum EventType { EventDefault, EventError, EventWarn, EventInfo };

    // Standard Constructors
    explicit EventLog(const char* theP=NULL, EventType theDefault=EventError);
    explicit EventLog(EventType theDefault);

    // Initialization constructor
    // Instance created by this constructor maintains EventLog connection.
    // Use this constructor once to make a global instance.  After that just
    // use the standard constructor.  Make sure instance created with this
    // constructor is not deleted until app exits.
    EventLog(const std::string& theAppName, const std::string& theLogicalName);

    // Destructor
    ~EventLog(void);

    // Are we connected to the event log
    bool IsOK(void) const;

    // Log the event, add theP to event text if defined
    void Log(EventType theType);
    void Log(const char* theP=NULL, EventType theType=EventDefault);

    // Clear any existing text from the buffer
    void Clear(void);

    // Default event type access
    EventType GetDefaultType() const;
    void      SetDefaultType(EventType theType);

    // Get the log stream
    std::ostream& GetStream(void);

private:
    EventType         mDefType;  // Default event log type
    std::stringstream mStream;   // Logging buffer (stream)

    // Static Members
    static std::string   mAppName;    // App name
    static std::string   mAppId;      // App identifier (LogicalName and PID)
    static HANDLE        mHandle;     // Event log handle
    static long          mInstCt;     // Instance count
    static std::ofstream mEventFile;  // Last chance file to log events

    // Critical section for logging
    static WONCommon::CriticalSection mLogCrit;

    // Private Methods
    void AddLogicalName(void);

    // Static methods
    static void LogEvent(EventType theType, const char* theP);

    // Disable Copy and Assignment
    EventLog(const EventLog&);
    EventLog& operator=(const EventLog&);

    // Friends
    friend class MemoryException;
};


// Inlines
inline std::ostream&
EventLog::GetStream(void)
{ return mStream; }

inline void
EventLog::Clear(void)
{ mStream.str(std::string()); }

inline EventLog::EventType
EventLog::GetDefaultType() const
{ return mDefType; }

inline void
EventLog::SetDefaultType(EventType theType)
{ mDefType = theType; }

};  //namespace WON

std::ostream& operator<<(std::ostream& os, WONCommon::EventLog::EventType aType);

#endif /* _EVENT_LOG_H_INCLUDED_ */