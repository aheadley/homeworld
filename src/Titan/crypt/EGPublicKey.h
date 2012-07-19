#ifndef _EGPUBLICKEY_H
#define _EGPUBLICKEY_H

// EGPublicKey

// Class that encapsulates an ElGamal Public key used for encryption and signatures.
// ElGamal Public keys cannot be created by this class.  Use the ElGamal private key
// class to generate the corresponding public key.


#include <ostream>
#include "PublicKey.h"

// Avoid including crypto headers anywhere but in WONCrypt source
#ifdef _WONCRYPT_SOURCEFILE
#include "cryptoFiles/md5.h"
#include "cryptoFiles/nr.h"
#endif

// From Crypto++
namespace CryptoPP {
	class ElGamalEncryptor;
	class BufferedTransformation;
};

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class EGPrivateKey;
class CryptException;


class EGPublicKey : public PublicKey
{
public:
	// Default constructor - makes an uncreated EGPublicKey.  Create method must
	// be called to make a valid key
	EGPublicKey();

	// Constructor - copy existing key from buffer.
	EGPublicKey(unsigned short theLen, const unsigned char* theKey) throw(CryptException);

	// Construct from private key
	explicit EGPublicKey(const EGPrivateKey& theKey) throw(CryptException);

	// Copy constructor
	EGPublicKey(const EGPublicKey& theKey);

	// Destructor
	~EGPublicKey(void);

	// Operators
	EGPublicKey& operator=(const EGPublicKey& theKey);
	EGPublicKey& operator=(const EGPrivateKey& theKey);

	// Create key - from existing key.
	void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Encrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	CryptReturn Encrypt(const void* theMsgP, unsigned long theLen) const;

	// Verify a signature block.  Returns true if signature is valid, false if not.
	bool Verify(const unsigned char* theSigP, unsigned long theSigLen,
	            const void* theMsgP, unsigned long theMsgLen) const;

	// Dump key to stream
	void Dump(std::ostream& os) const;

private:
	mutable CryptoPP::ElGamalEncryptor* mCryptP;  // Crypt object

// Avoid including crypto headers anywhere but in WONCrypt source
// So we'll trick the compiler by making mSigP a void* everywhere else
#ifdef _WONCRYPT_SOURCEFILE
	typedef CryptoPP::NRVerifier<CryptoPP::MD5> ElGamalVerifier;
	mutable ElGamalVerifier* mSigP;    // Verifier object
#else
	mutable void* mSigP;
#endif

	// Private Methods
	void CopyFromPrivateKey(const EGPrivateKey& theKeyR);
	void ClearLocal();
	void AllocateCrypt() const;
	void AllocateSig() const;
	void EncryptBlock(CryptoPP::BufferedTransformation& aQueue, const unsigned char* theBlockP,
	                  unsigned long theLen) const;
	void EncryptData(CryptoPP::BufferedTransformation& aQueue, const unsigned char* theMsgP,
	                 unsigned long theLen) const;
};

};  //namespace WONCrypt


#endif