#ifndef _TMsgAuth1GetPubKeys_H
#define _TMsgAuth1GetPubKeys_H

// TMsgAuth1GetPubKeys.h

// AuthServer request to fetch list of public keys for a given Auth Familty 1.
// Requests the AuthServer to return its current public key block for a family.


#include "msg/TMessage.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgAuthRawBufferBase.h"

// Forwards from WONSocket
namespace WONMsg {

// TMsgAuth1GetPubKeys - Request public key block from AuthServer
class TMsgAuth1GetPubKeys : public TMessage
{
public:
	// Default ctor
	TMsgAuth1GetPubKeys(ServiceType theServType);

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1GetPubKeys(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1GetPubKeys(const TMsgAuth1GetPubKeys& theMsgR);

	// Destructor
	~TMsgAuth1GetPubKeys();

	// Assignment
	TMsgAuth1GetPubKeys& operator=(const TMsgAuth1GetPubKeys& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

private:
};


// TMsgAuth1GetPubKeysReply - Reply to a request for public key block from
// the AuthServer
// Key block is contained in the raw buffer of the base class.
class TMsgAuth1GetPubKeysReply : public TMsgAuthRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1GetPubKeysReply(ServiceType theServType);

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1GetPubKeysReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1GetPubKeysReply(const TMsgAuth1GetPubKeysReply& theMsgR);

	// Destructor
	~TMsgAuth1GetPubKeysReply();

	// Assignment
	TMsgAuth1GetPubKeysReply& operator=(const TMsgAuth1GetPubKeysReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	// Status access
	ServerStatus GetStatus() const;
	void         SetStatus(ServerStatus theStatus);

private:
	ServerStatus mStatus;  // Status of the request
};


// Inlines
inline TRawMsg*
TMsgAuth1GetPubKeys::Duplicate(void) const
{ return new TMsgAuth1GetPubKeys(*this); }

inline TRawMsg*
TMsgAuth1GetPubKeysReply::Duplicate(void) const
{ return new TMsgAuth1GetPubKeysReply(*this); }

inline ServerStatus
TMsgAuth1GetPubKeysReply::GetStatus() const
{ return mStatus; }

inline void
TMsgAuth1GetPubKeysReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

};  // Namespace WONMsg

#endif