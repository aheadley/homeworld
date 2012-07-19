#ifndef _WONDEFS_H
#define _WONDEFS_H

// won.h
// Standard header file for WON modules

// Turn off all of the level 4 warnings that will pop up if we are using the STL.
#pragma warning(disable : 4097) // Typedef-name used as synonym for class name
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4290) // C++ exception specification ignored
#pragma warning(disable : 4514) // Unreferenced inline function removed
#pragma warning(disable : 4710) // Inline function not expanded
#pragma warning(disable : 4786) // Turn off annoying debug trunc message

// Here's an oxymoron, but it keeps windows.h from including everything.
#define WIN32_LEAN_AND_MEAN

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(disable : 4018) // Signed/unsigned mismatch.
#pragma warning(disable : 4100) // Unreferenced formal parameter.
#pragma warning(disable : 4244) // conversion may loose data.
#pragma warning(disable : 4511) // Copy constructor could not be generated.
#pragma warning(disable : 4512) // Assignment operator could not be generated.
#pragma warning(disable : 4530) // Exception handler used w/o unwind semantics.
#pragma warning(disable : 4663) // Language change (w/r/t templates).
#pragma warning(push, 3) // Turn off warnings within the STL.
#endif // WARNING_LVL_4_AND_STL

// Standard headers
#include "STRING"
#include <time.h>
#include "MEMORY"
#include "ALGORITHM"

#ifdef WIN32
#include <windows.h>
#elif defined(_LINUX)
#include "linuxGlue.h"
#else
#error unknown platform
#endif

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(pop) // Restore warnings.
#endif // WARNING_LVL_4_AND_STL

#include "WONDebug.h"

// Pull in common stuff from std namespace
using std::string;
using std::wstring;
using std::auto_ptr;
using std::endl;
using std::ostream;

// Common Definitions and Utility Methods
namespace WONCommon {

    // Raw binary buffer type
    typedef std::basic_string<unsigned char> RawBuffer;

    // Case insensitive string comparison
    int StrcmpNoCase(const string& s1, const string& s2);
    int StrcmpNoCase(const wstring& s1, const wstring& s2);

    // Convert string to a WONDebugLevel
    WONDebug::Debugger::DebugLevel StrToDebugLevel(const string& theStr);

    // ASCII <--> Wide conversion
    unsigned short* AsciiToWide(unsigned short* wBufP, const char* aBufP, size_t nChars);
    char*           WideToAscii(char* aBufP, const unsigned short* wBufP, size_t nChars);

    wstring StringToWString(const string& theStr);
    wstring StringToWString(const string& theStr, size_t n);
    wstring StringToWString(const string& theStr, size_t pos, size_t n);

    wstring StringToWString(const char* theStr);
    wstring StringToWString(const char* theStr, size_t n);
    wstring StringToWString(const char* theStr, size_t pos, size_t n);

    void    StringToWString(const string& src, wstring& dst);
    void    StringToWString(const string& src, size_t n, wstring& dst);
    void    StringToWString(const string& src, size_t pos, size_t n, wstring& dst);

    void    StringToWString(const char* src, wstring& dst);
    void    StringToWString(const char* src, size_t n, wstring& dst);
    void    StringToWString(const char* src, size_t pos, size_t n, wstring& dst);

    string  WStringToString(const wstring& theStr);
    string  WStringToString(const wstring& theStr, size_t n);
    string  WStringToString(const wstring& theStr, size_t pos, size_t n);

    string  WStringToString(const unsigned short* theStr);
    string  WStringToString(const unsigned short* theStr, size_t n);
    string  WStringToString(const unsigned short* theStr, size_t pos, size_t n);

    void    WStringToString(const wstring& src, string& dst);
    void    WStringToString(const wstring& src, size_t n, string& dst);
    void    WStringToString(const wstring& src, size_t pos, size_t n, string& dst);

    void    WStringToString(const unsigned short* theStr, string& dst);
    void    WStringToString(const unsigned short* theStr, size_t n, string& dst);
    void    WStringToString(const unsigned short* theStr, size_t pos, size_t n, string& dst);

    string  WStringToString(const wstring& theStr);
    string  WStringToString(const unsigned short* theStr);

    void    WStringToString(const wstring& src, string& dst);
    void    WStringToString(const unsigned short* src, string& dst);

    bool ConvertNumberStringToRawBytes(const string& theString, RawBuffer& theRawBytesR, unsigned char theBase = 10);
    bool ConvertWideStringToRawBytes(const wstring& theString, RawBuffer& theRawBytesR);
    bool ConvertStringToRawBytes(const string& theString, RawBuffer& theRawBytesR);

    bool ConvertRawBytesToString(const RawBuffer& theRawBytes, string& theStringR);
    bool ConvertRawBytesToWideString(const RawBuffer& theRawBytes, wstring& theStringR);

    inline string TimeToString( time_t theTime, bool useLocalTime=false )
    {
        string aTimeStr;
        if( useLocalTime )
            aTimeStr = asctime( localtime( &theTime ) );
        else
            aTimeStr = asctime( gmtime( &theTime ) );
        aTimeStr.erase( aTimeStr.size()-1, 1); // remove trailing newline
        return aTimeStr;
    }

    // convert strings or wstrings in place to upper or lower case.
    inline void CvUpper( string &theString )
    { std::transform( theString.begin(),theString.end(),theString.begin(), toupper ); }
    inline void CvLower( string &theString )
    { std::transform( theString.begin(),theString.end(),theString.begin(), tolower );}
    inline void CvUpper( wstring &theWString )
    { std::transform( theWString.begin(),theWString.end(),theWString.begin(), toupper ); }
    inline void CvLower( wstring &theWString )
    { std::transform( theWString.begin(),theWString.end(),theWString.begin(), tolower ); }

    // convert strings or wstrings to upper or lower case.
    inline string ToUpper( const string &theString )
    {
        string aString( theString );
        std::transform( aString.begin(),aString.end(),aString.begin(), toupper );
        return( aString );
    }
    inline string ToLower( const std::string &theString )
    {
        string aString( theString );
        std::transform( aString.begin(),aString.end(),aString.begin(), tolower );
        return( aString );
    }

    inline wstring ToUpper( const std::wstring &theWString )
    {
        wstring aWString( theWString );
        std::transform( aWString.begin(),aWString.end(),aWString.begin(), toupper );
        return( aWString );
    }
    inline wstring ToLower( const std::wstring &theWString )
    {
        wstring aWString( theWString );
        std::transform( aWString.begin(),aWString.end(),aWString.begin(), tolower );
        return( aWString );
    }

};

// Output operators
inline ostream&
operator<<(ostream& os, const wstring& aStr)
{ os << WONCommon::WStringToString(aStr);  return os; }

inline ostream&
operator<<(ostream& os, const unsigned short* aBuf)
{ os << wstring(aBuf);  return os; }

#ifdef WIN32

inline ostream&
operator<<(ostream& os, __int64 theValue)
{
    char s[21];

    // *NOTE*
    // Uncomment the following if block for VC5

    // There's a bug in VC5's _i64toa.  It's not able to convert negative numbers
    // correctly.  Get around this by outputting our own negative sign and then
    // the "positive version" of the number.
    //if (theValue < 0)
        //os << "-" << _i64toa(-theValue, s, 16);
    //else
        os << _i64toa(theValue, s, 16);
    return os;
}

#endif

#endif