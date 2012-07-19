#ifndef _TMsgAuthLastRawBufferBase_H
#define _TMsgAuthLastRawBufferBase_H

// TMsgAuthRawBufferBase.h

// Base class for AuthServer that adds a raw (binary) buffer of arbitrary length
// to TMessage.  This class is not a AuthServer message itself.  It provides methods
// fetch, set, pack and unpack a the binary buffer.

// Note that this message does not implement Pack and Unpack as the raw buffer
// may appended anywhere within an AuthServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"

namespace WONMsg {

class TMsgAuthLastRawBufferBase : public TMessage
{
public:
	// Default ctor
	TMsgAuthLastRawBufferBase();

	// TMessage ctor
	explicit TMsgAuthLastRawBufferBase(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuthLastRawBufferBase(const TMsgAuthLastRawBufferBase& theMsgR);

	// Destructor
	virtual ~TMsgAuthLastRawBufferBase();

	// Assignment
	TMsgAuthLastRawBufferBase& operator=(const TMsgAuthLastRawBufferBase& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate() const;

	// Force copy of raw buf if needed.
	void ForceRawBufOwn();

	// Buffer access
	const unsigned char* GetRawBuf() const;
	unsigned short       GetRawBufLen() const;

	// Update buffer.  Setting copyBuf to false will cause the specified
	// theBlockP pointer to be stored without copying its contents.  This will
	// improve performance, but theBlockP MUST NOT BE DEALLOCATED while in use
	// by this class.
	void SetRawBuf(const unsigned char* theRawP, unsigned short theLen,
	               bool copyBuf=false);

protected:
	const unsigned char* mRawP;    // Raw data (binary, possibly encrypted)
	unsigned short       mRawLen;  // Length of raw data
	WONCommon::RawBuffer mRawBuf;  // Buffer for raw data when owned

	// Pack raw buffer.  Appends 2 byte length followed by bytes equal to length.
	void PackRawBuf();

	// Unpack raw buffer.  Reads 2 byte length followed by bytes equal to length.
	// Setting copyBuf to false, will cause no copy; mrawP will point into the
	// binary rep of TMessage.  This is ok UNLESS Pack() is called again!  In this
	// case, RawBuf MUST BE UNPACKED with copyBuf set to true!
	void UnpackRawBuf(bool copyBuf=false);
};


// Inlines
inline TRawMsg*
TMsgAuthLastRawBufferBase::Duplicate(void) const
{ return new TMsgAuthLastRawBufferBase(*this); }

inline void
TMsgAuthLastRawBufferBase::ForceRawBufOwn(void)
{
	if ((mRawP) && (mRawP != mRawBuf.data()))
		{  mRawBuf.assign(mRawP, mRawLen);  mRawP = mRawBuf.data();  } 
}

inline const unsigned char*
TMsgAuthLastRawBufferBase::GetRawBuf() const
{ return mRawP; }

inline unsigned short
TMsgAuthLastRawBufferBase::GetRawBufLen() const
{ return mRawLen; }


};  // Namespace WONMsg

#endif