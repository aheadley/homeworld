#ifndef _CLIENTCDKEY_H
#define _CLIENTCDKEY_H

// ClientCDKey

// Class that implements WON CD-Key functionality for use by clients.  Allows
// conversion to/from human readable string.  Implements persistance to the registry
// in a secure manner.  Implements a lightweight validation check of the key.

#include "../common/won.h"
#include <ostream>

// Forwards
namespace WONCrypt {
	class BFSymmetricKey;
};


// In WONCDKey namespace
namespace WONCDKey
{

class ClientCDKey
{
public:
	// Standard constructor
	// ProductName is string representing product using the CDKey, (i.e., "Homeworld")
	// *IMPORTANT*  product must be at least 3 chars for good security!!!
	explicit ClientCDKey(const std::string& theProductR);

	// Copy Constructor
	ClientCDKey(const ClientCDKey& theKeyR);

	// Destructor
	virtual ~ClientCDKey();

	// Operators
	ClientCDKey& operator=(const ClientCDKey& theKeyR);
	bool operator==(const ClientCDKey& theKeyR) const;
	bool operator!=(const ClientCDKey& theKeyR) const;

	// Product access.  Careful setting product.  Product is used to encrypt/decypt
	// the key to/from its binary form and for the Lightweight check.
	const std::string& GetProduct() const;
	void SetProduct(const std::string& theProductR);

	// Check two keys for equality
	virtual bool IsEqual(const ClientCDKey& theKeyR) const;

	// Check validity of a key, performs lightweight check if needed.
	virtual bool IsValid() const;

	// Is this a beta key?
	bool IsBeta() const;

	// Init from raw, unencrypted key.  A return of true DOES NOT imply key is valid!
	// (Call IsValid() to verify validity).
	virtual bool Init(const __int64& theKeyR);

	// Initialize from Human readable string (dashes are optional):
	//   CVCN-CVCN-CVCN-CVCN-NNNN
	// Returns true if init is successful, false if not.  A return of true DOES NOT
	// imply key is valid!  (Call IsValid() to verify validity).
	virtual bool Init(const std::string& theStrR);

	// Init from encrypted binary image (what is returned by AsBinary).  A return of
	// true DOES NOT imply key is valid!  (Call IsValid() to verify validity).
	virtual bool Init(const WONCommon::RawBuffer& theKeyR);

	// Fetch as raw, unencrypted (64bit) key
	__int64 AsRaw() const;

	// Fetch as human readable string.  Validates key if needed if validate is true.
	// If Keys fails to validate, returns "#######INVALID KEY######"
	const std::string& AsString(bool validate=true) const;

	// Fetch as encrypted binary image.  Valdates key if needed.  Returns empty
	// buffer if key is not valid.
	const WONCommon::RawBuffer& AsBinary() const;

	// Load/Save key in registry securely (key is encrypted in the registry)
	virtual bool Load();
	virtual bool Save() const;

	// Clean key from registry
	virtual bool CleanReg();

protected:
	// Types
	enum ValidityState { Unknown, Invalid, Valid };

	// Members
	std::string           mProduct;     // Product string
	unsigned char         mLightCheck;  // Lightweight checksum
	WONCommon::RawBuffer  mKey;         // Key value
	mutable ValidityState mValidity;    // Current key validity

	// Buffers for string and binary versions of key
	mutable std::string          mStrKey;  // CD-Key as string
	mutable WONCommon::RawBuffer mBinKey;  // CD-Key as encrypted binary

	// Perform the light validity check.  Returns light checksum
	unsigned char LightValidityCheck() const;

	// Build human readable string rep of CDKey.  Fills in mStrKey
	void BuildStringKey() const;

	// Extract LightCheck, and Key from a buffer
	void FieldsFromBuffer(const __int64& theBuf);

	// Build buffer from LightCheck and Key
	__int64 BufferFromFields() const;

	// Decypt mBin Key into buffer
	bool DecryptKey(__int64& theBufR);

	// Encrypt buffer into mBinKey
	bool EncryptKey(const __int64& theBufR) const;

private:
	// Methods
	void BuildCChar(const __int64& theBuf, unsigned int& theOffset) const;
	void BuildVChar(const __int64& theBuf, unsigned int& theOffset) const;
	void BuildNChar(const __int64& theBuf, unsigned int& theOffset) const;
	void CreateSymmetricKey(WONCrypt::BFSymmetricKey& theSymKeyR) const;

	// Class methods
	static void RemoveSkipChars(string& theStrR);
	static char ValFromBits(const __int64& aBuf, unsigned int theOffset, unsigned int theBits);
	static bool ProcessCChar(__int64& theBuf, unsigned int& theOffset, char theChar);
	static bool ProcessVChar(__int64& theBuf, unsigned int& theOffset, char theChar);
	static bool ProcessNChar(__int64& theBuf, unsigned int& theOffset, char theChar);
};


// Inlines
inline bool
ClientCDKey::operator==(const ClientCDKey& theKeyR) const
{ return IsEqual(theKeyR); }

inline bool
ClientCDKey::operator!=(const ClientCDKey& theKeyR) const
{ return (! IsEqual(theKeyR)); }

inline const std::string&
ClientCDKey::GetProduct() const
{ return mProduct; }

inline void
ClientCDKey::SetProduct(const std::string& theProductR)
{ mProduct = theProductR; }

inline __int64
ClientCDKey::AsRaw() const
{ return BufferFromFields(); }

};  //namespace WONCDKey


// Output operators
inline std::ostream&
operator<<(std::ostream& os, const WONCDKey::ClientCDKey& theKeyR)
{ os << theKeyR.AsString();  return os; }

#endif