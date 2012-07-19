#ifndef _SYMMETRICKEY_H
#define _SYMMETRICKEY_H

// SymmetricKey

// Class that encapsulates a symmectric key.  Length of key in bytes can be
// defined on creation.  Key can be used to encrypt and decrypt binary blocks
// of arbitrary size.

// SymmetricKey is an abstract base class.  Several methods are pure virtual and must
// be overridden.

// NOTE: Source for the virtual methods of SymmetricKey are in the CryptKeyBase.cpp
// module.  (These methods could be inline if they were'nt virtual.)


#include "CryptKeyBase.h"

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class CryptException;

class SymmetricKey : public CryptKeyBase
{
public:
	// Default constructor - makes an uncreated SymmetricKey.  Create method must
	// be called to make a valid key
	SymmetricKey();

	// Copy constructor
	SymmetricKey(const SymmetricKey& theKey);

	// Destructor
	virtual ~SymmetricKey(void);

	// Operators
	SymmetricKey& operator=(const SymmetricKey& theKey);

	// Create key - from existing key. Just calls base class.
	virtual void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Create key - new key of specified length.  Specifying theLen as 0 will cause
	// the default key length to be used.
	// Pure virtual, MUST be overridden
	virtual void Create(unsigned short theLen=0) throw(CryptException) = 0;

	// Encrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	// Pure virtual, MUST be overridden
	virtual CryptReturn Encrypt(const void* theMsgP, unsigned long theLen) const = 0;

	// Decrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).  Will
	// return a (NULL, 0) pair of decryption fails.
	// Pure virtual, MUST be overridden
	virtual CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen) const = 0;

protected:

private:
};


// Inlines
inline
SymmetricKey::SymmetricKey() :
	CryptKeyBase()
{}

inline
SymmetricKey::SymmetricKey(const SymmetricKey& theKeyR) :
	CryptKeyBase(theKeyR)
{}

inline
SymmetricKey::~SymmetricKey(void)
{}

inline SymmetricKey&
SymmetricKey::operator=(const SymmetricKey& theKeyR)
{ CryptKeyBase::operator=(theKeyR);  return *this; }

};  //namespace WONCrypt

#endif