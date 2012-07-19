#ifndef _AuthFamilyBuffer_H
#define _AuthFamilyBuffer_H

// AuthFamilyBuffer

// Abstract base class for a binary buffer for Auth Family data.  Buffer is of
// arbitrary length with the following header:
//		Auth Family ID   (ushort - 2 bytes)
//		Issue Date/Time  (time_t - 4 bytes)
//		Expire Date/Time (time_t - 4 bytes)
// The ID identifies the family of the buffer and the Issue/Expire date/time
// identify the lifespan of the buffer.

// AuthFamilyBuffer supports copy, assignment, a full range of logical operators,
// and the stream output operator.  Dervied classes will get the logical and stream
// output operators for free if they override the Compare() and Dump() methods.
// Also note that AuthFamilyBuffers are suitable to be aused in STL containers.

// AuthFamilyBuffer has serveral pure virtual methods that must be overridden by
// derived classes.


#include "../common/won.h"
#include <ostream>
#include <time.h>

// Forwards
namespace WONCrypt {
	class PrivateKey;
	class PublicKey;
};

// In the WONAuth namespace
namespace WONAuth {


class AuthFamilyBuffer
{
public:
	// Default constructor
	AuthFamilyBuffer();

	// Copy Constructor
	AuthFamilyBuffer(const AuthFamilyBuffer& theBufR);

	// Destructor
	virtual ~AuthFamilyBuffer();

	// Operators
	AuthFamilyBuffer& operator=(const AuthFamilyBuffer& theBufR);
	bool operator==(const AuthFamilyBuffer& theBufR) const;
	bool operator!=(const AuthFamilyBuffer& theBufR) const;
	bool operator< (const AuthFamilyBuffer& theBufR) const;
	bool operator<=(const AuthFamilyBuffer& theBufR) const;
	bool operator> (const AuthFamilyBuffer& theBufR) const;
	bool operator>=(const AuthFamilyBuffer& theBufR) const;

	// Check validity.  Returns true if last call to Pack/Unpack was successful,
	// otherwise false.  Will return false if Pack/Unpack have not been called.
	bool IsValid() const;

	// Compare two buffers
	virtual int Compare(const AuthFamilyBuffer& theBufR) const;

	// Pack into raw form.  Must specifiy key for signature generation.  Returns
	// true on success, false on failure.  Default implementation calls PackData(),
	// and calls Sign().
	bool Pack(const WONCrypt::PrivateKey& theKeyR);

	// Unpack from raw form.  Returns true on success, false on failure.  Default
	// implementation validates family, fills in mRawBuf, reads issueTime/expireTime,
	// and calls UnpackData().
	bool Unpack(const unsigned char* theRawP, unsigned short theLen);

	// Verifies signature using specified public key.  mRawBuf must have the data and
	// sig within it already (pack/unpack have already been called).  Returns true on
	// success, false on failure.
	virtual bool Verify(const WONCrypt::PublicKey& theKeyR) const;

	// Invalidate the buffer
	void Invalidate();

	// Fetch Auth Family ID
	// Pure virtual, must be overridden.
	virtual unsigned short GetFamily() const = 0;

	// Lifespan access
	time_t        GetIssueTime() const;
	time_t        GetExpireTime() const;
	unsigned long GetLifespan() const;

	// Check for exiration.  Add delta to current time and check against
	// expire time.
	bool IsExpired(long theDelta=0) const;

	// Lifespan update
	void SetLifespan(time_t theIssueTime, unsigned long theLifespan);
	void SetIssueTime(time_t theTime);
	void SetExpireTime(time_t theTime);

	// Fetch raw buffer.  Return NULL/0 if IsValid is false
	const unsigned char* GetRaw() const;
	unsigned short       GetRawLen() const;

	// Fetch data or sig portion of buffer.  Both will return NULL/0
	// if IsValid() is false.
	const unsigned char* GetData() const;       // Return data block only
	unsigned short       GetDataLen() const;
	const unsigned char* GetSignature() const;  // Return sig block only
	unsigned short       GetSignatureLen() const;

	// Dump to stream
	virtual void Dump(std::ostream& os) const;

protected:
	// Types
	enum SizeComputeMode { PACK, UNPACK };

	// Members
	WONCommon::RawBuffer mRawBuf;      // Binary buffer
	time_t               mIssueTime;   // Date/time buffer was created
	time_t               mExpireTime;  // Date/time buffer will expire
	unsigned short       mDataLen;     // Length of data portion of buffer (all but sig)

	// Compute needed size of buffer for a pack or unpack operation.  Mode parameter
	// defines whether pack or unpack is is process.  Returns the min number of bytes
	// needed in the buffer.  Dervied classes should override this method first
	// call base class method, then add their needed bytes to that total.  Mode is
	// used to handle variable length data.  In 'PACK' mode, the total length
	// (including variable length data) should be returned.  This allows pack()
	// to preallocate a buffer.  In 'UNPACK' mode, the minimumn length (length
	// without variable length data) should be returned.  This allows Unpack() to
	// verify a minimum raw buffer size before proceeding.
	virtual WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack data portion into mRawBuf.  Return true on success, false on failure.
	// The mRawBuf will contain the 2 byte family, 4 byte issueTime, and 4 byte
	// expireTime when this method is called.  mDataLen will be set to the size of
	// the header data in mRawBuf.  This method should append to mRawBuf.
	// Pure virtual.  Must be overridden
	virtual bool PackData() = 0;

	// Unpack data portion into mRawBuf.  Return true on success, false on failure.
	// mRawBuf will be filled in when this method is called.  Note that mRawBuf
	// will start with a 2 byte family, 4 byte issueTime, and 4 byte expireTime.
	// IMPORTANT: This method MUST FILL IN mDataLen so start of sig can be found!
	// mDataLen will be set to the header length (10) when this method is called.
	// UnpackData() should add the length of its data to mDatalen.
	// Pure virtual.  Must be overridden
	virtual bool UnpackData() = 0;

	// Generate sig for current contents of mRawBuf.  Appends sig to mRawBuf and fills
	// in mDataLen.  Returns true on success, false on failure.
	virtual bool Sign(const WONCrypt::PrivateKey& theKeyR);

private:
};


// Inlines
inline bool
AuthFamilyBuffer::operator==(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) == 0); }

inline bool
AuthFamilyBuffer::operator!=(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) != 0); }

inline bool
AuthFamilyBuffer::operator<(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) < 0); }

inline bool
AuthFamilyBuffer::operator<=(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) <= 0); }

inline bool
AuthFamilyBuffer::operator>(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) > 0); }

inline bool
AuthFamilyBuffer::operator>=(const AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) >= 0); }

inline bool
AuthFamilyBuffer::IsValid() const
{ return (! mRawBuf.empty()); }

inline void
AuthFamilyBuffer::Invalidate()
{ mRawBuf.erase();  mDataLen = 0; }

inline time_t
AuthFamilyBuffer::GetIssueTime() const
{ return mIssueTime; }

inline time_t
AuthFamilyBuffer::GetExpireTime() const
{ return mExpireTime; }

inline unsigned long
AuthFamilyBuffer::GetLifespan() const
{ return (mExpireTime - mIssueTime); }

inline bool
AuthFamilyBuffer::IsExpired(long theDelta) const
{ return ((time(NULL) + theDelta) > mExpireTime); }

inline void
AuthFamilyBuffer::SetLifespan(time_t theIssueTime, unsigned long theLifespan)
{ Invalidate();  mIssueTime = mExpireTime = theIssueTime;  mExpireTime += theLifespan; }

inline void
AuthFamilyBuffer::SetIssueTime(time_t theTime)
{ Invalidate();  mIssueTime = theTime; }

inline void
AuthFamilyBuffer::SetExpireTime(time_t theTime)
{ Invalidate();  mExpireTime = theTime; }

inline const unsigned char*
AuthFamilyBuffer::GetRaw() const
{ return (IsValid() ? mRawBuf.c_str() : NULL); }

inline unsigned short
AuthFamilyBuffer::GetRawLen() const
{ return mRawBuf.size(); }

inline const unsigned char*
AuthFamilyBuffer::GetData() const
{ return (IsValid() ? mRawBuf.c_str() : NULL); }

inline unsigned short
AuthFamilyBuffer::GetDataLen() const
{ return (IsValid() ? mDataLen : 0); }

inline const unsigned char*
AuthFamilyBuffer::GetSignature() const
{ return (IsValid() ? (mRawBuf.c_str() + mDataLen) : NULL); }

inline unsigned short
AuthFamilyBuffer::GetSignatureLen() const
{ return (IsValid() ? (mRawBuf.size() - mDataLen) : 0); }

};  // Namespace WONAuth


// Output operator
inline std::ostream&
operator<<(std::ostream& os, const WONAuth::AuthFamilyBuffer& theBufR)
{ theBufR.Dump(os);  return os; }

#endif