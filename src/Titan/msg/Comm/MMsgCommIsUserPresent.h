#ifndef _MMsgCommIsUserPresent_H
#define _MMsgCommIsUserPresent_H

// MMsgCommIsUserPresent.h

#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {


const BYTE cShouldSendNackBit  = 1;
const BYTE cCaseInsensitiveBit = 2;

class MMsgCommIsUserPresent : public MiniMessage
{
public:
	// Default ctor
	MMsgCommIsUserPresent(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommDebugLevel
	explicit MMsgCommIsUserPresent(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommIsUserPresent(const MMsgCommIsUserPresent& theMsgR);

	// Destructor
	~MMsgCommIsUserPresent(void);

	// Assignment
	MMsgCommIsUserPresent& operator=(const MMsgCommIsUserPresent& theMsgR);

	// Virtual Duplicate from MiniMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	unsigned short GetTag() const;
	bool GetShouldSendNack() const;
	bool GetCaseInsensitive() const;
	const std::wstring GetUserName() const;

	void SetTag(unsigned short theTag);
	void SetShouldSendNack(bool shouldSendNack);
	void SetCaseInsensitive(bool caseInsensitive);
	void SetUserName(const std::wstring theUserName);
private:
	unsigned short mTag;
	bool           mShouldSendNack;
	bool           mCaseInsensitive;
	std::wstring   mUserName;
};


class MMsgCommIsUserPresentReply : public MiniMessage
{
public:
	// Default ctor
	MMsgCommIsUserPresentReply(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommDebugLevel
	explicit MMsgCommIsUserPresentReply(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommIsUserPresentReply(const MMsgCommIsUserPresentReply& theMsgR);

	// Destructor
	~MMsgCommIsUserPresentReply(void);

	// Assignment
	MMsgCommIsUserPresentReply& operator=(const MMsgCommIsUserPresentReply& theMsgR);

	// Virtual Duplicate from MiniMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	unsigned short GetTag() const;
	bool GetIsUserPresent() const;

	void SetTag(unsigned short theTag);
	void SetIsUserPresent(bool isUserPresent);
private:
	unsigned short mTag;
	bool           mIsUserPresent;
};


// Inlines
inline TRawMsg*
MMsgCommIsUserPresent::Duplicate(void) const
{ return new MMsgCommIsUserPresent(*this); }

inline TRawMsg*
MMsgCommIsUserPresentReply::Duplicate(void) const
{ return new MMsgCommIsUserPresentReply(*this); }

inline unsigned short MMsgCommIsUserPresent::GetTag() const
{ return mTag; }
inline bool MMsgCommIsUserPresent::GetShouldSendNack() const
{ return mShouldSendNack; }
inline bool MMsgCommIsUserPresent::GetCaseInsensitive() const
{ return mCaseInsensitive; }
inline const std::wstring MMsgCommIsUserPresent::GetUserName() const
{ return mUserName; }
inline void MMsgCommIsUserPresent::SetTag(unsigned short theTag)
{ mTag = theTag; }
inline void MMsgCommIsUserPresent::SetShouldSendNack(bool shouldSendNack)
{ mShouldSendNack = shouldSendNack; }
inline void MMsgCommIsUserPresent::SetCaseInsensitive(bool caseInsensitive)
{ mCaseInsensitive = caseInsensitive; }
inline void MMsgCommIsUserPresent::SetUserName(const std::wstring theUserName)
{ mUserName = theUserName; }

inline unsigned short MMsgCommIsUserPresentReply::GetTag() const
{ return mTag; }
inline bool MMsgCommIsUserPresentReply::GetIsUserPresent() const
{ return mIsUserPresent; }
inline void MMsgCommIsUserPresentReply::SetTag(unsigned short theTag)
{ mTag = theTag; }
inline void MMsgCommIsUserPresentReply::SetIsUserPresent(bool isUserPresent)
{ mIsUserPresent = isUserPresent; }

};  // Namespace WONMsg

#endif