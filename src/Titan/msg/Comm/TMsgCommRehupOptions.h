#ifndef _TMsgCommRehupOptions_H
#define _TMsgCommRehupOptions_H

// TMsgCommRehupOptions.h

// Common Message Rehup Options Message class.  Causes WON servers to re-read
// their options from the registry/command line.


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommRehupOptions : public TMessage
{
public:
	// Default ctor
	TMsgCommRehupOptions(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommRehupOptions(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommRehupOptions(const TMsgCommRehupOptions& theMsgR);

	// Destructor
	~TMsgCommRehupOptions(void);

	// Assignment
	TMsgCommRehupOptions& operator=(const TMsgCommRehupOptions& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

private:
};


// Inlines
inline TRawMsg*
TMsgCommRehupOptions::Duplicate(void) const
{ return new TMsgCommRehupOptions(*this); }


};  // Namespace WONMsg

#endif