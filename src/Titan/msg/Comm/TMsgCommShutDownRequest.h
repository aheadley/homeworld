#if !defined(TMsgCommShutDownRequest_H)
#define TMsgCommShutDownRequest_H

// TMsgCommShutDownRequest.h

// Message that is used to request a process to shutdown


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgCommShutDownRequest : public TMessage {

public:
	// Default ctor
	TMsgCommShutDownRequest(void);

	// TMessage ctor
	explicit TMsgCommShutDownRequest(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommShutDownRequest(const TMsgCommShutDownRequest& theMsgR);

	// Destructor
	virtual ~TMsgCommShutDownRequest(void);

	// Assignment
	TMsgCommShutDownRequest& operator=(const TMsgCommShutDownRequest& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	unsigned short GetPortID(void) const;

	virtual void SetPortID(unsigned short thePortID);

protected:
	unsigned short mPortID;

};


// Inlines
inline TRawMsg* TMsgCommShutDownRequest::Duplicate(void) const
{ return new TMsgCommShutDownRequest(*this); }

inline unsigned short TMsgCommShutDownRequest::GetPortID(void) const
{ return mPortID; }

inline void TMsgCommShutDownRequest::SetPortID(unsigned short thePortID)
{ mPortID = thePortID; }

};  // Namespace WONMsg

#endif