#ifndef _Auth1Certificate_H
#define _Auth1Certificate_H

// Auth1Certificate

// WON Authentication Certificate for Auth Family 1.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.


#include <time.h>
#include "../crypt/EGPublicKey.h"
#include "AuthCertificateBase.h"

// In the WONAuth namespace
namespace WONAuth {


class Auth1Certificate : public AuthCertificateBase
{
public:
	// Default constructor - may provide user info
	Auth1Certificate(unsigned long theUserId=0, unsigned long theCommunityId=0,
	                 unsigned short theTrustLevel=0);

	// Construct from raw representation (calls Unpack())
	Auth1Certificate(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	Auth1Certificate(const Auth1Certificate& theCertR);

	// Destructor
	~Auth1Certificate();

	// Operators
	Auth1Certificate& operator=(const Auth1Certificate& theCertR);

	// Compare (overridden from base class)
	int Compare(const AuthFamilyBuffer& theBufR) const;

	// Fetch certificate family
	unsigned short GetFamily() const;

	// User information access
	const unsigned long  GetUserId() const;
	const unsigned long  GetCommunityId() const;
	const unsigned short GetTrustLevel() const;

	// Public Key access
	const WONCrypt::EGPublicKey& GetPubKey() const;

	// Member update - will invalidate certificate until pack is called again
	void SetUserId(unsigned long theId);
	void SetCommunityId(unsigned long theId);
	void SetTrustLevel(unsigned short theLevel);
	void SetPublicKey(const WONCrypt::EGPublicKey& theKeyR);

	// Dump to stream
	void Dump(std::ostream& os) const;

private:
	unsigned long         mUserId;       // WON User ID (WONUserSeq)
	unsigned long         mCommunityId;  // User's community ID
	unsigned short        mTrustLevel;   // User's trust level
	WONCrypt::EGPublicKey mPubKey;       // Public key

	// Compute size of buffer needed form pack and unpack operations.
	WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack local members into base raw buffer
	bool PackData();

	// Unpack local members from base raw buffer
	bool UnpackData();
};


// Inlines
inline const unsigned long
Auth1Certificate::GetUserId() const
{ return mUserId; }

inline const unsigned long
Auth1Certificate::GetCommunityId() const
{ return mCommunityId; }

inline const unsigned short
Auth1Certificate::GetTrustLevel() const
{ return mTrustLevel; }

inline const WONCrypt::EGPublicKey&
Auth1Certificate::GetPubKey() const
{ return mPubKey; }

inline void
Auth1Certificate::SetUserId(unsigned long theId)
{ Invalidate();  mUserId = theId; }

inline void
Auth1Certificate::SetCommunityId(unsigned long theId)
{ Invalidate();  mCommunityId = theId; }

inline void
Auth1Certificate::SetTrustLevel(unsigned short theLevel)
{ Invalidate();  mTrustLevel = theLevel; }

inline void
Auth1Certificate::SetPublicKey(const WONCrypt::EGPublicKey& theKeyR)
{ Invalidate();  mPubKey = theKeyR; }

};  // Namespace WONAuth

#endif