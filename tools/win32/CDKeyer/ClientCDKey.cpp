// ClientCDKey

// Class that implements WON CD-Key functionality for use by clients.  Allows conversion
// to/from human readable string.  Implements persistance to the registry in a secure
// manner.  Implements a lightweight validation check of the key.


// Uncomment the following line to use original Lightweight check algorithm.  This
// should only be needed for CD-Keys used in Beta builds of Homeworld.
//#define __LWCHECK_ORIG


#include "common/won.h"
#include "common/OutputOperators.h"
#include "common/RegKey.h"
#include "common/CRC16.h"
#include "crypt/BFSymmetricKey.h"
#include "crypt/CryptException.h"
#include "ClientCDKey.h"

#ifdef _DEBUG
#include "common/WONException.h"
#include "common/WONExceptCodes.h"
#endif

// private namespace for using and constants
namespace {
	using std::hex;
	using std::dec;
	using WONCommon::RawBuffer;
	using WONCommon::RegKey;
	using WONCommon::CRC16;
	using WONCrypt::BFSymmetricKey;
	using WONCDKey::ClientCDKey;

	// Constants
	const unsigned char BETA_MASK      = 0x01;
	const unsigned int  BINARYKEY_LEN  = 16;
	const char          SKIPCHAR_MAP[] = { '-', ' ', '\t', '\0' };
	const char*         STRINGKEY_MAP  = "CVCNCVCNCVCNCVCNNNNN";
	const unsigned int  STRINGKEY_LEN  = strlen(STRINGKEY_MAP);
	const int           DASH_OFFSET    = 4;
	const string        REG_CDKEY_PATH(REG_CONST::REG_WON_KEY_NAME + REG_CONST::REG_BACKSLASH + "CDKeys");
	const string        INVALIDKEY_STR(  "#######INVALID KEY######");
};


// ** Constructors / Destructor

// Standard constructor
ClientCDKey::ClientCDKey(const string& theProductR) :
	mProduct(theProductR),
	mValidity(Invalid),
	mLightCheck(0),
	mKey(),
	mStrKey(),
	mBinKey()
{}


// Copy Constructor
ClientCDKey::ClientCDKey(const ClientCDKey& theKeyR) :
	mProduct(theKeyR.mProduct),
	mValidity(theKeyR.mValidity),
	mLightCheck(theKeyR.mLightCheck),
	mKey(theKeyR.mKey),
	mStrKey(theKeyR.mStrKey),
	mBinKey(theKeyR.mBinKey)
{}

	
// Destructor
ClientCDKey::~ClientCDKey()
{}


// ** Private Methods **

// ClientCDKey::RemoveSkipChars
// Removes all occurences of chars in the SKIPCHAR_MAP from specified string.  Note
// that string is modified in place.
void
ClientCDKey::RemoveSkipChars(string& theStrR)
{
	WTRACE("ClientCDKey::RemoveSkipChars");
	WDBG_LL("ClientCDKey::RemoveSkipChars Old string=" << theStrR);

	// Delete all chars from SKIPCHAR map
	for (const char* p=SKIPCHAR_MAP; *p; p++)
	{
		string::size_type aPos = theStrR.rfind(*p);
		while (aPos != string::npos)
		{
			theStrR.erase(aPos, 1);
			aPos = theStrR.rfind(*p);
		}
	}

	WDBG_LL("ClientCDKey::RemoveSkipChars New string=" << theStrR);
}


// ClientCDKey::ValFromBits
// Generates unsignec har value starting a bit theOffset and using theBits bits from
// 8 byte buffer.
char
ClientCDKey::ValFromBits(const __int64& aBuf, unsigned int theOffset, unsigned int theBits)
{
	WTRACE("ClientCDKey::ValFromBits");
	char aRet = 0;
	while (theBits-- > 0)
	{
		aRet <<= 1;
		__int64 aMask = 1;
		aMask <<= (theOffset + theBits);
		if (aBuf & aMask)
			aRet |= 0x01;
	}

	return aRet;
}


// ClientCDKey::ProcessCChar
// Places a C into 8 byte buffer starting at theOffset.  Cs use 4 bits.  Value of
// theOffset is incremented by 4.
bool
ClientCDKey::ProcessCChar(__int64& theBuf, unsigned int& theOffset, char theChar)
{
	WTRACE("ClientCDKey::ProcessCChar");
	WDBG_LL("ClientCDKey::ProcessCChar char=" << theChar << " offset=" << theOffset);
	bool    aRet  = true;
	__int64 aMask = 0;

	// Determine mask based on char
	switch (toupper(theChar))
	{
	case 'B':
		aMask = 0;   break;
	case 'C':
		aMask = 1;   break;
	case 'D':
		aMask = 2;   break;
	case 'F':
		aMask = 3;   break;
	case 'G':
		aMask = 4;   break;
	case 'J':
		aMask = 5;   break;
	case 'L':
		aMask = 6;   break;
	case 'M':
		aMask = 7;   break;
	case 'N':
		aMask = 8;   break;
	case 'P':
		aMask = 9;   break;
	case 'R':
		aMask = 10;  break;
	case 'S':
		aMask = 11;  break;
	case 'T':
		aMask = 12;  break;
	case 'W':
		aMask = 13;  break;
	case 'X':
		aMask = 14;  break;
	case 'Z':
		aMask = 15;  break;
	default:
		WDBG_LH("ClientCDKey::ProcessCChar Bad input, char=" << theChar);
		return false;
	}

	// Shift mask by offset and or with buffer.  Update offset
	theBuf |= (aMask << theOffset);
	theOffset += 4;
	return aRet;
}


// ClientCDKey::ProcessVChar
// Places a V into 8 byte buffer starting at theOffset.  Vs use 2 bits.  Value of
// theOffset is incremented by 2.
bool
ClientCDKey::ProcessVChar(__int64& theBuf, unsigned int& theOffset, char theChar)
{
	WTRACE("ClientCDKey::ProcessVChar");
	WDBG_LL("ClientCDKey::ProcessVChar char=" << theChar << " offset=" << theOffset);
	bool    aRet  = true;
	__int64 aMask = 0;

	// Determine mask based on char
	switch (toupper(theChar))
	{
	case 'A':
		aMask = 0;  break;
	case 'E':
		aMask = 1;  break;
	case 'U':
		aMask = 2;  break;
	case 'Y':
		aMask = 3;  break;
	default:
		WDBG_LH("ClientCDKey::ProcessVChar Bad input, char=" << theChar);
		return false;
	}

	// Shift mask by offset and or with buffer.  Update offset
	theBuf |= (aMask << theOffset);
	theOffset += 2;
	return aRet;
}


// ClientCDKey::ProcessNChar
// Places a N into 8 byte buffer starting at theOffset.  Ns use 3 bits.  Value of
// theOffset is incremented by 3.
bool
ClientCDKey::ProcessNChar(__int64& theBuf, unsigned int& theOffset, char theChar)
{
	WTRACE("ClientCDKey::ProcessNChar");
	WDBG_LL("ClientCDKey::ProcessNChar char=" << theChar << " offset=" << theOffset);
	bool aRet = ((isdigit(theChar)) && (theChar > '1'));
	if (aRet)
	{
		__int64 aMask = (theChar - '0') - 2;

		// Shift mask by offset and or with buffer.  Update offset
		theBuf |= (aMask << theOffset);
		theOffset += 3;
	}

	return aRet;
}


// ClientCDKey::BuildCChar
// Extracts a C from buffer starting at offset theOffset.  Extracted C is appened
// to mStrKey.  Cs use 4 bits.  Value of theOffset is incremented by 4.
void
ClientCDKey::BuildCChar(const __int64& theBuf, unsigned int& theOffset) const
{
	WTRACE("ClientCDKey::BuildCChar");
	WDBG_LL("ClientCDKey::BuildCChar offset=" << theOffset);
	char aChar = 0;

	// Determine mask based on char
	switch (ValFromBits(theBuf, theOffset, 4))
	{
	case 0:
		aChar = 'B';   break;
	case 1:
		aChar = 'C';   break;
	case 2:
		aChar = 'D';   break;
	case 3:
		aChar = 'F';   break;
	case 4:
		aChar = 'G';   break;
	case 5:
		aChar = 'J';   break;
	case 6:
		aChar = 'L';   break;
	case 7:
		aChar = 'M';   break;
	case 8:
		aChar = 'N';   break;
	case 9:
		aChar = 'P';   break;
	case 10:
		aChar = 'R';   break;
	case 11:
		aChar = 'S';   break;
	case 12:
		aChar = 'T';   break;
	case 13:
		aChar = 'W';   break;
	case 14:
		aChar = 'X';   break;
	case 15:
		aChar = 'Z';   break;
#ifdef _DEBUG
	default:
		throw WONCommon::WONException(WONCommon::ExSoftwareFail, __LINE__, __FILE__,
			                          "ClientCDKey::BuildCChar ValFromBits returned invalid char!");
		break;
#endif
	}

	// Add char to buf and update offset
	WDBG_LL("ClientCDKey::BuildCChar Char=" << aChar);
	mStrKey   += aChar;
	theOffset += 4;
}


// ClientCDKey::BuildVChar
// Extracts a V from buffer starting at offset theOffset.  Extracted V is appened
// to mStrKey.  Vs use 2 bits.  Value of theOffset is incremented by 2.
void
ClientCDKey::BuildVChar(const __int64& theBuf, unsigned int& theOffset) const
{
	WTRACE("ClientCDKey::BuildVChar");
	WDBG_LL("ClientCDKey::BuildVChar offset=" << theOffset);
	char aChar = 0;

	// Determine mask based on char
	switch (ValFromBits(theBuf, theOffset, 2))
	{
	case 0:
		aChar = 'A';   break;
	case 1:
		aChar = 'E';   break;
	case 2:
		aChar = 'U';   break;
	case 3:
		aChar = 'Y';   break;
#ifdef _DEBUG
	default:
		throw WONCommon::WONException(WONCommon::ExSoftwareFail, __LINE__, __FILE__,
			                          "ClientCDKey::BuildVChar ValFromBits returned invalid char!");
		break;
#endif
	}

	// Add char to buf and update offset
	WDBG_LL("ClientCDKey::BuildVChar Char=" << aChar);
	mStrKey   += aChar;
	theOffset += 2;
}


// ClientCDKey::BuildNChar
// Extracts a N from buffer starting at offset theOffset.  Extracted N is appened
// to mStrKey.  Ns use 32 bits.  Value of theOffset is incremented by 3.
void
ClientCDKey::BuildNChar(const __int64& theBuf, unsigned int& theOffset) const
{
	WTRACE("ClientCDKey::BuildNChar");
	WDBG_LL("ClientCDKey::BuildNChar offset=" << theOffset);
	char aChar = ValFromBits(theBuf, theOffset, 3) + '0' + 2;

#ifdef _DEBUG
	if ((aChar < '2') || (aChar > '9'))
		throw WONCommon::WONException(WONCommon::ExSoftwareFail, __LINE__, __FILE__,
			                          "ClientCDKey::BuildNChar ValFromBits returned invalid char!");
#endif

	// Add char to buf and update offset
	WDBG_LL("ClientCDKey::BuildNChar Char=" << aChar);
	mStrKey   += aChar;
	theOffset += 3;
}


// ClientCDKey::CreateSymmetricKey
// Creates a symmetric key from product name used to save/load CD-Key to/from
// the registry.  Symmetric key is created via a series of CRCs on the product.
void
ClientCDKey::CreateSymmetricKey(BFSymmetricKey& theSymKeyR) const
{
	WTRACE("ClientCDKey::CreateSymmetricKey");
	WDBG_LL("ClientCDKey::CreateSymmetricKey from product=" << mProduct);
	CRC16     aCRC;
	RawBuffer aBuf;

	// CRC the product and use it as 1st 2 bytes of key
	aCRC.Put(mProduct);
	unsigned short aCheckSum = aCRC.GetCRC();
	WDBG_LL("ClientCDKey::CreateSymmetricKey First CRC=" << aCheckSum);
	aBuf.assign(reinterpret_cast<unsigned char*>(&aCheckSum), sizeof(aCheckSum));

	// CRC each of 1st 3 chars of product and add them to key.
	for (int i=0; (i < 3) && (i < mProduct.size()); i++)
	{
		aCRC.Put(static_cast<unsigned char>(mProduct[i]));
		aCheckSum = aCRC.GetCRC();
		WDBG_LL("ClientCDKey::CreateSymmetricKey Add CRC=" << aCheckSum);
		aBuf.append(reinterpret_cast<unsigned char*>(&aCheckSum), sizeof(aCheckSum));
	}

	// Create the key
	WDBG_LL("ClientCDKey::CreateSymmetricKey Buf=" << aBuf);
	theSymKeyR.Create(aBuf.size(), aBuf.data());
}


// ** Protected Methods **

// ClientCDKey::LightValidityCheck
// Perform lightwight validty check on mKey.  Return light checksum.
unsigned char
ClientCDKey::LightValidityCheck() const
{
	WTRACE("ClientCDKey::LightValidityCheck");
	WDBG_LL("ClientCDKey::LightValidityCheck key=" << mKey);
	CRC16 aCRC;

#ifndef __LWCHECK_ORIG
	aCRC.Put(mProduct);
#endif

	aCRC.Put(mKey);
	WDBG_LL("ClientCDKey::LightValidityCheck Key CRC=" << aCRC.GetCRC());

	// Take the middle 8 bits of the the 16 bit CRC to get the 8-bit light check
	unsigned char aRet = (aCRC.GetCRC() & 0x0fff) >> 4;

	WDBG_LL(hex << "ClientCDKey::LightValidityCheck Checksum=" << aCRC.GetCRC() << " lightCheck=" << aRet << dec);
	return aRet;
}


// ClientCDKey::BuildStringKey
// Build string representation of CD-Key from internal form.
void
ClientCDKey::BuildStringKey() const
{
	WTRACE("ClientCDKey::BuildStringKey");
	__int64      aBuf     = BufferFromFields();
	unsigned int anOffset = 0;

	// Extract each char from 8 byte vuffer and add to mStrKey
	WDBG_LL("ClientCDKey::BuildStringKey Buf=" << hex << aBuf << dec);
	mStrKey.erase();
	for (int i=0; i < STRINGKEY_LEN;)
	{
		switch (STRINGKEY_MAP[i])
		{
		case 'C':
			BuildCChar(aBuf, anOffset);  break;
		case 'V':
			BuildVChar(aBuf, anOffset);  break;
		case 'N':
			BuildNChar(aBuf, anOffset);  break;
#ifdef _DEBUG
	default:
		throw WONCommon::WONException(WONCommon::ExSoftwareFail, __LINE__, __FILE__,
			                          "ClientCDKey::BuildStringKey Unknown char is STRINGKEY_MAP!");
		break;
#endif
		}

		// Add dash every four chars
		if ((((++i) % DASH_OFFSET) == 0) && (i < STRINGKEY_LEN))
			mStrKey += '-';
	}

	WDBG_LL("ClientCDKey::BuildStringKey StrKey=" << mStrKey);
}


// ClientCDKey::FieldsFromBuffer
// Fill in internal fields from 8 byte buffer.  Light check is byte 3, key is bytes
// 0-2 and 4-7.
void
ClientCDKey::FieldsFromBuffer(const __int64& theBuf)
{
	WTRACE("ClientCDKey::FieldsFromBuffer");
	WDBG_LL("ClientCDKey::FieldsFromBuffer Buffer=" << hex << theBuf << dec);
	const unsigned char* aP = reinterpret_cast<const unsigned char*>(&theBuf);

	// Extract lightCheck, byte 3
	mLightCheck = *(aP+3);
	WDBG_LL("ClientCDKey::FieldsFromBuffer lightCheck=" << hex << mLightCheck << dec);

	// Key value is rest of bytes (0-2, 4-7)
	mKey.assign(aP, 3);
	mKey.append(aP+4, 4);
	WDBG_LL("ClientCDKey::FieldsFromBuffer key=" << mKey);
}


// ClientCDKey::BufferFromFields
// Build 8 byte raw form from internal fields.  Light check is byte 3, key is bytes
// 0-2 and 4-7.
__int64
ClientCDKey::BufferFromFields() const
{
	WTRACE("ClientCDKey::BufferFromFields");
	__int64        aRet = 0;
	unsigned char* aP   = reinterpret_cast<unsigned char*>(&aRet);

	// Put lightCheck, byte 3
	WDBG_LL("ClientCDKey::BufferFromFields lightCheck=" << hex << mLightCheck << dec);
	*(aP+3) = mLightCheck;

	// Put key, bytes 0-2, 4-7
	memcpy(aP, mKey.data(), 3);
	memcpy(aP+4, mKey.data()+3, 4);

	WDBG_LL("ClientCDKey::FieldsFromBuffer Buffer=" << hex << aRet << dec);
	return aRet;
}


// ClientCDKey::DecryptKey
// Decrypt 16 byte binary key into 8 byte buffer.  Uses symmetric key built from product
// name for decryption.
bool
ClientCDKey::DecryptKey(__int64& theBufR)
{
	WTRACE("ClientCDKey::DecryptKey");
	try
	{
		// Build symmetric key from product
		WDBG_LL("ClientCDKey::DecryptKey Creating symmetric key from product=" << mProduct);
		BFSymmetricKey aSymKey;
		CreateSymmetricKey(aSymKey);

		// Decrypt the key
		WDBG_LL("ClientCDKey::DecryptKey Decrypting CDKey.");
		BFSymmetricKey::CryptReturn aDecrypt(aSymKey.Decrypt(mBinKey.data(), mBinKey.size()));
		auto_ptr<unsigned char>     aDelP(aDecrypt.first);
		if (aDecrypt.second != sizeof(theBufR))
		{
			WDBG_LM("ClientCDKey::DecryptKey Decrypt of key has bad length.");
			return false;
		}

		// Fill buffer with decrypted key
		memcpy(static_cast<void*>(&theBufR), aDecrypt.first, sizeof(theBufR));
	}
	catch (WONCrypt::CryptException& anExR)
	{
		WDBG_LH("ClientCDKey::DecryptKey exception decrypting key: " << anExR);
		return false;
	}

	return true;
}


// ClientCDKey::EncryptKey
// Encrypts into 8 byte buffer in 16 byte binary.  Uses symmetric key built from product
// name for encryption.
bool
ClientCDKey::EncryptKey(const __int64& theBufR) const
{
	WTRACE("ClientCDKey::EncryptKey");
	try
	{
		// Build symmetric key from product
		WDBG_LL("ClientCDKey::EncryptKey Creating symmetric key from product=" << mProduct);
		BFSymmetricKey aSymKey;
		CreateSymmetricKey(aSymKey);

		// Decrypt the key
		WDBG_LL("ClientCDKey::EncryptKey Encrypting CDKey.");
		BFSymmetricKey::CryptReturn anEncrypt(aSymKey.Encrypt(reinterpret_cast<const unsigned char*>(&theBufR), sizeof(theBufR)));
		auto_ptr<unsigned char>     aDelP(anEncrypt.first);
		if (anEncrypt.second != BINARYKEY_LEN)
		{
			WDBG_LM("ClientCDKey::EncryptKey Encrypt of key has bad length.");
			return false;
		}

		// Fill BinKey with encrypted key
		mBinKey.assign(anEncrypt.first, anEncrypt.second);
	}
	catch (WONCrypt::CryptException& anExR)
	{
		WDBG_LH("ClientCDKey::EncryptKey exception encrypting key: " << anExR);
		return false;
	}

	return true;
}


// ** Public Methods **

// Assignment operator
ClientCDKey&
ClientCDKey::operator=(const ClientCDKey& theKeyR)
{
	if (this != &theKeyR)  // protect vs a = a
	{
		mProduct    = theKeyR.mProduct;
		mLightCheck = theKeyR.mLightCheck;
		mKey        = theKeyR.mKey;
		mValidity   = theKeyR.mValidity;
		mStrKey     = theKeyR.mStrKey;
		mBinKey     = theKeyR.mBinKey;
	}
	return *this;
}


// ClientCDKey::IsEqual
// Comapre 2 client keys for equality.
bool
ClientCDKey::IsEqual(const ClientCDKey& theKeyR) const
{
	WTRACE("ClientCDKey::IsEqual");
	return ((mLightCheck == theKeyR.mLightCheck) && (mKey == theKeyR.mKey) &&
	        (mProduct == theKeyR.mProduct));
}


// ClientCDKey::IsValid
// Check client key for validity.  Performs lightweight check only if needed.
bool
ClientCDKey::IsValid() const
{
	WTRACE("ClientCDKey::IsValid");
	if (mValidity == Unknown)
		mValidity = (LightValidityCheck() == mLightCheck ? Valid : Invalid);

	WDBG_LM("ClientCDKey::IsValid Validity=" << mValidity);
	return (mValidity == Valid);
}


// ClientCDKey::IsBeta
// Checks to see if this is a beta key.  Key is beta if bit 0 of first byte of
// mKey is set.
bool
ClientCDKey::IsBeta() const
{
	WTRACE("ClientCDKey::IsBeta");
	WDBG_LM("ClientCDKey::IsBeta Key=" << mKey);
	return (mKey.size() >= 1 ? (mKey[0] & BETA_MASK) : false);
}


// ClientCDKey::Init(__int64)
// Initializes key from 8 byte raw buffer.  Does not validate key.
bool
ClientCDKey::Init(const __int64& theKeyR)
{
	WTRACE("ClientCDKey::Init(__int64)");
	WDBG_LH("ClientCDKey::Init(__int64) Key=" << hex << theKeyR << dec);
	mValidity = Unknown;
	mKey.erase();
	mStrKey.erase();
	mBinKey.erase();

	WDBG_LM("ClientCDKey::Init(__int64) Extracting fields.");
	FieldsFromBuffer(theKeyR);
	return true;
}


// ClientCDKey::Init(string)
// Initializes key from human readbale string.  Does not validate key.
bool
ClientCDKey::Init(const string& theKeyR)
{
	WTRACE("ClientCDKey::Init(string)");
	WDBG_LH("ClientCDKey::Init(string) String=" << theKeyR);
	mValidity = Unknown;
	mKey.erase();
	mBinKey.erase();

	// Init StrKey and remove and dashes
	mStrKey = theKeyR;
	RemoveSkipChars(mStrKey);

	// Validate length
	if (mStrKey.size() != STRINGKEY_LEN)
	{
		WDBG_LH("ClientCDKey::Init(string) Key length invalid, len=" << mStrKey.size());
		mValidity = Invalid;
		return false;
	}

	// Process each char in string
	WDBG_LM("ClientCDKey::Init(string) Parsing string key.");
	__int64      aBuf     = 0;
	unsigned int anOffset = 0;
	for (int i=0; i < STRINGKEY_LEN; i++)
	{
		bool aTst = true;
		switch (STRINGKEY_MAP[i])
		{
		case 'C':
			aTst = ProcessCChar(aBuf, anOffset, mStrKey[i]);  break;
		case 'V':
			aTst = ProcessVChar(aBuf, anOffset, mStrKey[i]);  break;
		case 'N':
			aTst = ProcessNChar(aBuf, anOffset, mStrKey[i]);  break;
#ifdef _DEBUG
		default:
			throw WONCommon::WONException(WONCommon::ExSoftwareFail, __LINE__, __FILE__,
										  "ClientCDKey::Init(string) Unknown char is STRINGKEY_MAP!");
			break;
#endif
		}

		if (! aTst)
		{
			WDBG_LH("ClientCDKey::Init(string) Parse of string to binary failed.");
			mValidity = Invalid;
			return false;
		}
	}

	// Extract fields from buffer
	WDBG_LM("ClientCDKey::Init(string) Extracting fields.");
	FieldsFromBuffer(aBuf);
	mStrKey.erase();
	return true;
}


// ClientCDKey::Init(RawBuffer)
// Initializes key from encrypted binary buffer.  Does not validate key.
bool
ClientCDKey::Init(const RawBuffer& theKeyR)
{
	WTRACE("ClientCDKey::Init(RawBuffer)");
	WDBG_LH("ClientCDKey::Init(RawBuffer) Buf=" << theKeyR);
	mValidity = Unknown;
	mKey.erase();
	mStrKey.erase();

	// Init BinKey and validate length
	mBinKey = theKeyR;
	if (mBinKey.size() != BINARYKEY_LEN)
	{
		WDBG_LH("ClientCDKey::Init(RawBuffer) Key length invalid, len=" << mBinKey.size());
		mValidity = Invalid;
		return false;
	}

	// Build symmetric key from Product name
	__int64 aBuf = 0;
	if (! DecryptKey(aBuf))
	{
		WDBG_LH("ClientCDKey::Init(RawBuffer) Decrypt of bin key failed.");
		mValidity = Invalid;
		return false;
	}

	WDBG_LM("ClientCDKey::Init(RawBuffer) Extracting fields.");
	FieldsFromBuffer(aBuf);
	return true;
}


// ClientCDKey::AsString
// Returns human readable string form of CD-Key.  Builds string form if needed.
const string&
ClientCDKey::AsString(bool validate) const
{
	WTRACE("ClientCDKey::AsString");
	if ((mStrKey.empty()) || (mValidity != Valid))
	{
		if ((! validate) || (IsValid()))
			BuildStringKey();
		else
			mStrKey = INVALIDKEY_STR;
	}
	return mStrKey;
}


// ClientCDKey::AsString
// Returns encrypted binary form of CD-Key.  Builds binary form if needed.
const RawBuffer&
ClientCDKey::AsBinary() const
{
	WTRACE("ClientCDKey::AsBinary");
	if (mBinKey.empty())
	{
		if (IsValid())
			EncryptKey(BufferFromFields());
		else
			mBinKey.erase();
	}

	return mBinKey;
}


// ClientCDKey::Load
// Loads CD-Key from WON standard location in registry.  Entry is registry must be
// the enrypted binary form.  Calls Init(RawBuffer).
bool
ClientCDKey::Load()
{
	WTRACE("ClientCDKey::Load");
	WDBG_LM("ClientCDKey::Load Loading key from registry.");

	// Open registry key
	RegKey aRegKey(REG_CDKEY_PATH, HKEY_LOCAL_MACHINE);
	if (! aRegKey.IsOpen())
	{
		WDBG_LH("ClientCDKey::Load Fail open registry key: " << REG_CDKEY_PATH);
		mValidity = Invalid;
		return false;
	}

	// Fetch key from registry for value product
	unsigned char* aBufP = NULL;
	unsigned long  aLen  = 0;
	if (aRegKey.GetValue(mProduct, aBufP, aLen) != RegKey::Ok)
	{
		WDBG_LH("ClientCDKey::Load Fail fetch key value for product=" << mProduct);
		mValidity = Invalid;
		return false;
	}

	// Build buffer and call Init(RawBuffer)
	RawBuffer aKey(aBufP, aLen);
	delete aBufP;
	return Init(aKey);
}


// ClientCDKey::Save
// Saves CD-Key to WON standard location in registry.  Entry is saved to registry as
// the enrypted binary form.  Calls AsBinary().
bool
ClientCDKey::Save() const
{
	WTRACE("ClientCDKey::Save");
	WDBG_LM("ClientCDKey::Save Save encrypted key to registry.");

	// Open registry key
	RegKey aRegKey(REG_CDKEY_PATH, HKEY_LOCAL_MACHINE, true);
	if (! aRegKey.IsOpen())
	{
		WDBG_LH("ClientCDKey::Save Fail open registry key: " << REG_CDKEY_PATH);
		return false;
	}

	// Build encrypted binary form if needed and write to registry
	AsBinary();
	return aRegKey.SetValue(mProduct.c_str(), mBinKey.data(), mBinKey.size());
}


// ClientCDKey::CleanReg
// Cleans registry of CD-Key in WON standard location.
bool
ClientCDKey::CleanReg()
{
	WTRACE("ClientCDKey::CleanReg");
	WDBG_LM("ClientCDKey::CleanReg Clean key from registry.");

	// Open registry key
	RegKey aRegKey(REG_CDKEY_PATH, HKEY_LOCAL_MACHINE, true);
	if (! aRegKey.IsOpen())
	{
		WDBG_LH("ClientCDKey::CleanReg Fail open registry key: " << REG_CDKEY_PATH);
		return false;
	}

	// Delete key from registry
	return aRegKey.DeleteValue(mProduct.c_str());
}
