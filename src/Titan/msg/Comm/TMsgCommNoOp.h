#ifndef _TMsgCommNoOp_H
#define _TMsgCommNoOp_H

// TMsgCommNoOp.h

// Common Message NoOp Message class.  Can be sent to a Titan server
// without eliciting any response, error or otherwise.  Can be useful
// as a "keep-alive" message.

#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommNoOp : public TMessage
{
public:
	// Default ctor
	TMsgCommNoOp(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommNoOp(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommNoOp(const TMsgCommNoOp& theMsgR);

	// Destructor
	~TMsgCommNoOp(void);

	// Assignment
	TMsgCommNoOp& operator=(const TMsgCommNoOp& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);
};


// Inlines
inline TRawMsg*
TMsgCommNoOp::Duplicate(void) const
{ return new TMsgCommNoOp(*this); }

};  // Namespace WONMsg

#endif