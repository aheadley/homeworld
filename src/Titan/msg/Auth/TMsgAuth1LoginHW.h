#ifndef _TMsgAuth1LoginHW_H
#define _TMsgAuth1LoginHW_H

// TMsgAuth1LoginRequestHW.h

// Homeworld specific Auth messages

// Auth1LoginRequest begins login for an existing user and is sent from the Client to
// the AuthServer.

// Auth1LoginReply is sent in response to both Auth1LoginRequest and


#include "STRING"
#include "common/won.h"
#include "auth/Auth1PublicKeyBlock.h"
#include "crypt/BFSymmetricKey.h"
#include "crypt/EGPrivateKey.h"
#include "msg/BadMsgException.h"
#include "TMsgAuth1LoginBase2.h"
#include "TMsgAuthRawBufferBase.h"

// Forwards from WONSocket
namespace WONMsg {

// TMsgAuth1LoginRequestHW - begin login for HomeWorld user
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

//     // This is the ticket that allows re-login to the AuthServer
//     // with the same CDKey. The authServer will change this and send the
//     // new one to the client upon sucessful login.
//     LoginKey len  (2 bytes)
//     LoginKey      (var bytes == LoginSession ID len)

//   Login data is encrypted with the and stored in the
//   raw buffer from the TMsgAuthRawBufferBase class.
class TMsgAuth1LoginRequestHW : public TMsgAuth1LoginBase2
{
public:

    // Default ctor
    TMsgAuth1LoginRequestHW();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1LoginRequestHW(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1LoginRequestHW(const TMsgAuth1LoginRequestHW& theMsgR);

    // Destructor
    ~TMsgAuth1LoginRequestHW();

    // Assignment
    TMsgAuth1LoginRequestHW& operator=(const TMsgAuth1LoginRequestHW& theMsgR);

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
    const WONCommon::RawBuffer& GetLoginKey(void) { return mLoginKey; }

    void SetNeedKeyFlg(bool theNeedKeyFlg) { mNeedKeyFlg = theNeedKeyFlg; }
    void SetCreateAcctFlg(bool theCreateAcctFlg) { mCreateAcctFlg = theCreateAcctFlg; }
    void SetUserName(const std::wstring &theUserName) { mUserName = theUserName; }
    void SetCommunityName(const std::wstring &theCommunityName) { mCommunityName = theCommunityName; }
    void SetNicknameKey(const std::wstring &theNicknameKey) { mNicknameKey = theNicknameKey; }
    void SetPassword(const std::wstring &thePassword) { mPassword = thePassword; }
    void SetNewPassword(const std::wstring &theNewPassword) { mNewPassword = theNewPassword; }
    void SetCDKey(const unsigned char *theKey, unsigned short theKeyLen) { mCDKey.assign(theKey,theKeyLen); }
    void SetLoginKey(const unsigned char *theKey, unsigned short theKeyLen) { mLoginKey.assign(theKey,theKeyLen); }

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
    WONCommon::RawBuffer mLoginKey;

    unsigned char *mKeyBuf;
    unsigned char *mDataBuf;

};

// TMsgAuth1ChallengeHW - Challenge the client's validity
//   Contains challenge seed encrypted with session key.  Session key was specified
//   in the TMsgAuth1LoginRequestHW message.
class TMsgAuth1ChallengeHW : public TMsgAuthRawBufferBase
{
public:
    // Default ctor
    TMsgAuth1ChallengeHW();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1ChallengeHW(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1ChallengeHW(const TMsgAuth1ChallengeHW& theMsgR);

    // Destructor
    ~TMsgAuth1ChallengeHW();

    // Assignment
    TMsgAuth1ChallengeHW& operator=(const TMsgAuth1ChallengeHW& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

private:
};


// TMsgAuth1ConfirmHW - Confirm the client's validity
//   Contains confirm seed encrypted with session key.  Session key was specified
//   in the TMsgAuth1LoginRequestHW message.
class TMsgAuth1ConfirmHW : public TMsgAuthRawBufferBase
{
public:
    // Default ctor
    TMsgAuth1ConfirmHW();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1ConfirmHW(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1ConfirmHW(const TMsgAuth1ConfirmHW& theMsgR);

    // Destructor
    ~TMsgAuth1ConfirmHW();

    // Assignment
    TMsgAuth1ConfirmHW& operator=(const TMsgAuth1ConfirmHW& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

private:
};

// TMsgAuth1RefreshHW - HomeWorld user refresh
//   Key Block ID        (2 bytes)
//   Session Block Length(2 bytes)
//   Session Key Block   (variable)
//
//  Before encryption, Session Key Block contains 2 byte random
//  pad followed by Login Secret.
//
// AuthServer replies with TMsgAuth1LoginReply
//
class TMsgAuth1RefreshHW : public TMsgAuth1LoginBase2
{
public:

    // Default ctor
    TMsgAuth1RefreshHW();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1RefreshHW(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1RefreshHW(const TMsgAuth1RefreshHW& theMsgR);

    // Destructor
    ~TMsgAuth1RefreshHW();

    // Assignment
    TMsgAuth1RefreshHW& operator=(const TMsgAuth1RefreshHW& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

private:

};

inline TRawMsg*
TMsgAuth1LoginRequestHW::Duplicate(void) const
{ return new TMsgAuth1LoginRequestHW(*this); }

inline TRawMsg*
TMsgAuth1ChallengeHW::Duplicate(void) const
{ return new TMsgAuth1ChallengeHW(*this); }

inline TRawMsg*
TMsgAuth1ConfirmHW::Duplicate(void) const
{ return new TMsgAuth1ConfirmHW(*this); }

inline TRawMsg*
TMsgAuth1RefreshHW::Duplicate(void) const
{ return new TMsgAuth1RefreshHW(*this); }

};  // Namespace WONMsg

#endif