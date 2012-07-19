#ifndef _BFSYMMETRICKEY_H
#define _BFSYMMETRICKEY_H

// BFSymmetricKey

// Class that encapsulates a Blowfish implementation of a symmectric key.  Length of
// key in bytes can be defined on creation.  Key can be used to encrypt and decrypt
// binary blocks of arbitrary size.


#include "SymmetricKey.h"

// From Crypto++
namespace CryptoPP {
	class BlowfishEncryption;
	class BlowfishDecryption;
};


// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class CryptException;

class BFSymmetricKey : public SymmetricKey
{
public:
	// Default constructor - makes an uncreated BFSymmetricKey.  Create method must
	// be called to make a valid key
	BFSymmetricKey();

	// Constructor - construct new key of specified length if theKeyP is NULL.
	// Otherwise copy existing key from buffer.
	explicit BFSymmetricKey(unsigned short theLen, const unsigned char* theKeyP=NULL) throw(CryptException);

	// Copy constructor
	BFSymmetricKey(const BFSymmetricKey& theKey);

	// Destructor
	~BFSymmetricKey(void);

	// Operators
	BFSymmetricKey& operator=(const BFSymmetricKey& theKey);

	// Create key - new key of specified length.  Specifying theLen as 0 will cause
	// the default key length to be used.
	void Create(unsigned short theLen=0) throw(CryptException);

	// Create key - from existing key.
	void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Encrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	CryptReturn Encrypt(const void* theMsgP, unsigned long theLen) const;

	// Decrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).  Will
	// return a (NULL, 0) pair if decryption fails.
	CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen) const;

private:
	mutable CryptoPP::BlowfishEncryption* mEncryptP;  // Encryption object
	mutable CryptoPP::BlowfishDecryption* mDecryptP;  // Decryption object

	// Private Methods
	void ClearLocal();
};


};  //namespace WONCrypt

#endif