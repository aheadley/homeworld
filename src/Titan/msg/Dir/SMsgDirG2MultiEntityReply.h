#ifndef _SMsgDirG2MultiEntityReply_H
#define _SMsgDirG2MultiEntityReply_H

// SMsgDirG2MultiEntityReply.h

// Directory multi entity reply.  Reply for query request that can return one or
// more entities.


#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "SMsgDirG2EntityListBase.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgDirG2MultiEntityReply : public SMsgDirG2EntityListBase
{
public:
	// Default ctor
	SMsgDirG2MultiEntityReply(void);

	// SmallMessage ctor - will throw if SmallMessage type is not of this type
	explicit SMsgDirG2MultiEntityReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2MultiEntityReply(const SMsgDirG2MultiEntityReply& theMsgR);

	// Destructor
	~SMsgDirG2MultiEntityReply(void);

	// Assignment
	SMsgDirG2MultiEntityReply& operator=(const SMsgDirG2MultiEntityReply& theMsgR);

	// Virtual Duplicate from SmallMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Status access
	ServerStatus GetStatus(void) const;
	void         SetStatus(ServerStatus theStatus);

	// Flags for pack/unpack entities (from SMsgDirG2EntityBase)
	unsigned long GetFlags() const;
	void          SetFlags(unsigned long theFlags);
	
	// Get current size of messsage (in bytes)
	unsigned long ComputePackSize() const;


private:
	ServerStatus  mStatus;     // Request status
	unsigned long mFlags;      // Get flags
};


// Inlines
inline TRawMsg*
SMsgDirG2MultiEntityReply::Duplicate(void) const
{ return new SMsgDirG2MultiEntityReply(*this); }

inline ServerStatus
SMsgDirG2MultiEntityReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgDirG2MultiEntityReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline unsigned long
SMsgDirG2MultiEntityReply::GetFlags() const
{ return mFlags; }

inline void
SMsgDirG2MultiEntityReply::SetFlags(unsigned long theFlags)
{ mFlags = theFlags; }


};  // Namespace WONMsg

#endif