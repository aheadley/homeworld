#ifndef _TMsgDirRenewService_H
#define _TMsgDirRenewService_H

// TMsgDirRenewService.h

// DirectoryServer Renew Entry message.  Defines an entry to renew.


#include "msg/TMessage.h"
#include "TMsgDirServiceBase.h"

namespace WONMsg {

class TMsgDirRenewService : public TMsgDirServiceBase
{
public:
	// Default ctor
	TMsgDirRenewService(void);

	// TMessage ctor
	explicit TMsgDirRenewService(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirRenewService(const TMsgDirRenewService& theMsgR);

	// Destructor
	~TMsgDirRenewService(void);

	// Assignment
	TMsgDirRenewService& operator=(const TMsgDirRenewService& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Member access
	unsigned long GetLifespan(void) const;
	void SetLifespan(unsigned long theLifespan);

private:
	unsigned long mLifespan;
};


// Inlines
inline TRawMsg*
TMsgDirRenewService::Duplicate(void) const
{ return new TMsgDirRenewService(*this); }

inline unsigned long
TMsgDirRenewService::GetLifespan(void) const
{ return mLifespan; }

inline void
TMsgDirRenewService::SetLifespan(unsigned long theLifespan)
{ mLifespan = theLifespan; }

};  // Namespace WONMsg

#endif