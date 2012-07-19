#ifndef _TMsgAuth1Login2_H
#define _TMsgAuth1Login2_H

// TMsgAuth1LoginRequest2.h

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

// Forwards from WONSocket
namespace WONMsg {

// TMsgAuth1LoginRequest2 - begin login for HomeWorld user
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
//     NicknameLen      (2 bytes)
//     Nickname         (variable)

//   Login data is encrypted with the and stored in the
//   raw buffer from the TMsgAuthRawBufferBase class.
class TMsgAuth1LoginRequest2 : public TMsgAuth1LoginBase2
{
public:

    // Default ctor
    TMsgAuth1LoginRequest2();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1LoginRequest2(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1LoginRequest2(const TMsgAuth1LoginRequest2& theMsgR);

    // Destructor
    ~TMsgAuth1LoginRequest2();

    // Assignment
    TMsgAuth1LoginRequest2& operator=(const TMsgAuth1LoginRequest2& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

private:

};

inline TRawMsg*
TMsgAuth1LoginRequest2::Duplicate(void) const
{ return new TMsgAuth1LoginRequest2(*this); }

};  // Namespace WONMsg

#endif