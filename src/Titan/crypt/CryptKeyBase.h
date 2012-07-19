#ifndef _CRYPTKEYBASE_H
#define _CRYPTKEYBASE_H

// CryptKeyBase

// Base class for keys within the WONCrypt library.  CryptKeyBase is an abstract base
// class.  It has a pure vitural method (Create) to be overridden.


#include <ostream>

// In WONCrypt namespace
namespace WONCrypt
{

// Forwards
class CryptException;

class CryptKeyBase
{
public:
	// Class constants
	enum {
		KEYLEN_DEF = 8  // Default key length in bytes
	};

	// Types
	// CryptReturn - allocated binary data and length of data
	typedef std::pair<unsigned char*, unsigned long> CryptReturn;

	// Constructors
	CryptKeyBase();
	CryptKeyBase(const CryptKeyBase& theKeyR);

	// Destructor
	virtual ~CryptKeyBase();

	// Operators
	CryptKeyBase& operator=(const CryptKeyBase& theKeyR);
	bool operator==(const CryptKeyBase& theKeyR) const;
	bool operator!=(const CryptKeyBase& theKeyR) const;
	bool operator< (const CryptKeyBase& theKeyR) const;  // For STL containers

	// Fetch raw key
	const unsigned char* GetKey() const;
	unsigned short       GetKeyLen() const;

	// Invalidate the key
	void Invalidate();

	// Check validity
	virtual bool IsValid() const;

	// Create key - from existing key.    Default behavior will delete mKey, allocate
	// new mKey of theLen bytes, and memcpy theKeyP.
	virtual void Create(unsigned short theLen, const unsigned char* theKeyP) throw(CryptException);

	// Create key - new key of specified length.  Specifying theLen a 0 will use
	// the default length.
	// ** Pure virtual, must be overridden **
	virtual void Create(unsigned short theLen=0) throw(CryptException) = 0;

	// Dump key to stream.  Default behavior outputs length and key (in hex)
	virtual void Dump(std::ostream& os) const;

protected:
	unsigned char* mKey;     // Key value (array)
	unsigned short mKeyLen;  // Length of key in bytes

	// Allocate a new key buffer.  Deletes old buffer.  Sets mKeyLen to theLen and
	// allocates mKey.
	void AllocKeyBuf(unsigned short theLen);

private:
};


// Inlines
inline bool
CryptKeyBase::operator!=(const CryptKeyBase& theKeyR) const
{ return (! operator==(theKeyR)); }

inline void
CryptKeyBase::Invalidate()
{ AllocKeyBuf(1);  *mKey = 0; }

inline const unsigned char*
CryptKeyBase::GetKey() const
{ return mKey; }

inline unsigned short
CryptKeyBase::GetKeyLen() const
{ return mKeyLen; }

inline void
CryptKeyBase::AllocKeyBuf(unsigned short theLen)
{ mKeyLen = theLen;  delete [] mKey;  mKey = new unsigned char [mKeyLen]; }


};  //namespace WONCrypt


// Output operators
inline std::ostream&
operator<<(std::ostream& os, const WONCrypt::CryptKeyBase& theKey)
{ theKey.Dump(os);  return os; }

#endif