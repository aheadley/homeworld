#ifndef _CryptException_H
#define _CryptException_H

// CryptException

// Defines Crypt Exception class.  Instances of this class are to be thrown when
// exceptions within the crypt lib are encountered.

// Exception Codes and standard text are defined in the WONExceptCodes.h header.

// **NOTE**
// Always catch CryptException by REFERENCE.  This will improve performance and
// limit ownership and exception logging issues.
// **NOTE**


#include "common/WONException.h"

// CryptException in WONCrypt namespace
namespace WONCrypt
{

class CryptException : public WONCommon::WONException
{
public:
	// Standard ctor from WONException
	explicit CryptException(int theCode=0, int theLine=0, const char* theFileP=NULL,
	                        const char* addTextP=NULL) throw();

	// Copy ctor
	CryptException(const CryptException& theExR) throw();

	// Destructor
	virtual ~CryptException(void) throw();

	// Assignment
	CryptException& operator=(const CryptException& theExR) throw();

protected:

private:
};


// Inlines
inline
CryptException::CryptException(int theCode, int theLine, const char* theFileP,
                               const char* addTextP) throw() :
	WONException(theCode, theLine, theFileP, addTextP)
{}

inline
CryptException::CryptException(const CryptException& theExR) throw() :
	WONException(theExR)
{}

inline
CryptException::~CryptException(void) throw()
{}

inline CryptException&
CryptException::operator=(const CryptException& theExR) throw()
{ WONException::operator=(theExR);  return *this; }

};  // namespace WONCrypt


#endif