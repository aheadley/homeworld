#ifndef _AuthPublicKeyBlock1_H
#define _AuthPublicKeyBlock1_H

// AuthPublicKeyBlock1

// WON AuthServer Public Key Block for Auth Family 1.  Encapsulates a list of one
// or more ElGamal public keys.

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.


#include "LIST"
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthPublicKeyBlockBase.h"

// In the WONAuth namespace
namespace WONAuth {


class AuthPublicKeyBlock1 : public AuthPublicKeyBlockBase
{
public:
    // Types
    typedef std::list<WONCrypt::EGPublicKey> PublicKeyList;

    // Default constructor - may provide block ID (for base class)
    explicit AuthPublicKeyBlock1(unsigned short theBlockId=0);

    // Construct from raw representation (calls Unpack())
    AuthPublicKeyBlock1(const unsigned char* theRawP, unsigned short theLen);

    // Copy Constructor
    AuthPublicKeyBlock1(const AuthPublicKeyBlock1& theBlockR);

    // Destructor
    ~AuthPublicKeyBlock1();

    // Operators
    AuthPublicKeyBlock1& operator=(const AuthPublicKeyBlock1& theBlockR);

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
inline const AuthPublicKeyBlock1::PublicKeyList&
AuthPublicKeyBlock1::KeyList() const
{ return mKeyList; }

inline AuthPublicKeyBlock1::PublicKeyList&
AuthPublicKeyBlock1::KeyList()
{ return mKeyList; }


};  // Namespace WONAuth

#endif