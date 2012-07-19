#ifndef _TMsgDirRenewDirectory_H
#define _TMsgDirRenewDirectory_H

// TMsgDirRenewDirectory.h

// DirectoryServer Renew SubDirectory message.  Defines a subdirectory to renew.

#include "msg/TMessage.h"
#include "TMsgDirDirectoryBase.h"

namespace WONMsg {

class TMsgDirRenewDirectory : public TMsgDirDirectoryBase
{
public:
	// Default ctor
	TMsgDirRenewDirectory(void);

	// TMessage ctor
	explicit TMsgDirRenewDirectory(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirRenewDirectory(const TMsgDirRenewDirectory& theMsgR);

	// Destructor
	~TMsgDirRenewDirectory(void);

	// Assignment
	TMsgDirRenewDirectory& operator=(const TMsgDirRenewDirectory& theMsgR);

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
TMsgDirRenewDirectory::Duplicate(void) const
{ return new TMsgDirRenewDirectory(*this); }

inline unsigned long
TMsgDirRenewDirectory::GetLifespan(void) const
{ return mLifespan; }

inline void
TMsgDirRenewDirectory::SetLifespan(unsigned long theLifespan)
{ mLifespan = theLifespan; }

};  // Namespace WONMsg

#endif