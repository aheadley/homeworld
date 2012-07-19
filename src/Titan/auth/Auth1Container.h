#ifndef _Auth1Container_H
#define _Auth1Container_H

// Auth1Container

// Container for the objects that are used for authenticated communication between
// servers/clients.  This includes: the Auth Verifier Public Key, the AuthServer
// PublicKey Block, the Auth Certificate, and the EGPrivateKey attached to the
// certificate.


#include "common/CriticalSection.h"
#include "Auth1PublicKeyBlock.h"
#include "Auth1Certificate.h"
#include "crypt/EGPublicKey.h"
#include "crypt/EGPrivateKey.h"


// In the WONAuth namespace
namespace WONAuth {


struct Auth1Container
{
	WONCrypt::EGPublicKey  mAuthVerifier;  // AuthServer Verifier Key
	Auth1PublicKeyBlock    mPubKeyBlock;   // AuthServer PublicKey Block
	Auth1Certificate       mCert;          // Server's Auth Certificate
	WONCrypt::EGPrivateKey mPrivKey;       // Server's Private Key
	long                   mAuthDelta;     // Delta (in seconds) from Auth Server time

	// Critical section for access control
	mutable WONCommon::CriticalSection mCrit;

	// Constructor
	Auth1Container();

	// Utility methods for common operations (thread safe)
	bool IsValid() const;
	bool IsExpired(long theDelta=0) const;

private:
	// Much too expensive to copy/assign this object
	Auth1Container(const Auth1Container&);
	Auth1Container& operator=(const Auth1Container&);
};

// Inlines
inline
Auth1Container::Auth1Container() :
	mAuthVerifier(), mPubKeyBlock(), mCert(), mPrivKey(), mAuthDelta(0), mCrit()
{}

inline bool
Auth1Container::IsValid() const
{ WONCommon::AutoCrit aCrit(mCrit);  return mCert.IsValid(); }

inline bool
Auth1Container::IsExpired(long theDelta) const
{ WONCommon::AutoCrit aCrit(mCrit);  return mCert.IsExpired(mAuthDelta + theDelta); }

};  // Namespace WONAuth

#endif