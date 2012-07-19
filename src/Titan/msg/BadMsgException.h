#ifndef _BadMsgException_H
#define _BadMsgException_H

// BadMsgException

// Defines Bad Message Exception class.  Instances of this class are to be
// thrown when invalid/corrupt titan messages are encountered.

// Exception Codes and standard text are defined in the WONExceptCodes.h header.

// **NOTE**
// Always catch BadMsgExceptions by REFERENCE.  This will improve performance and
// limit ownership and exception logging issues.
// **NOTE**


#include "TMessage.h"
#include "common/WONException.h"

// BadMsgException in WONMsg namespace
namespace WONMsg
{

class BadMsgException : public WONCommon::WONException
{
public:
	// Standard ctor from WONException
	explicit BadMsgException(int theCode=0, int theLine=0, const char* theFileP=NULL,
	                         const char* addTextP=NULL) throw();

	// Ctor for a TMessage, uses ExBadTitanMessage code
	explicit BadMsgException(const BaseMessage& theMsg, int theLine=0,
	                         const char* theFileP=NULL,
	                         const char* addTextP=NULL) throw();

	// Copy ctor
	BadMsgException(const BadMsgException& theExR) throw();

	// Destructor
	virtual ~BadMsgException(void) throw();

	// Assignment
	BadMsgException& operator=(const BadMsgException& theExR) throw();
};


};  // namespace WONMsg


#endif