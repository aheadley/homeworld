#ifndef _WONDBTypes_H_
#define _WONDBTypes_H_

//
// WON Database Types
// Types and constants used to communicate with the database (Oracle at this time).

#include "STRING"

namespace WONDatabase
{
    enum DBConstants
    {
        DB_NAMESTANDARD_MAX = 65,
        DB_DESCSTANDARD_MAX = 65
    };

    // DB array types
    typedef std::basic_string<unsigned char> DBBlob;          // VARBINARY
    typedef std::wstring                     DBVarCharWide;   // VARCHAR as Wide
    typedef std::string                      DBVarCharAscii;  // VARCHAR as ASCII

    // DB Number types
    typedef __int8  DBNumber2;     // NUMBER(2)  =  7 bits + sign-bit
    typedef __int16 DBNumber4;     // NUMBER(4)  = 14 bits + sign-bit
    typedef __int32 DBNumber8;     // NUMBER(8)  = 27 bits + sign-bit
    typedef __int64 DBNumber10;    // NUMBER(10) = 34 bits + sign-bit
    typedef double  DBNumber10_2;  // NUMBER(10,2)

    // DB Boolean
    typedef bool DBBoolean;

    // DB Date/Time
    typedef std::string DBDateTime;

    // DB Sequence types
    // Seq Identifer is a special case, these are stored s Seq_standard in the DB
    // but are restricted to a max of ULONG_MAX and thus can be stored as a ulong.
    typedef unsigned __int64 DBSeqStandard;   // NUMBER(10)
    typedef unsigned __int32 DBSeqIdentifier; // NUMBER(10) w/ max of ULONG_MAX
    typedef unsigned __int16 DBSeqSmall;      // NUMBER(4)
    typedef unsigned __int8  DBSeqTiny;       // NUMBER(2)

    // Standard column types
    typedef DBVarCharWide  DBNameStandard;  // VARCHAR2(65)
    typedef DBVarCharAscii DBDescStandard;  // VARCHAR2(65)
};

#endif