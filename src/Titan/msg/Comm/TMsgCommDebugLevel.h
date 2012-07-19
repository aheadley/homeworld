#ifndef _TMsgCommDebugLevel_H
#define _TMsgCommDebugLevel_H

// TMsgCommDebugLevel.h

// Common Debug Level Message class.  Allows a debug level to be sent to
// WON servers.  See WONDebug.h for debug level definitions.


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommDebugLevel : public TMessage
{
public:
	// Default ctor
	TMsgCommDebugLevel(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommDebugLevel(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommDebugLevel(const TMsgCommDebugLevel& theMsgR);

	// Destructor
	~TMsgCommDebugLevel(void);

	// Assignment
	TMsgCommDebugLevel& operator=(const TMsgCommDebugLevel& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// DebugLevel access
	const std::string& GetDebugLevel(void) const;
	void               SetDebugLevel(const std::string& theLevel);

private:
	std::string mDebugLevel;  // Debug level
};


// Inlines
inline TRawMsg*
TMsgCommDebugLevel::Duplicate(void) const
{ return new TMsgCommDebugLevel(*this); }

inline const std::string&
TMsgCommDebugLevel::GetDebugLevel(void) const
{ return mDebugLevel; }

inline void
TMsgCommDebugLevel::SetDebugLevel(const std::string& theLevel)
{ mDebugLevel = theLevel; }


};  // Namespace WONMsg

#endif