#ifndef _TMsgAuth1LoginReply_H
#define _TMsgAuth1LoginReply_H

// TMsgAuth1LoginReply.h

// AuthServer login reply for many of the Auth1 login protocol. This is the final
// message is the login exchange that delivers the Auth Certificate or failure
// information.  It may optionally deliver other pieces of information as well which
// are described below.

// This message contains a status, followed by an option array of error strings,
// followed by an optional array of data encrypted with the session key.  Once
// decrypted, the array will be composed as follows:
//  1) 0 or 1 Auth Certificate
//  2) 0 or 1 Client PrivateKey
//  3) 0 or 1 Auth PublicKey Block
//  4) 0 or 1 Secret confirmation
//  5) 0 or 1 Nickname info

// All items if present will be in the above order.  Item (3) may be present in all
// cases.  The other items will only be present ofr successful statuses.

// If present, the secret confirmation contains:
//     2 byte random pad
//     Session key
// If the Session key in the message matches the session key sent in the Auth1Login
// message, the client knows that the data it received came from the auth server, and
// is not simply garbage.

// The Nickname Info contains:
//     1 byte flag
//     Nickname
// If the 1 byte flag is true, The NicknameKey in the Auth1LoginRequest was not found,
// and the Nickname is the default value.


#include "common/won.h"
#include "LIST"
#include "MAP"
#include "crypt/BFSymmetricKey.h"
#include "msg/TMessage.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"


// Forwards from WONSocket
namespace WONMsg {

class TMsgAuth1LoginReply : public TMessage
{
public:
    // Types
    enum EntryType {
        ALCertificate      = 1,  // Certificate
        ALClientPrivateKey = 2,  // Client's Private Key
        ALPublicKeyBlock   = 3,  // AuthServer Public Key Block
        ALSecretConfirm    = 4,  // Client secret confirmation
        ALNicknameInfo     = 5,  // Nickname information
    };

    typedef std::pair<const unsigned char*, unsigned short> RawBlock;
    typedef std::list<std::string> ErrorList;

    // Default ctor
    TMsgAuth1LoginReply(ServiceType theServType);

    // TMessage ctor - will throw if TMessage type is not of this type
    // Must provide session key for Unpack.
    TMsgAuth1LoginReply(const TMessage& theMsgR, WONCrypt::SymmetricKey* theKeyP,
                        bool copyKey=false);

    // Copy ctor
    TMsgAuth1LoginReply(const TMsgAuth1LoginReply& theMsgR);

    // Destructor
    ~TMsgAuth1LoginReply();

    // Assignment
    TMsgAuth1LoginReply& operator=(const TMsgAuth1LoginReply& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate() const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

    // Status access
    ServerStatus GetStatus() const;
    void         SetStatus(ServerStatus theStatus);

    // Session key access - Must set session key befor packing.
    const WONCrypt::SymmetricKey* GetSessKey() const;
    void SetSessKey(WONCrypt::SymmetricKey* theKeyP, bool copyKey=false);

    // Error List access
    const ErrorList& ErrList() const;
    ErrorList&       ErrList();

    // Block access
    const RawBlock& GetRawBlock(EntryType theType) const;// May be null

    // Update a raw block.  Setting copyBlock to false will cause the specified raw
    // pointer to be stored without copying its contents.  This will improve
    // performance, but raw pointer MUST NOT BE DEALLOCATED while in use by this class.
    void SetRawBlock(EntryType theType, const unsigned char* theBlockP,
                     unsigned short theLen, bool copyBlock=false);

    // Force copy of raw blocks if needed.
    void ForceOwn(EntryType theType);
    void ForceOwnAll();  // Do all blocks

private:
    // Types
    typedef std::map<EntryType, RawBlock> RawBlockMap;
    typedef std::map<EntryType, WONCommon::RawBuffer> BufferMap;

    // Members
    ServerStatus mStatus;   // Status of the request
    ErrorList    mErrList;  // List of extended error info (may be empty)
    RawBlockMap  mRawMap;   // Map of raw blocks
    BufferMap    mBufMap;   // Map of buffers for ownership of raw block data

    // Key for encrypt/decrypt as it's ownership indicator
    WONCrypt::SymmetricKey* mSessKeyP;
    bool                    mOwnKey;

    // Save the decrypted block from unpack to save copies
    WONCrypt::BFSymmetricKey::CryptReturn mDecrypt;

    // Get a ref to block of specified type.  Inits block to (NULL,0) if needed.
    RawBlock& GetBlockRef(EntryType theType);

    // Copy blocks from another instance
    void CopyBlocks(const TMsgAuth1LoginReply& theMsgR);

    // Pack/Unpack a block
    void PackBlock(WONCommon::RawBuffer& theBufR, EntryType theType, RawBlock& theBlockR);
    void PackClearBlock(EntryType theType, RawBlock& theBlockR);
    void UnpackBlock(EntryType theType, unsigned char*& theBufP, unsigned long& theLen);
    void UnpackClearBlock(EntryType theType);
    void UnpackErrString(unsigned char*& theBufP, unsigned long& theLen);

    void EncryptAndPack();
    void DecryptAndUnpack();
};


// Inlines
inline TRawMsg*
TMsgAuth1LoginReply::Duplicate(void) const
{ return new TMsgAuth1LoginReply(*this); }

inline ServerStatus
TMsgAuth1LoginReply::GetStatus() const
{ return mStatus; }

inline void
TMsgAuth1LoginReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const WONCrypt::SymmetricKey*
TMsgAuth1LoginReply::GetSessKey() const
{ return mSessKeyP; }

inline const TMsgAuth1LoginReply::ErrorList&
TMsgAuth1LoginReply::ErrList() const
{ return mErrList; }

inline TMsgAuth1LoginReply::ErrorList&
TMsgAuth1LoginReply::ErrList()
{ return mErrList; }

inline const TMsgAuth1LoginReply::RawBlock&
TMsgAuth1LoginReply::GetRawBlock(EntryType theType) const
{ return const_cast<TMsgAuth1LoginReply*>(this)->GetBlockRef(theType); }

};  // Namespace WONMsg

#endif