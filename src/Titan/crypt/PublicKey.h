#ifndef _PUBLICKEY_H
#define _PUBLICKEY_H

// PublicKey

// Base class that encapsulates a Public key used for encryption and signature verify.
// Public keys cannot be created by this class.  Public/Private keys must be generated
// in pairs.  Use instances of the PrivateKey class to generate the corresponding public
// key.  PublicKey contains methods to encrypt blocks and verify signatures.

// PublicKey is an abstract base class.  Several methods are pure virtual and must
// be overridden.

// NOTE: Source for the virtual methods of PublicKey are in the CryptKeyBase.cpp
// module.  (These methods could be inline if they were'nt virtual.)


#include <ostream>
#include "CryptKeyBase.h"

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class CryptException;

class PublicKey : public CryptKeyBase
{
public:
	// Default constructor - makes an uncreated PublicKey.  Create method must
	// be called to make a valid key
	PublicKey();

	// Copy constructor
	PublicKey(const PublicKey& theKey);

	// Destructor
	virtual ~PublicKey();

	// Operators
	PublicKey& operator=(const PublicKey& theKey);

	// Create key - from existing key. Just calls base class.
	virtual void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Create key - new key of specified length.  This method is not allowed on
	// PublicKey and will throw an exception if called.
	virtual void Create(unsigned short theLen=0) throw(CryptException);

	// Encrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	// Pure virtual, MUST be overridden
	virtual CryptReturn Encrypt(const void* theMsgP, unsigned long theLen) const = 0;

	// Verify a signature block.  Returns true if signature is valid, false if not.
	// Pure virtual, MUST be overridden
	virtual bool Verify(const unsigned char* theSigP, unsigned long theSigLen,
	                    const void* theMsgP, unsigned long theMsgLen) const = 0;

protected:

private:
};


// Inlines
inline
PublicKey::PublicKey() : CryptKeyBase()
{}

inline
PublicKey::PublicKey(const PublicKey& theKeyR) : CryptKeyBase(theKeyR)
{}

inline
PublicKey::~PublicKey()
{}

inline PublicKey&
PublicKey::operator=(const PublicKey& theKeyR)
{ CryptKeyBase::operator=(theKeyR);  return *this; }


};  //namespace WONCrypt


#endif