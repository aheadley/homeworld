#ifndef _PRIVATEKEY_H
#define _PRIVATEKEY_H

// PrivateKey

// Class that encapsulates a Private key used for decryption and signatures.  Length
// of each portion of the key in bytes can be defined on creation.  PrivateKey
// contains methods to decrypt blocks and sign blocks.

// PrivateKey is an abstract base class.  Several methods are pure virtual and must
// be overridden.

// NOTE: Source for the virtual methods of PrivateKey are in the CryptKeyBase.cpp
// module.  (These methods could be inline if they were'nt virtual.)


#include <ostream>
#include "PublicKey.h"

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class CryptException;

class PrivateKey : public CryptKeyBase
{
public:
	// Default constructor - makes an uncreated PrivateKey.  Create method must
	// be called to make a valid key
	PrivateKey();

	// Copy constructor
	PrivateKey(const PrivateKey& theKey);

	// Destructor
	virtual ~PrivateKey(void);

	// Operators
	PrivateKey& operator=(const PrivateKey& theKeyR);

	// Create key - from existing key. Just calls base class.
	virtual void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Create key - new key of specified length.
	// Pure virtual, MUST be overridden
	virtual void Create(unsigned short theLen=0) throw(CryptException) = 0;

	// Fetch the PublicKey.  Returns a const ref so derived clases may return
	// a derived PublicKey type without slicing.
	// Pure virtual, MUST be overridden
	virtual const PublicKey& GetPublicKey() const throw(CryptException) = 0;

	// Decrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).  Will
	// return a (NULL, 0) pair if decryption fails.
	// Pure virtual, MUST be overridden
	virtual CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen) const = 0;

	// Sign a block.  Returns allocated signature block and length of signature block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	// Pure virtual, MUST be overridden
	virtual CryptReturn Sign(const void* theMsgP, unsigned long theLen) const = 0;

protected:

private:
};


// Inlines
inline
PrivateKey::PrivateKey() : CryptKeyBase()
{}

inline
PrivateKey::PrivateKey(const PrivateKey& theKeyR) : CryptKeyBase(theKeyR)
{}

inline
PrivateKey::~PrivateKey()
{}

inline PrivateKey&
PrivateKey::operator=(const PrivateKey& theKeyR)
{ CryptKeyBase::operator=(theKeyR);  return *this; }

};  //namespace WONCrypt

#endif