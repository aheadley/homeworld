#ifndef _TMsgCommTimeout_H
#define _TMsgCommTimeout_H

// TMsgCommTimeout.h

// Common Message Timeout Message class.  Allows the message timeout for
// WON servers to be set.  May be set to infinite (value <= 0) or a number
// or milliseconds (value > 0)


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommTimeout : public TMessage
{
public:
	// Default ctor
	TMsgCommTimeout(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommTimeout(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommTimeout(const TMsgCommTimeout& theMsgR);

	// Destructor
	~TMsgCommTimeout(void);

	// Assignment
	TMsgCommTimeout& operator=(const TMsgCommTimeout& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// DebugLevel access
	long GetTimeout(void) const;
	void SetTimeout(long theValue);

private:
	long mTimeout;  // Timeout value
};


// Inlines
inline TRawMsg*
TMsgCommTimeout::Duplicate(void) const
{ return new TMsgCommTimeout(*this); }

inline long
TMsgCommTimeout::GetTimeout(void) const
{ return mTimeout; }

inline void
TMsgCommTimeout::SetTimeout(long theValue)
{ mTimeout = theValue; }


};  // Namespace WONMsg

#endif