#ifndef _TMsgAuth1PeerToPeer_H
#define _TMsgAuth1PeerToPeer_H

// TMsgAuth1PeerToPeer.h

// AuthServer messages implementing the Auth1 Family peer-to-peer protocol.  These
// messages support both the persistent and lightweight Authentication protocols
// between clients/servers

// This header/source implements messages:
//  Auth1Request
//  Auth1Challenge1
//  Auth1Challenge2
//  Auth1Complete

// Auth1Request begins authentication bwteen two clients/servers and is sent from
// Client A to Client B.

// Auth1Challenge1 is sent in response to Auth1Request from Client B to Client A.

// Auth1Challenge2 is sent in response to Auth1Challenge1 from Client A to Client B.

// Auth1Complete is sent in response to Auth1Challenge2 from Client B to Client A and
// completes the authentication.


#include "LIST"
#include "auth/CryptFlags.h"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "TMsgAuthRawBufferBase.h"

// In WONMsg namespace
namespace WONMsg {


// Use these types in the messages
using WONAuth::AuthenticationMode;
using WONAuth::EncryptionMode;
using WONAuth::EncryptionFlags;

// Auth1Request - begin authentication
// Contains identification of client A (its certificate) and mode information for
// establishing the authentication.
// Raw buffer from base class is used to store client A certificate.
class TMsgAuth1Request : public TMsgAuthRawBufferBase
{
public:
    // Default ctor
    TMsgAuth1Request();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1Request(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1Request(const TMsgAuth1Request& theMsgR);

    // Destructor
    ~TMsgAuth1Request();

    // Assignment
    TMsgAuth1Request& operator=(const TMsgAuth1Request& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

    // AuthMode access
    AuthenticationMode GetAuthMode() const;
    void               SetAuthMode(AuthenticationMode theMode);

    // EncryptMode access
    EncryptionMode GetEncryptMode() const;
    void           SetEncryptMode(EncryptionMode theMode);

    // Encrypt flags access
    unsigned short GetEncryptFlags  () const;
    void           SetEncryptFlags  (unsigned short theFlags);
    void           AddEncryptFlag   (unsigned short theFlag);
    void           ClearEncryptFlags();

private:
    AuthenticationMode mAuthMode;      // Authentication mode
    EncryptionMode     mEncryptMode;   // Encryption mode
    unsigned short     mEncryptFlags;  // Encryption flags (bit field)
};


// TMsgAuth1Challenge1 - Challenge client A's validity and provide Client B
// certificate.  Contains Secret B encrypted with public key of client A and
// client B certificate.
// Raw buffer from base class is used to store client B certificate.  SecretB buffer
// must be encrypted with Client A's public key.  Structure SecretB block before
// encrypt as:
//      Length of Secret B (2 bytes)
//      Secret B
class TMsgAuth1Challenge1 : public TMsgAuthRawBufferBase
{
public:
    // Default ctor
    TMsgAuth1Challenge1();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1Challenge1(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1Challenge1(const TMsgAuth1Challenge1& theMsgR);

    // Destructor
    ~TMsgAuth1Challenge1();

    // Assignment
    TMsgAuth1Challenge1& operator=(const TMsgAuth1Challenge1& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

    // Secret B access
    const unsigned char* GetSecretB() const;
    unsigned short       GetSecretBLen() const;
    void SetSecretB(const unsigned char* theDataP, unsigned short theLen, bool copyData=false);
    void ForceSecretBOwn();

private:
    // Client B Secret
    const unsigned char* mSecretBP;    // Secret data
    unsigned short       mSecretBLen;  // Length of secret data
    WONCommon::RawBuffer mSecretBBuf;  // Buffer for secret data when owned
};


// TMsgAuth1Challenge2 - Challenge client B's validity and prove Client A's validity.
// Contains Secret B  and Secret A encrypted with public key of client B.
// Raw buffer from base class is used to store encrypted block of both secrets.  This
// block must be encrypted with Client B's public key.  Structure block before encrypt
// as:
//      Length of secret B (2 bytes)
//      Secret B
//      Secret A
class TMsgAuth1Challenge2 : public TMsgAuthRawBufferBase
{
public:
    // Default ctor
    TMsgAuth1Challenge2();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1Challenge2(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1Challenge2(const TMsgAuth1Challenge2& theMsgR);

    // Destructor
    ~TMsgAuth1Challenge2();

    // Assignment
    TMsgAuth1Challenge2& operator=(const TMsgAuth1Challenge2& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

private:
};


// TMsgAuth1Complete - Complete authentication.  Contains a status, optionally
// a confirmation of Client B's validity, and optionally, a session ID if needed.
// Raw buffer from base class is used to store encrypted block of Client A's secret.
// This block must be encrypted with Client A's public key.  Structure before encrypt
// as:
//      Len of secret (2 bytes)
//      secretA
class TMsgAuth1Complete : public TMsgAuthRawBufferBase
{
public:
    // Types
    typedef std::list<std::string> ErrorList;

    // Default ctor
    TMsgAuth1Complete();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1Complete(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1Complete(const TMsgAuth1Complete& theMsgR);

    // Destructor
    ~TMsgAuth1Complete();

    // Assignment
    TMsgAuth1Complete& operator=(const TMsgAuth1Complete& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

    // Status access
    ServerStatus GetStatus() const;
    void         SetStatus(ServerStatus theStatus);

    // Session Id access, 0 implies no session Id
    unsigned short GetSessionId() const;
    void           SetSessionId(unsigned short theId);

    // Error List access, may be empty
    const ErrorList& ErrList() const;
    ErrorList&       ErrList();

private:
    ServerStatus   mStatus;     // Status of the request
    unsigned short mSessionId;  // Optional session ID
    ErrorList      mErrList;    // List of extended error info (may be empty)
};


// Inlines
inline TRawMsg*
TMsgAuth1Request::Duplicate(void) const
{ return new TMsgAuth1Request(*this); }

inline WONMsg::AuthenticationMode
TMsgAuth1Request::GetAuthMode() const
{ return mAuthMode; }

inline void
TMsgAuth1Request::SetAuthMode(WONMsg::AuthenticationMode theMode)
{ mAuthMode = theMode; }

inline WONMsg::EncryptionMode
TMsgAuth1Request::GetEncryptMode() const
{ return mEncryptMode; }

inline void
TMsgAuth1Request::SetEncryptMode(WONMsg::EncryptionMode theMode)
{ mEncryptMode = theMode; }

inline unsigned short
TMsgAuth1Request::GetEncryptFlags() const
{ return mEncryptFlags; }

inline void
TMsgAuth1Request::SetEncryptFlags(unsigned short theFlags)
{ mEncryptFlags = theFlags; }

inline void
TMsgAuth1Request::AddEncryptFlag(unsigned short theFlag)
{ mEncryptFlags |= theFlag; }

inline void
TMsgAuth1Request::ClearEncryptFlags()
{ mEncryptFlags = WONAuth::EFLAGS_NONE; }

inline TRawMsg*
TMsgAuth1Challenge1::Duplicate(void) const
{ return new TMsgAuth1Challenge1(*this); }

inline const unsigned char*
TMsgAuth1Challenge1::GetSecretB() const
{ return mSecretBP; }

inline unsigned short
TMsgAuth1Challenge1::GetSecretBLen() const
{ return mSecretBLen; }

inline void
TMsgAuth1Challenge1::ForceSecretBOwn()
{
    if ((mSecretBP) && (mSecretBP != mSecretBBuf.data()))
        {  mSecretBBuf.assign(mSecretBP, mSecretBLen);  mSecretBP = mSecretBBuf.data();  }
}

inline TRawMsg*
TMsgAuth1Challenge2::Duplicate(void) const
{ return new TMsgAuth1Challenge2(*this); }

inline TRawMsg*
TMsgAuth1Complete::Duplicate(void) const
{ return new TMsgAuth1Complete(*this); }

inline ServerStatus
TMsgAuth1Complete::GetStatus() const
{ return mStatus; }

inline void
TMsgAuth1Complete::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline unsigned short
TMsgAuth1Complete::GetSessionId() const
{ return mSessionId; }

inline void
TMsgAuth1Complete::SetSessionId(unsigned short theId)
{ mSessionId = theId; }

inline const TMsgAuth1Complete::ErrorList&
TMsgAuth1Complete::ErrList() const
{ return mErrList; }

inline TMsgAuth1Complete::ErrorList&
TMsgAuth1Complete::ErrList()
{ return mErrList; }

};  // Namespace WONMsg

#endif