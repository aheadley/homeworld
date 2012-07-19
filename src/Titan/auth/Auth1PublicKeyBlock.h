#ifndef _Auth1PublicKeyBlock_H
#define _Auth1PublicKeyBlock_H

// Auth1PublicKeyBlock

// WON AuthServer Public Key Block for Auth Family 1.  Encapsulates a list of one
// or more ElGamal public keys.

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.


#include "LIST"
#include <time.h>
#include "../crypt/EGPublicKey.h"
#include "AuthPublicKeyBlockBase.h"

// In the WONAuth namespace
namespace WONAuth {


class Auth1PublicKeyBlock : public AuthPublicKeyBlockBase
{
public:
    // Types
    typedef std::list<WONCrypt::EGPublicKey> PublicKeyList;

    // Default constructor - may provide block ID (for base class)
    explicit Auth1PublicKeyBlock(unsigned short theBlockId=0);

    // Construct from raw representation (calls Unpack())
    Auth1PublicKeyBlock(const unsigned char* theRawP, unsigned short theLen);

    // Copy Constructor
    Auth1PublicKeyBlock(const Auth1PublicKeyBlock& theBlockR);

    // Destructor
    ~Auth1PublicKeyBlock();

    // Operators
    Auth1PublicKeyBlock& operator=(const Auth1PublicKeyBlock& theBlockR);

    // Compare (overridden from base class)
    int Compare(const AuthFamilyBuffer& theBufR) const;

    // Fetch key block family
    unsigned short GetFamily() const;

    // Public Key set access
    const PublicKeyList& KeyList() const;
    PublicKeyList&       KeyList();

    // Verify signature on an AuthFamily buffer.  Tries each public key in block
    // in turn til one succeeds or all fail.  Returns true if buffer signature was
    // verified, false if not.
    bool VerifyFamilyBuffer(const AuthFamilyBuffer& theBufR) const;

    // Verify signature on a raw buffer.  Tries each public key in block in turn
    // til one succeeds or all fail.  Returns true if buffer signature was
    // verified, false if not.
    bool VerifyRawBuffer(const unsigned char* theSigP, unsigned long theSigLen,
                         const unsigned char* theMsgP, unsigned long theMsgLen) const;

    // Encrypts a raw buffer with first key from the key block.  Returns encrypted block
    WONCrypt::EGPublicKey::CryptReturn EncryptRawBuffer(
        const unsigned char* theBufP, unsigned long theLen
    ) const;

    // Dump to stream
    void Dump(std::ostream& os) const;

private:
    PublicKeyList mKeyList;

    // Compute size of buffer needed form pack and unpack operations.
    WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

    // Pack local members into base raw buffer
    bool PackData();

    // Unpack local members from base raw buffer
    bool UnpackData();
};


// Inlines
inline const Auth1PublicKeyBlock::PublicKeyList&
Auth1PublicKeyBlock::KeyList() const
{ return mKeyList; }

inline Auth1PublicKeyBlock::PublicKeyList&
Auth1PublicKeyBlock::KeyList()
{ return mKeyList; }


};  // Namespace WONAuth

#endif