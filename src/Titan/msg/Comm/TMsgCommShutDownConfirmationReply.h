#if !defined(TMsgCommShutDownConfirmationReply_H)
#define TMsgCommShutDownConfirmationReply_H

// TMsgCommShutDownConfirmationReply.h

// Message that is used to reply to a ShutDownConfirmation message to cnfirm shutdown request


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgCommShutDownConfirmationReply : public TMessage {

public:
	// Default ctor
	TMsgCommShutDownConfirmationReply(void);

	// TMessage ctor
	explicit TMsgCommShutDownConfirmationReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommShutDownConfirmationReply(const TMsgCommShutDownConfirmationReply& theMsgR);

	// Destructor
	virtual ~TMsgCommShutDownConfirmationReply(void);

	// Assignment
	TMsgCommShutDownConfirmationReply& operator=(const TMsgCommShutDownConfirmationReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	short GetStatus(void) const;

	virtual void SetStatus(short theStatus);

protected:
	short mStatus;

};


// Inlines
inline TRawMsg* TMsgCommShutDownConfirmationReply::Duplicate(void) const
{ return new TMsgCommShutDownConfirmationReply(*this); }

inline short TMsgCommShutDownConfirmationReply::GetStatus(void) const
{ return mStatus; }

inline void TMsgCommShutDownConfirmationReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

};  // Namespace WONMsg

#endif