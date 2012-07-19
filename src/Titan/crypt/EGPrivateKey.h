#ifndef _EGPRIVATEKEY_H
#define _EGPRIVATEKEY_H

// EGPrivateKey

// Class that encapsulates an Elgamal Private key used for encryption and signatures.
// Length of each portion of the key in bytes can be defined on creation.

// Note that EGPrivateKey also generates the corresponding EGPublicKey.


#include <ostream>
#include "PrivateKey.h"

// Avoid including crypto headers anywhere but in WONCrypt source
#ifdef _WONCRYPT_SOURCEFILE
#include "cryptoFiles/md5.h"
#include "cryptoFiles/nr.h"
#endif

// From Crypto++
namespace CryptoPP {
	class ElGamalDecryptor;
	class BufferedTransformation;
};

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class EGPublicKey;
class CryptException;


class EGPrivateKey : public PrivateKey
{
public:
	// Default constructor - makes an uncreated EGPrivateKey.  Create method must
	// be called to make a valid key
	EGPrivateKey();

	// Constructor - Build a new key using specified len (in bytes) for each
	// portion of the key.
	explicit EGPrivateKey(unsigned short theLen) throw(CryptException);

	// Constructor - Build from existing key buffer.
	EGPrivateKey(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Copy constructor
	EGPrivateKey(const EGPrivateKey& theKey);

	// Destructor
	~EGPrivateKey(void);

	// Operators
	EGPrivateKey& operator=(const EGPrivateKey& theKey);

	// Create key - from existing key.
	void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Create key - new key of specified length.
	// NOTE: Specifying theLen as 0 will cause the default key length to be used.
	void Create(unsigned short theLen=0) throw(CryptException);

	// Fetch the EGPublicKey (overridden from PrivateKey)
	const PublicKey& GetPublicKey() const throw(CryptException);

	// Decrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).  Will
	// return a (NULL, 0) pair if decryption fails.
	CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen) const;

	// Sign a block.  Returns allocated signature block and length of signature block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	CryptReturn Sign(const void* theMsgP, unsigned long theLen) const;

	// Dump key to stream
	void Dump(std::ostream& os) const;

private:
	// Types

	// Members
	mutable EGPublicKey*                mPubKeyP;  // Buffer for generating EGPublicKey
	mutable CryptoPP::ElGamalDecryptor* mCryptP;   // Decrypt object

// Avoid including crypto headers anywhere but in WONCrypt source
// So we'll trick the compiler by making mSigP a void* everywhere else
#ifdef _WONCRYPT_SOURCEFILE
	typedef CryptoPP::NRSigner<CryptoPP::MD5> ElGamalSigner;
	mutable ElGamalSigner* mSigP;  // Signature object
#else
	mutable void* mSigP;
#endif

	// Private methods
	void ClearLocal();
	void AllocateCrypt() const;
	void AllocateSig() const;
	void DecryptBlock(CryptoPP::BufferedTransformation& aQueue, const unsigned char* theBlockP,
	                  unsigned long theLen) const;
	bool DecryptData(CryptoPP::BufferedTransformation& aQueue, const unsigned char* theMsgP,
	                 unsigned long theLen) const;
};


};  //namespace WONCrypt

#endif