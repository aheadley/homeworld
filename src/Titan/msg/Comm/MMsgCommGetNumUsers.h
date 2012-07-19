#ifndef _MMsgCommGetNumUsers_H
#define _MMsgCommGetNumUsers_H

// MMsgCommGetNumUsers.h

#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgCommGetNumUsers : public MiniMessage
{
public:
	// Default ctor
	MMsgCommGetNumUsers(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommDebugLevel
	explicit MMsgCommGetNumUsers(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommGetNumUsers(const MMsgCommGetNumUsers& theMsgR);

	// Destructor
	~MMsgCommGetNumUsers(void);

	// Assignment
	MMsgCommGetNumUsers& operator=(const MMsgCommGetNumUsers& theMsgR);

	// Virtual Duplicate from MiniMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	unsigned short GetTag() const;
	void SetTag(unsigned short theTag);
private:
	unsigned short mTag;
};


class MMsgCommGetNumUsersReply : public MiniMessage
{
public:
	// Default ctor
	MMsgCommGetNumUsersReply(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommDebugLevel
	explicit MMsgCommGetNumUsersReply(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommGetNumUsersReply(const MMsgCommGetNumUsersReply& theMsgR);

	// Destructor
	~MMsgCommGetNumUsersReply(void);

	// Assignment
	MMsgCommGetNumUsersReply& operator=(const MMsgCommGetNumUsersReply& theMsgR);

	// Virtual Duplicate from MiniMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	unsigned short GetTag() const;
	unsigned short GetNumActiveUsers() const;
	unsigned short GetUserCapacity() const;

	void SetTag(unsigned short theTag);
	void SetNumActiveUsers(unsigned short theNumActiveUsers);
	void SetUserCapacity(unsigned short theUserCapacity);
private:
	unsigned short mTag;
	unsigned short mNumActiveUsers;
	unsigned short mUserCapacity;
};


// Inlines
inline TRawMsg*
MMsgCommGetNumUsers::Duplicate(void) const
{ return new MMsgCommGetNumUsers(*this); }

inline TRawMsg*
MMsgCommGetNumUsersReply::Duplicate(void) const
{ return new MMsgCommGetNumUsersReply(*this); }

inline unsigned short MMsgCommGetNumUsers::GetTag() const
{ return mTag; }
inline void MMsgCommGetNumUsers::SetTag(unsigned short theTag)
{ mTag = theTag; }

inline unsigned short MMsgCommGetNumUsersReply::GetTag() const
{ return mTag; }
inline unsigned short MMsgCommGetNumUsersReply::GetNumActiveUsers() const
{ return mNumActiveUsers; }
inline unsigned short MMsgCommGetNumUsersReply::GetUserCapacity() const
{ return mUserCapacity; }
inline void MMsgCommGetNumUsersReply::SetTag(unsigned short theTag)
{ mTag = theTag; }
inline void MMsgCommGetNumUsersReply::SetNumActiveUsers(unsigned short theNumActiveUsers)
{ mNumActiveUsers = theNumActiveUsers; }
inline void MMsgCommGetNumUsersReply::SetUserCapacity(unsigned short theUserCapacity)
{ mUserCapacity = theUserCapacity; }

};  // Namespace WONMsg

#endif