#if !defined(TMsgCommShutDownConfirmation_H)
#define TMsgCommShutDownConfirmation_H

// TMsgCommShutDownConfirmation.h

// Message that is used to reply to a Shutdown request 


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgCommShutDownConfirmation : public TMessage {

public:
	// Default ctor
	TMsgCommShutDownConfirmation(void);

	// TMessage ctor
	explicit TMsgCommShutDownConfirmation(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommShutDownConfirmation(const TMsgCommShutDownConfirmation& theMsgR);

	// Destructor
	virtual ~TMsgCommShutDownConfirmation(void);

	// Assignment
	TMsgCommShutDownConfirmation& operator=(const TMsgCommShutDownConfirmation& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	short GetStatus(void) const;
	unsigned short GetPortID(void) const;

	virtual void SetStatus(short theStatus);
	virtual void SetPortID(unsigned short thePortID);

protected:
	short          mStatus;
	unsigned short mPortID;

};


// Inlines
inline TRawMsg* TMsgCommShutDownConfirmation::Duplicate(void) const
{ return new TMsgCommShutDownConfirmation(*this); }

inline short TMsgCommShutDownConfirmation::GetStatus(void) const
{ return mStatus; }

inline unsigned short TMsgCommShutDownConfirmation::GetPortID(void) const
{ return mPortID; }

inline void TMsgCommShutDownConfirmation::SetStatus(short theStatus)
{ mStatus = theStatus; }

inline void TMsgCommShutDownConfirmation::SetPortID(unsigned short thePortID)
{ mPortID = thePortID; }

};  // Namespace WONMsg

#endif