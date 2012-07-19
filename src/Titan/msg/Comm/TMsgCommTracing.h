#ifndef _TMsgCommTracing_H
#define _TMsgCommTracing_H

// TMsgCommTracing.h

// Common Tracing Message class.  Allows tracing to be turned on or
// off in WON servers.  See WONDebug.h for tracing definitions.


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommTracing : public TMessage
{
public:
	// Default ctor
	TMsgCommTracing(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommTracing(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommTracing(const TMsgCommTracing& theMsgR);

	// Destructor
	~TMsgCommTracing(void);

	// Assignment
	TMsgCommTracing& operator=(const TMsgCommTracing& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// DebugLevel access
	bool GetTracing(void) const;
	void SetTracing(bool theFlag);

private:
	bool mTracing;  // Tracing on or off
};


// Inlines
inline TRawMsg*
TMsgCommTracing::Duplicate(void) const
{ return new TMsgCommTracing(*this); }

inline bool
TMsgCommTracing::GetTracing(void) const
{ return mTracing; }

inline void
TMsgCommTracing::SetTracing(bool theFlag)
{ mTracing = theFlag; }


};  // Namespace WONMsg

#endif