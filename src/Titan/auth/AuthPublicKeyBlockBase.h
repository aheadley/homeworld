#ifndef _AuthPublicKeyBlockBase_H
#define _AuthPublicKeyBlockBase_H

// AuthPublicKeyBlockBase

// Abstract base class for WON Auth Server Public Key Blocks.  Defines common
// functionality for AuthServer Public Key Blocks.  Note that AuthPublicKeyBlockBase
// does not override the pure virtual GetFamily() method of its base class.  This
// must be overridden in derived classes.


#include "AuthFamilyBuffer.h"


// In the WONAuth namespace
namespace WONAuth {


class AuthPublicKeyBlockBase : public AuthFamilyBuffer
{
public:
	// Default constructor - may provide block ID
	explicit AuthPublicKeyBlockBase(unsigned short theBlockId=0);

	// Copy Constructor
	AuthPublicKeyBlockBase(const AuthPublicKeyBlockBase& theBlockR);

	// Destructor
	virtual ~AuthPublicKeyBlockBase();

	// Operators
	AuthPublicKeyBlockBase& operator=(const AuthPublicKeyBlockBase& theBlockR);

	// Compare two buffers
	virtual int Compare(const AuthFamilyBuffer& theBufR) const;

	// Block ID access
	unsigned short GetBlockId() const;
	void SetBlockId(unsigned short theId);

	// Dump to stream
	virtual void Dump(std::ostream& os) const;

protected:
	unsigned short mBlockId;  // Identifier (version) of block

	// Compute size of buffer needed form pack and unpack operations.  Returns
	// size in bytes.  When derived classes override this emthod, they call the
	// base class version and add its result to their own.  See base class header
	// for more information on ComputeBufSize().
	virtual WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack local members into base raw buffer.  When derived classes override this
	// method, they must call the base class version before performing their own
	// operations.  See base class header for more information on PackData().
	virtual bool PackData();

	// Unpack local members from base raw buffer.  When derived classes override this
	// method, they must call the base class version before performing their own
	// operations.  After this method returns, mDataLen will be set to the length of
	// the header (from AuthFamilyBuffer) + the length of mBlockId.  See base class
	// header for more information on PackData().
	virtual bool UnpackData();

private:
};


// Inlines
inline unsigned short
AuthPublicKeyBlockBase::GetBlockId() const
{ return mBlockId; }

inline void
AuthPublicKeyBlockBase::SetBlockId(unsigned short theBlockId)
{ mBlockId = theBlockId; }


};  // Namespace WONAuth

#endif