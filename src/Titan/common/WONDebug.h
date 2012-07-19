#ifndef _WONDEBUG_H
#define _WONDEBUG_H

// WONDebug.h
//
// ** WARNING **
// Debugging and tracing use a static class that is instanciated at
// load time.  DO NOT place debugging or trace statements in constructors
// or destructors of objects that are instanciated at LOAD TIME!  This
// leads to a race condition and will cause weird GPs!
//
// It is OK to use debugging/tracing in constructors/destructors of
// objects that are instanciated only at run time.
// ** WARNING **
//
// Debugging definitions, all acess via macros below.  DO NOT use
// debug class methods directly as they will not compile out like
// the macros will.  Each debug message will be on its own line
// (endl is appended automatically) and will be formatted as follows:
//
//   threadName(threadId): msg
//
// where threadName is the logical threadName (or appName if the main
// thread), threadId is the threadId, and msg is the message output.
// Messages will be output to a debug file and debug window.
//
// Setting a debug level will cause all debug methods that correspond
// to that level or a higher level to execute.  For example, setting
// debug level to WDBG_APPMED will cause all _AH and _AM macros to
// be executed.
//
// The constant WON_DEBUGENABLE must be defined to enable debugging
// and tracing in your application/library.
//
// The constant WON_TRACEENABLE must be defined to enable tracing in
// your application/library.
//
// Debugging Use:
// WDBG_INIT(level, appName, logicalName, port, dirPath)
//   Initializes debugging.  Call as early as possible within your
//   app (i.e., in main). Parameters:
//     level - Level of debug output.  Use a debug level constant
//     appName - Application name (string).
//     logicalName - Application logical name (string)
//     port - Server's main port (string)
//     dirPath - directory for log file, NULL for CWD (string)
//
// WDBG_THREADSTART(name)
//   Initialize debugging for a thread.  Call as early as possible
//   after thread starts.  Parameters:
//     name - Logical name for thread (string).
//
// WDBG_THREADSTOP
//   Cleanup when a thread ends.  Call before a thread exits.
//
// WDBG_SETLEVEL(level)
//   Set the debug level for the current thread.  If not called, the
//   debug level for a thread defaults to the one set when debug was
//   initialized in the app.  Parameters:
//     level - Level of debug output.  Use a debug level constant.
//
// WDBG_SETGLOBALLEVEL(level, allThreads)
//   Set the global debug level.  Set the allThreads parm to true to
//   update the level of all existing threads as well or to false to
//   leave the level of existing threads as is.  Parameters:
//     level - Level of debug output.  Use a debug level constant.
//     allThreads - Flag to modify all threads (bool)
//
// WDBG_SETFILE(name, logicalName, dir)
//   Change the file used for debug output.  Previous file is closed.
//   Parameters:
//     name - Name of file.  (string)
//     logicalName - Application logical name (string)
//     dirPath - directory for log file, NULL for CWD (string)
//
// WTRACE(method)
//   Trace execution of a method,  Place WTRACE as early in a method
//   as possible.  When tracing is enabled, all WTRACE macros will
//   generate a message when method starts an a end message when
//   method terminates.  Parameters:
//     method - Method name (string)
//
// WTRACE_ENABLE(flag)
//   Enable or disable tracing for current thread  If not called, the
//   tracing is enabled/disabled based on the global trace enable.
//   Parameters:
//     flag - Enable/disable tracing (bool).
//
// WTRACE_SETGLOBALENABLE(flag, allThreads)
//   Enable or disable tracing at global level.  Set the allThreads
//   parm to true to update the level of all existing threads as well
//   or to false to leave the level of existing threads as is.
//   Parameters:
//     flag - Enable or disable tracing (bool)
//     allThreads - Flag to modify all threads (bool)
//
// Message Macros: <macro>(msg)
//   Includes: WDBG_AH, WDBG_AM, WDBG_AL, WDBG_LH, WDBG_LM, WDBG_LL
//   Outputs a msg to the debug logs if current debug level allows it.
//   The msg parameter is a stream expresion without a stream.  For
//   example:
//     WDBG_AH("This is test " << 1 << " for the " << appName);
//   Remember that an endl will automatically be appended.  Each
//   macro corresponds to a debug level such as AH == APPHIG to
//   LL == LIBLOW, etc.
//
// Call Macros: <macro>(stmt)
//   Includes: WCALL_AH, WCALL_AM, WCALL_AL, WCALL_LH, WCALL_LM, WCALL_LL
//   Executes statement if current debug level allows it.  Statement
//   can be any valid C++ statement.  Each macro corresponds to a
//   debug level such as AH == APPHIG to LL == LIBLOW, etc.


// Define Debug level constants
#define WDBG_OFF     WONDebug::Debugger::Off
#define WDBG_APPHIG  WONDebug::Debugger::AppHig
#define WDBG_APPMED  WONDebug::Debugger::AppMed
#define WDBG_APPLOW  WONDebug::Debugger::AppLow
#define WDBG_LIBHIG  WONDebug::Debugger::LibHig
#define WDBG_LIBMED  WONDebug::Debugger::LibMed
#define WDBG_LIBLOW  WONDebug::Debugger::LibLow

// Enable debugging if defined
#ifdef WON_DEBUGENABLE

// Define _DEBUG if needed
#ifndef _DEBUG
#define _DEBUG
#endif

// Application level message debugging
#define WDBG_AH(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppHig)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }
#define WDBG_AM(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppMed)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }
#define WDBG_AL(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppLow)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }

// Library level message debugging
#define WDBG_LH(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibHig)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }
#define WDBG_LM(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibMed)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }
#define WDBG_LL(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibLow)) { WONDebug::Debugger::GetStream() << arg; WONDebug::Debugger::Flush(); } }

// Application level call debugging
#define WCALL_AH(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppHig)) { arg; } }
#define WCALL_AM(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppMed)) { arg; } }
#define WCALL_AL(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::AppLow)) { arg; } }

// Library level call debugging
#define WCALL_LH(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibHig)) { arg; } }
#define WCALL_LM(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibMed)) { arg; } }
#define WCALL_LL(arg)  { if (WONDebug::Debugger::CheckLevel(WONDebug::Debugger::LibLow)) { arg; } }

// Misc. Debug methods
#define WDBG_INIT(level, appName, logicalName, port, dir)  WONDebug::Debugger::Init(level, appName, logicalName, port, dir)
#define WDBG_SETLEVEL(level)  WONDebug::Debugger::SetLevel(level)
#define WDBG_SETGLOBALLEVEL(level, allThreads)  WONDebug::Debugger::SetGlobalLevel(level, allThreads)
#define WDBG_SETFILE(name, logicalName, dir)  WONDebug::Debugger::SetFile(name, logicalName, dir)
#define WDBG_THREADSTART(name)  WONDebug::Debugger::ThreadStart(name)
#define WDBG_THREADSTOP  WONDebug::Debugger::ThreadStop()

// Enable tracing if defined
#ifdef WON_TRACEENABLE

#define WTRACE(method)  WONDebug::Tracer _wonTracer(method)
#define WTRACE_ENABLE(flag)  WONDebug::Tracer::SetEnable(flag)
#define WTRACE_GLOBALENABLE(flag, allThreads)  WONDebug::Tracer::SetGlobalEnable(flag, allThreads)

// Disable tracing
#else

#define WTRACE(method)  {}
#define WTRACE_ENABLE(flag)  {}
#define WTRACE_GLOBALENABLE(flag, allThreads)  {}

#endif

// Disable all debugging
#else

#define WDBG_AH(arg)  {}
#define WDBG_AM(arg)  {}
#define WDBG_AL(arg)  {}

#define WDBG_LH(arg)  {}
#define WDBG_LM(arg)  {}
#define WDBG_LL(arg)  {}

#define WCALL_AH(arg)  {}
#define WCALL_AM(arg)  {}
#define WCALL_AL(arg)  {}

#define WCALL_LH(arg)  {}
#define WCALL_LM(arg)  {}
#define WCALL_LL(arg)  {}

#define WDBG_INIT(level, appName, logicalName, port, dir)  {}
#define WDBG_SETGLOBALLEVEL(level, allThreads)  {}
#define WDBG_SETFILE(name, logicalName, dir)  {}
#define WDBG_SETLEVEL(level)  {}
#define WDBG_THREADSTART(name)  {}
#define WDBG_THREADSTOP  {}

#define WTRACE(method)  {}
#define WTRACE_ENABLE(flag)  {}
#define WTRACE_GLOBALENABLE(flag, allThreads)  {}

#endif


// Headers
#include "STRING"
#include <fstream>

// Debug namespace
namespace WONDebug
{

// Debugger class
// Handles all debug messages and debug level information
// This is a static class.  It cannot be instanciated.

class Debugger
{
public:
    // Types
    enum DebugLevel {
        Off=0, AppHig, AppMed, AppLow, LibHig, LibMed, LibLow
    };

    // Initialization, sets global level and debug file
    static void Init(DebugLevel theLevel, const char* theAppName=NULL,
                     const char* theLogicalName=NULL, unsigned short thePort=0, const char* theDirPath=NULL);

    // Debug file methods
    static bool SetFile(const char* theAppName, const char* theLogicalName=NULL,
                        unsigned short thePort=0, const char* theDirPath=NULL);

    // Debug level methods (current thread)
    static DebugLevel GetLevel(void);
    static void       SetLevel(DebugLevel theLevel);

    // Global debug level methods
    static DebugLevel GetGlobalLevel(void);
    static void       SetGlobalLevel(DebugLevel theLevel, bool allThreads=false);

    // Thread methods
    static void ThreadStart(const char* theName=NULL);
    static void ThreadStop (void);

    // Check the debug level
    static bool CheckLevel(DebugLevel theLevel);

    // Streaming methods
    static std::ostream& GetStream(void);
    static void          Flush    (void);

private:
    // Static Members
    static DebugLevel    mGLevel;     // Global debug level
    static std::ofstream mDebugFile;  // Debug file

    // Build debug file name
    static std::string BuildFileName(const char* theAppName, const char* theLogicalName,
                                     unsigned short thePort, const char* theDirPath);

    // Static class, no construction allowed!
    Debugger(void);
};


// Tracer class
// Handles trace messages
class Tracer
{
public:
    // Constructor / Destructor
    explicit Tracer(const std::string& theMethod);
    ~Tracer(void);

    // Enable methods for current thread
    static bool GetEnable(void);
    static void SetEnable(bool theFlag=true);

    // Global enable methods
    static bool GetGlobalEnable(void);
    static void SetGlobalEnable(bool theFlag=true, bool allThreads=false);

private:
    // Members
    std::string mMethod;  // Method being traced

    // Static Members
    static int mGEnable;  // Tracing on (>= 0) or off (-1)

    // Private methods
    static std::string GetIndentStr(int theLevel);

    // Disable copy and assignment
    Tracer(const Tracer&);
    Tracer& operator=(const Tracer&);
};


// Inlines

inline Debugger::DebugLevel
Debugger::GetGlobalLevel(void)
    { return mGLevel; }

inline bool
Tracer::GetGlobalEnable(void)
    { return (mGEnable >= 0); }


};  // namespace WONDebug

#endif