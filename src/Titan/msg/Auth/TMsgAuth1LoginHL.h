#ifndef _TMsgAuth1LoginHL_H
#define _TMsgAuth1LoginHL_H

 // TMsgAuth1LoginHL.h

// AuthServer messages implementing the Auth1 Family login Half-Life protocol.
// This header/source implements messages:
//	Auth1LoginRequestHL
//	Auth1ChallengeHL
//	Auth1ConfirmHL
// It also included the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequestHL begins login for Half-Life and is sent from the Client to
// the AuthServer.

// Auth1ChallengeHL is sent in response to Auth1LoginRequest from the AuthServer
// to the client.

// Auth1ConfirmHL is sent in response to Auth1ChallengeHL from the Client to the
// AuthServer.

// Auth1RefreshHL is from the Client to the AuthServer to request extension of an
// Login session and AuthCertificate.

// Auth1LoginReply is sent in response to Auth1ConfirmHL from the AuthServer
// to the client and completes the login. It is also sent in response to
// Auth1RefreshHL, extending the login session.

#include "TMsgAuthRawBufferBase.h"
#include "TMsgAuthLastRawBufferBase.h"
#include "TMsgAuth1LoginBase2.h"

#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"

#include "auth/Auth1PublicKeyBlock.h"
#include "crypt/BFSymmetricKey.h"

// Forwards from WONSocket
namespace WONMsg {


// TMsgAuth1LoginRequestHL - begin login
//   Contains login data encrypted with public key.  Public key used is specified
//   by the KeyBlockId.  Login data is expected to contain:
//	   Key Block ID     (2 bytes)  Must match key block Id in message
//     Session Key len  (2 bytes)
//     Session Key      (var bytes == Session Key Len)
//     Half-Life CD Key (? bytes)
class TMsgAuth1LoginRequestHL : public TMsgAuthRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1LoginRequestHL();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1LoginRequestHL(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1LoginRequestHL(const TMsgAuth1LoginRequestHL& theMsgR);

	// Destructor
	~TMsgAuth1LoginRequestHL();

	// Assignment
	TMsgAuth1LoginRequestHL& operator=(const TMsgAuth1LoginRequestHL& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	// KeyBlock ID access
	unsigned short GetKeyBlockId() const;
	void           SetKeyBlockId(unsigned short theId);

	// NeedKey flag access
	bool GetNeedKey() const;
	void SetNeedKey(bool theFlag);

private:
	unsigned short mKeyBlockId;  // Id of AuthServ pub key block used to encrypt
	bool           mNeedKey;     // Flag, client requesting to pub/priv key
};


// TMsgAuth1ChallengeHL - Challenge the client's validity
//   Contains challenge seed encrypted with session key.  Session key was specified
//	 in the TMsgAuth1LoginRequestHL message.
class TMsgAuth1ChallengeHL : public TMsgAuthRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1ChallengeHL();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1ChallengeHL(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1ChallengeHL(const TMsgAuth1ChallengeHL& theMsgR);

	// Destructor
	~TMsgAuth1ChallengeHL();

	// Assignment
	TMsgAuth1ChallengeHL& operator=(const TMsgAuth1ChallengeHL& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

private:
};


// TMsgAuth1ConfirmHL - Confirm the client's validity
//   Contains confirm seed encrypted with session key.  Session key was specified
//	 in the TMsgAuth1LoginRequestHL message.
class TMsgAuth1ConfirmHL : public TMsgAuthRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1ConfirmHL();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1ConfirmHL(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1ConfirmHL(const TMsgAuth1ConfirmHL& theMsgR);

	// Destructor
	~TMsgAuth1ConfirmHL();

	// Assignment
	TMsgAuth1ConfirmHL& operator=(const TMsgAuth1ConfirmHL& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

private:
};

// TMsgAuth1RefreshHL - request extension
//   Contains Refresh Data encrypted with session key. The session key
//   matches the key sent in the Auth1LoginRequestHL message.
//   RawBuf is expected to contain:
//	   Random Pad       (6 bytes)  Must be different for each Refresh request.
//	   User ID          (4 bytes)  Must match key block Id in message

class TMsgAuth1RefreshHL : public TMsgAuthLastRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1RefreshHL();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1RefreshHL(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1RefreshHL(const TMsgAuth1RefreshHL& theMsgR);

	// Destructor
	~TMsgAuth1RefreshHL();

	// Assignment
	TMsgAuth1RefreshHL& operator=(const TMsgAuth1RefreshHL& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	// User ID access
	unsigned long GetUserId() const;
	void          SetUserId(unsigned long theId);

private:
	unsigned long mUserId;
};


// TMsgAuth1CheckHLKey - 
//   Key Block ID        (2 bytes)  Must match key block Id in encrypted block
//   Session Block Length(2 bytes)
//   Session Key Block   (variable) Session key Encrypted with AuthPublicKey
//   Data Block          (variable) encrypted with Session Key
//   Before encryption, Data Block contains:
//     KeyBlockID       (2 bytes) Must match key block Id in message
//     NeedKeyFlg       (1 byte )
//     CreateAcctFlg    (1 byte )

//     UserName len     (2 bytes)
//     UserName         (variable)
//     CommunityNameLen (2 bytes)
//     CommunityName    (variable)
//     NicknameKeyLen   (2 bytes)
//     NicknameKey      (variable)
//     Password len     (2 bytes)
//     Password         (var bytes == Password Len)
//     NewPassword len  (2 Bytes)
//     NewPassword      (var bytes == newPassword Len) (May be 0)

//     // LoginSessionID is typically a CD key, but could be something else.
//     // The AuthServer only allows one login session per key. It may also
//     // do validation checks on the key.
//     CDKey len (2 bytes)
//     CDKey     (var bytes == LoginSessionKeyLen) - typically a CD Key


//   Login data is encrypted with the and stored in the
//   raw buffer from the TMsgAuthRawBufferBase class.
class TMsgAuth1CheckHLKey : public TMsgAuth1LoginBase2
{
public:

	// Default ctor
	TMsgAuth1CheckHLKey();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1CheckHLKey(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1CheckHLKey(const TMsgAuth1CheckHLKey& theMsgR);

	// Destructor
	~TMsgAuth1CheckHLKey();

	// Assignment
	TMsgAuth1CheckHLKey& operator=(const TMsgAuth1CheckHLKey& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	bool GetNeedKeyFlg(void) { return mNeedKeyFlg; }
	bool GetCreateAcctFlg(void) { return mCreateAcctFlg; }
	const std::wstring& GetUserName(void) { return mUserName; }
	const std::wstring& GetCommunityName(void) { return mCommunityName; }
	const std::wstring& GetNicknameKey(void) { return mNicknameKey; }
	const std::wstring& GetPassword(void) { return mPassword; }
	const std::wstring& GetNewPassword(void) { return mNewPassword; }
	const WONCommon::RawBuffer& GetCDKey(void) { return mCDKey; }

	void SetNeedKeyFlg(bool theNeedKeyFlg) { mNeedKeyFlg = theNeedKeyFlg; }
	void SetCreateAcctFlg(bool theCreateAcctFlg) { mCreateAcctFlg = theCreateAcctFlg; }
	void SetUserName(const std::wstring &theUserName) { mUserName = theUserName; }
	void SetCommunityName(const std::wstring &theCommunityName) { mCommunityName = theCommunityName; }
	void SetNicknameKey(const std::wstring &theNicknameKey) { mNicknameKey = theNicknameKey; }
	void SetPassword(const std::wstring &thePassword) { mPassword = thePassword; }
	void SetNewPassword(const std::wstring &theNewPassword) { mNewPassword = theNewPassword; }
	void SetCDKey(const unsigned char *theKey, unsigned short theKeyLen) { mCDKey.assign(theKey,theKeyLen); }

	bool BuildBuffer(const WONAuth::Auth1PublicKeyBlock &thePubKeyBlock, const WONCrypt::BFSymmetricKey &theSessionKey);
	bool ExtractBuffer(const WONCrypt::EGPrivateKey &thePrivateKey, WONCrypt::BFSymmetricKey *theSessionKey);

private:
	bool mNeedKeyFlg;
	bool mCreateAcctFlg;
	std::wstring mUserName;
	std::wstring mCommunityName;
	std::wstring mNicknameKey;
	std::wstring mPassword;
	std::wstring mNewPassword;
	WONCommon::RawBuffer mCDKey;

	unsigned char *mKeyBuf;
	unsigned char *mDataBuf;
};


// Inlines
inline TRawMsg*
TMsgAuth1LoginRequestHL::Duplicate(void) const
{ return new TMsgAuth1LoginRequestHL(*this); }

inline unsigned short
TMsgAuth1LoginRequestHL::GetKeyBlockId() const
{ return mKeyBlockId; }

inline void
TMsgAuth1LoginRequestHL::SetKeyBlockId(unsigned short theId)
{ mKeyBlockId = theId; }

inline bool
TMsgAuth1LoginRequestHL::GetNeedKey() const
{ return mNeedKey; }

inline void
TMsgAuth1LoginRequestHL::SetNeedKey(bool theFlag)
{ mNeedKey = theFlag; }

inline TRawMsg*
TMsgAuth1ChallengeHL::Duplicate(void) const
{ return new TMsgAuth1ChallengeHL(*this); }

inline TRawMsg*
TMsgAuth1ConfirmHL::Duplicate(void) const
{ return new TMsgAuth1ConfirmHL(*this); }

inline TRawMsg*
TMsgAuth1RefreshHL::Duplicate(void) const
{ return new TMsgAuth1RefreshHL(*this); }

inline unsigned long
TMsgAuth1RefreshHL::GetUserId() const
{ return mUserId; }

inline void
TMsgAuth1RefreshHL::SetUserId(unsigned long theId)
{ mUserId = theId; }

inline TRawMsg*
TMsgAuth1CheckHLKey::Duplicate(void) const
{ return new TMsgAuth1CheckHLKey(*this); }

};  // Namespace WONMsg

#endif