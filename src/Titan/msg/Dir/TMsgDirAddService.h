#ifndef _TMsgDirAddService_H
#define _TMsgDirAddService_H

// TMsgDirAddService.h

// DirectoryServer Add Entry message.  Defines an entry to add.


#include "msg/TMessage.h"
#include "TMsgDirServiceBase.h"
#include "DirServerSKMasks.h"

namespace WONMsg {

class TMsgDirAddService : public TMsgDirServiceBase
{
public:
	// Default ctor
	TMsgDirAddService(void);

	// TMessage ctor
	explicit TMsgDirAddService(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirAddService(const TMsgDirAddService& theMsgR);

	// Destructor
	~TMsgDirAddService(void);

	// Assignment
	TMsgDirAddService& operator=(const TMsgDirAddService& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Member access
	unsigned long  GetLifespan(void) const;
	unsigned char  GetAddMask(void) const;
	unsigned short GetBlobLen(void) const;
	const void*    GetBlob(void) const;

	// Member update
	void SetLifespan(unsigned long theLifespan);
	void SetAddMask(unsigned char theMask);
	void SetBlob(const void* theBlob, unsigned short theLen);

private:
	unsigned long  mLifespan;  // Service lifespan in seconds
	unsigned char  mAddMask;   // Service add mask (SK_MASK values 'or'ed)
	unsigned short mBlobLen;   // Service data length
	unsigned char* mBlob;      // Service binary data
};


// Inlines
inline TRawMsg*
TMsgDirAddService::Duplicate(void) const
{ return new TMsgDirAddService(*this); }

inline unsigned long
TMsgDirAddService::GetLifespan(void) const
{ return mLifespan; }

inline unsigned char
TMsgDirAddService::GetAddMask(void) const
{ return mAddMask; }

inline unsigned short
TMsgDirAddService::GetBlobLen(void) const
{ return mBlobLen; }

inline const void*
TMsgDirAddService::GetBlob(void) const
{ return mBlob; }

inline void
TMsgDirAddService::SetLifespan(unsigned long theLifespan)
{ mLifespan = theLifespan; }

inline void
TMsgDirAddService::SetAddMask(unsigned char theMask)
{ mAddMask = theMask; }

};  // Namespace WONMsg

#endif