#if !defined(TMsgCommRegisterRequest_H)
#define TMsgCommRegisterRequest_H

// TMsgCommRegisterRequest.h

// Message that is used to tell the server to register itself with a directory server


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgCommRegisterRequest : public TMessage {

public:
	// Default ctor
	TMsgCommRegisterRequest(void);

	// TMessage ctor
	explicit TMsgCommRegisterRequest(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommRegisterRequest(const TMsgCommRegisterRequest& theMsgR);

	// Destructor
	virtual ~TMsgCommRegisterRequest(void);

	// Assignment
	TMsgCommRegisterRequest& operator=(const TMsgCommRegisterRequest& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	const std::string&  GetDirServerAddress(void) const;
	const std::wstring& GetDisplayName(void) const;
	const std::wstring& GetRegisterDir(void) const;

	// Blob Access
	const void*    GetBinaryBlob(unsigned short* size=NULL) const;
	const void*    GetBinaryBlob(unsigned short& size) const;
	unsigned short GetBinaryBlobSize() const;

	virtual void SetDirServerAddress(const std::string& theDirServerAdress);
	virtual void SetDisplayName(const std::wstring& theDisplayName);
	virtual void SetRegisterDir(const std::wstring& theRegisterDir);
	virtual void SetBinaryBlob(const void* blob, unsigned short size);

protected:
	std::string    mDirServerAddress;
	std::wstring   mRegisterDir;
	std::wstring   mDisplayName;
	const void*    mBinaryBlob;
	unsigned short mBinaryBlobSize;
	TMessage       mSaveMsg;  // Used to save binary blob on unpack.  This allows re-pack
};


// Inlines
inline TRawMsg* TMsgCommRegisterRequest::Duplicate(void) const
{ return new TMsgCommRegisterRequest(*this); }

inline const std::string& TMsgCommRegisterRequest::GetDirServerAddress(void) const
{ return mDirServerAddress; }

inline const std::wstring& TMsgCommRegisterRequest::GetDisplayName(void) const
{ return mDisplayName; }

inline const std::wstring& TMsgCommRegisterRequest::GetRegisterDir(void) const
{ return mRegisterDir; }

inline const void* TMsgCommRegisterRequest::GetBinaryBlob(unsigned short* size) const
{ if (size) *size = mBinaryBlobSize; return mBinaryBlob; }

inline const void* TMsgCommRegisterRequest::GetBinaryBlob(unsigned short& size) const
{ size = mBinaryBlobSize; return mBinaryBlob; }

inline unsigned short TMsgCommRegisterRequest::GetBinaryBlobSize() const
{ return mBinaryBlobSize; }

inline void TMsgCommRegisterRequest::SetDirServerAddress(const std::string& theDirServerAddress)
{ mDirServerAddress = theDirServerAddress; }

inline void TMsgCommRegisterRequest::SetDisplayName(const std::wstring& theDisplayName)
{ mDisplayName = theDisplayName; }

inline void TMsgCommRegisterRequest::SetRegisterDir(const std::wstring& theRegisterDir)
{ mRegisterDir = theRegisterDir; }

inline void TMsgCommRegisterRequest::SetBinaryBlob(const void* blob, unsigned short size)
{ mBinaryBlob = blob; mBinaryBlobSize = size; }


};  // Namespace WONMsg

#endif