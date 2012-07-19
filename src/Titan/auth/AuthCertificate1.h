#ifndef _AuthCertificate1_H
#define _AuthCertificate1_H

// AuthCertificate1

// WON Authentication Certificate for Auth Family 1.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.


#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthCertificateBase.h"

// In the WONAuth namespace
namespace WONAuth {


class AuthCertificate1 : public AuthCertificateBase
{
public:
	// Default constructor - may provide user info
	AuthCertificate1(unsigned long theUserId=0, unsigned long theCommunityId=0,
	                 unsigned short theTrustLevel=0);

	// Construct from raw representation (calls Unpack())
	AuthCertificate1(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	AuthCertificate1(const AuthCertificate1& theCertR);

	// Destructor
	~AuthCertificate1();

	// Operators
	AuthCertificate1& operator=(const AuthCertificate1& theCertR);

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
AuthCertificate1::GetUserId() const
{ return mUserId; }

inline const unsigned long
AuthCertificate1::GetCommunityId() const
{ return mCommunityId; }

inline const unsigned short
AuthCertificate1::GetTrustLevel() const
{ return mTrustLevel; }

inline const WONCrypt::EGPublicKey&
AuthCertificate1::GetPubKey() const
{ return mPubKey; }

inline void
AuthCertificate1::SetUserId(unsigned long theId)
{ Invalidate();  mUserId = theId; }

inline void
AuthCertificate1::SetCommunityId(unsigned long theId)
{ Invalidate();  mCommunityId = theId; }

inline void
AuthCertificate1::SetTrustLevel(unsigned short theLevel)
{ Invalidate();  mTrustLevel = theLevel; }

inline void
AuthCertificate1::SetPublicKey(const WONCrypt::EGPublicKey& theKeyR)
{ Invalidate();  mPubKey = theKeyR; }

};  // Namespace WONAuth

#endif