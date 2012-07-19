/******************************************************************************/
/*                                                                            */
/*     !  N O T I C E  !  N O T I C E  !  N O T I C E  !  N O T I C E  !      */
/*                                                                            */
/*             ©1998 Sierra On-Line, Inc.  All Rights Reserved.               */
/*                     U.S. and foreign patents pending.                      */
/*                                                                            */
/*                          THIS SOFTWARE BELONGS TO                          */
/*                            Sierra On-Line, Inc.                            */
/*     IT IS CONSIDERED A TRADE SECRET AND IS NOT TO BE DIVULGED OR USED      */
/*        BY PARTIES WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM         */
/*                            Sierra On-Line, Inc.                            */
/*                       3380 146th Place SE, Suite 300                       */
/*                          Bellevue, WA  98007-6472                          */
/*                                206 649-9800                                */
/*                                                                            */
/*              Federal law provides severe civil penalties for               */
/*                   misuse or violation of trade secrets.                    */
/*                                                                            */
/******************************************************************************/
// TMessage.h: 
//
//////////////////////////////////////////////////////////////////////
// 2/12/98 Greg Hoglund,	adding new message classes, TRaw, TData, and TMessage
//							including reference counting 

#ifndef H_TMessage
#define H_TMessage

#include "common/won.h"
#include "msg/HeaderTypes.h"

namespace WONMsg {

// Message class types
enum MessageClass {
	eTMessage, eMiniMessage, eSmallMessage, eLargeMessage
};


// TRawData
// Stores a contiguous buffer and provides methods for appending and removing
// data from this buffer.  TRawData is nested within TRawMsg and is not meant
// to be used alone.  Access to this class is via TRawMsg.
// --------------------------------------------------------------------------
class TRawData
{
private:

	friend class TRawMsg;

	// *NOTE* Interlocked operations are used on this member, and it must be
	// 32-bit aligned.  Change the byte alignment of the compiler at your own risk!
	long mRefCount;   // reference count

	int                    mChunkMult;  // Chunking multiplier
	unsigned char*         mDataP;      // Data Buffer
	unsigned long          mDataLen;    // Data buffer length (total)
	mutable unsigned char* mReadPtrP;   // Buffer read pointer
	unsigned char*         mWritePtrP;  // Buffer write pointer
	mutable string         mStrBuf;     // Buffer for Read string
	mutable wstring        mWStrBuf;    // Buffer for read WString

	TRawData(const TRawData& theMsgR);  //Only TRawMsg can copy

	// Validate length before a read
	// May throw BadMsgException
	void CheckLength(unsigned long theLen) const;

	//Disable assignment
	TRawData& operator =(const TRawData& theMsgR);  //prevent assignment

public:

	TRawData();
	explicit TRawData(unsigned long theDataLen, const void *theData=NULL);
	~TRawData();

	void *	AllocateNewBuffer(unsigned long theSize);
	void *	AddAllocate(unsigned long nNewBytes, bool chunkAlloc, bool doDumbCheck = true);

	unsigned long GetDataLen() const;
	unsigned long GetBufferLen() const { return mDataLen; }

	void	AppendLong(unsigned long theLong);
	void	AppendShort(unsigned short theShort);
	void	AppendByte(unsigned char theByte);
	void	AppendBytes(long theNumBytes, const void* theBytesP, bool chunkAlloc = true, bool doDumbCheck = true);
	void	Append_PW_STRING(const wstring& theString);
	void	Append_PA_STRING(const string& theString);
 

	void	ResetBuffer(void);
	void	ResetReadPointer(void) const { mReadPtrP = mDataP; }
	void	ResetWritePointer(void) { mWritePtrP = mDataP; }

	unsigned long BytesLeftToRead(void) const;


	void  SkipWritePtrAhead(unsigned long theNumBytes);

	// The following two methods return ref to internal buffer that is overwritten
	// on each call to that method
	// These two methods are OBSOLETE, use ReadString and ReadWString instead!
	const wstring& Read_PW_STRING() const;
	const string&  Read_PA_STRING() const;

	void           ReadWString(std::wstring& theBufR) const;
	void           ReadString(std::string& theBufR) const;
	unsigned long  ReadLong() const;
	unsigned short ReadShort() const;
	unsigned char  ReadByte() const;
	const void*    ReadBytes(long theNumBytes) const;
    const void*    PeekBytes(long theNumBytes) const;
};


// TRawMsg
// Provides access to a contiguous buffer in memory that represents 
// a data packet.  The actual data is referenced.  Use the Duplicate() method
// to create an exclusive copy of the data.
// Pack() and Unpack() are meant to access the memory buffer and append or remove
// data fields as needed.  Override and implement as needed.
// ------------------------------------------------------------------------------
class TRawMsg 
{
public:
	TRawMsg();
	TRawMsg(unsigned long theDataLen, const void *theData=NULL);
	TRawMsg(const TRawMsg& Rmsg);

	virtual ~TRawMsg();

	TRawMsg& operator= (const TRawMsg&);

	virtual TRawMsg * Duplicate() const;

	virtual void Unpack();
	virtual void * Pack();

	const void * GetDataPtr() const;
	void *       GetDataPtr();

	const void * GetWritePtr() const { return rep->mWritePtrP; }
	void *			 GetWritePtr() { return rep->mWritePtrP; }


	unsigned long GetDataLen() const { return rep->GetDataLen(); }
	unsigned long GetBufferLen() const { return rep->GetBufferLen(); }

	// Buffering methods

	void *	AllocateNewBuffer(unsigned long theSize)
		{
			CopyOnWrite();
			return rep->AllocateNewBuffer(theSize);
		}

	void *	AddAllocate(unsigned long nNewBytes)
		{ 
			CopyOnWrite();
			return rep->AddAllocate(nNewBytes, false); 
		}

	void	AppendLong(unsigned long theLong)
		{
			CopyOnWrite();
			rep->AppendLong(theLong);
		}

	void	AppendShort(unsigned short theShort)
		{
			CopyOnWrite();
			rep->AppendShort(theShort);
		}

	void	AppendByte(unsigned char theByte)
		{
			CopyOnWrite();
			rep->AppendByte(theByte);
		}

	void	AppendBytes(long theNumBytes, const void* theBytesP, bool chunkAlloc = true, bool doDumbCheck = true)
		{
			CopyOnWrite();
			rep->AppendBytes(theNumBytes, theBytesP, chunkAlloc, doDumbCheck);
		}

	void	Append_PW_STRING(const wstring& theString)
		{ 
			CopyOnWrite();
			rep->Append_PW_STRING(theString); 
		}

	void	Append_PA_STRING(const string& theString)
		{ 
			CopyOnWrite();
			rep->Append_PA_STRING(theString);
		}

	void	ResetBuffer(void)
		{ 
			CopyOnWrite();
			rep->ResetBuffer(); 
		}

	void  SkipWritePtrAhead(unsigned long theNumBytes) { rep->SkipWritePtrAhead(theNumBytes); }

	void  ResetReadPointer(void) const { rep->ResetReadPointer(); }
	void  ResetWritePointer(void) { rep->ResetWritePointer(); }

	unsigned long BytesLeftToRead(void) const { return rep->BytesLeftToRead(); }

	// Read data, all these methods may throw
	// The following two methods return ref to internal buffer that is overwritten
	// on each call to that method
	// These two methods are OBSOLETE, use ReadString and ReadWString instead!
	const wstring& Read_PW_STRING() const { return rep->Read_PW_STRING(); }
	const string&  Read_PA_STRING() const { return rep->Read_PA_STRING(); }

	void           ReadWString(std::wstring& theBufR) const { rep->ReadWString(theBufR); }
	void           ReadString(std::string& theBufR) const { rep->ReadString(theBufR); }
	unsigned long  ReadLong() const { return rep->ReadLong(); }
	unsigned short ReadShort() const { return rep->ReadShort(); }
	unsigned char  ReadByte() const { return rep->ReadByte(); }
	const void *   ReadBytes(long theNumBytes) const { return rep->ReadBytes(theNumBytes); }
	const void *   PeekBytes(long theNumBytes) const { return rep->PeekBytes(theNumBytes); }

	// Compute size required to pack strings and wstrings
	static unsigned long ComputeStringPackSize (const std::string& theStrR)
		{ return (sizeof(unsigned short) + theStrR.size()); }

	static unsigned long ComputeWStringPackSize(const std::wstring& theStrR)
		{ return (sizeof(unsigned short) + theStrR.size() * sizeof(wchar_t)); }

private:
	TRawData *rep;  // ref counted member

	// Duplciate rep if needed when write operations occur.
	void CopyOnWrite();
};


// ** Begin message header definitions **

// *NOTE*
// No padding of structs for message headers.  Padding screws up size and
// pack/unpack alogirthms
#pragma pack (push, 1)

// Old-style header for TMessages (no header type)
// **OBSOLETE**
struct TitanHeader
{
	unsigned long _ServiceType;
	unsigned long _MessageType;

	explicit TitanHeader(unsigned long theServType=0, unsigned long theMsgType=0) :
		_ServiceType(theServType), _MessageType(theMsgType) {}

    // Cast helper function
    static TitanHeader &Cast( void *thePtr )
    {
        return *(static_cast<TitanHeader*>( thePtr ) );
    }
    
    static const TitanHeader &Cast( const void *thePtr )
    {
        return *(static_cast<const TitanHeader*>( thePtr ) );
    }

};

// MiniMessage header
struct MiniHeader {
	unsigned char  _HeaderType;
	unsigned char  _ServiceType;
	unsigned char  _MessageType;

	explicit MiniHeader(unsigned char theServType=0, unsigned char theMsgType=0) :
		_HeaderType(HeaderService1Message1), _ServiceType(theServType), _MessageType(theMsgType) 
	{}
};

// SmallMessage header
struct SmallHeader {
	unsigned char  _HeaderType;
	unsigned short _ServiceType;
	unsigned short _MessageType;

	explicit SmallHeader(unsigned short theServType=0, unsigned short theMsgType=0) :
		_HeaderType(HeaderService2Message2), _ServiceType(theServType), _MessageType(theMsgType) 
	{}
};

// LargeMessage header
struct LargeHeader {
	unsigned char _HeaderType;
	unsigned long _ServiceType;
	unsigned long _MessageType;

	explicit LargeHeader(unsigned long theServType=0, unsigned long theMsgType=0) :
		_HeaderType(HeaderService4Message4), _ServiceType(theServType), _MessageType(theMsgType) 
	{}
};

// Restore previous packing level
#pragma pack (pop)

// ** End message header definitions **


// BaseMessage
// Base class for all Titan message formats.  The Pack() and Unpack() methods will 
// build a correct message packet in memory.  If the object cannot build a packet 
// correctly, an exception should be thrown.
// ---------------------------------------------------------------------------
class BaseMessage : public TRawMsg
{
public:
	BaseMessage();
	BaseMessage(const BaseMessage& msg);
	explicit BaseMessage(MessageClass theMessageClass) : mMessageClass(theMessageClass) {};
	BaseMessage(unsigned long theDataLen, const void *theData =NULL);
	virtual ~BaseMessage();

	BaseMessage& operator=(const BaseMessage&);

	virtual TRawMsg* Duplicate() const =0;

	virtual void* Pack() =0; // May throw
	virtual void  Unpack();  // May throw
	virtual void  UnpackHeader() const;

	virtual void* AllocateHeaderBuffer() { return AllocateNewBuffer(GetHeaderLength());	}
	virtual void* AllocateBodyBuffer(unsigned long size); // May throw

	virtual const void* GetBodyPtr() const { return( static_cast<const void*>(static_cast<const char*>(GetDataPtr()) + GetHeaderLength()) ); }
	virtual void*       GetBodyPtr()       { return( static_cast<void*>(static_cast<char*>(GetDataPtr()) + GetHeaderLength()) ); }

	virtual unsigned long GetHeaderLength() const =0;
	virtual unsigned char GetHeaderType() const =0;
	virtual unsigned char GetClearHeaderType() const =0;
	virtual unsigned char GetEncryptedHeaderType() const =0;
	virtual unsigned long GetServiceType(void) const =0;
	virtual unsigned long GetMessageType(void) const =0;
	virtual void SetServiceType(unsigned long theServiceType) =0;
	virtual void SetMessageType(unsigned long theMessageType) =0;

	MessageClass GetMessageClass() const { return mMessageClass; }

	virtual void Dump(std::ostream& os) const {};
	
	// Class Constants
	enum { MAXMSG_SIZE = 131068 }; // Maximum message size
protected:
	MessageClass mMessageClass;
};


class MiniMessage : public BaseMessage {
public:
	MiniMessage();
	MiniMessage(const MiniMessage& theRSMsg);
	MiniMessage(unsigned long theDataLen, const void *theData =NULL);
	virtual ~MiniMessage();

	MiniMessage& operator=(const MiniMessage&);

	virtual TRawMsg* Duplicate() const;

	virtual void* Pack(); // May throw

	const MiniHeader& GetHeader() const       { return *(static_cast<const MiniHeader*>(GetDataPtr())); }
	unsigned long     GetHeaderLength() const { return sizeof(MiniHeader); }

	unsigned char GetHeaderType(void) const    { return GetHeader()._HeaderType;    }
	unsigned char GetClearHeaderType() const   { return HeaderService1Message1; }
	unsigned char GetEncryptedHeaderType() const { return MiniEncryptedService; }
	unsigned long GetServiceType(void) const   { return GetHeader()._ServiceType;   }
	unsigned long GetMessageType(void) const   { return GetHeader()._MessageType;   }
	void SetServiceType(unsigned long theServiceType) { GetHeaderRef()._ServiceType = theServiceType; }
	void SetMessageType(unsigned long theMessageType) { GetHeaderRef()._MessageType = theMessageType; }

	virtual void Dump(std::ostream& os) const;

private:
	void SetHeaderType(unsigned char theHeaderType)   { GetHeaderRef()._HeaderType = theHeaderType;  }
	// Non-const version of getheader for set methods
	// MUST RETURN A NON-CONST REF!!!
	MiniHeader& GetHeaderRef() { return *(static_cast<MiniHeader*>(GetDataPtr())); }
};


class SmallMessage : public BaseMessage {
public:
	SmallMessage();
	SmallMessage(const SmallMessage& theRSMsg);
	SmallMessage(unsigned long theDataLen, const void *theData =NULL);
	virtual ~SmallMessage();

	SmallMessage& operator=(const SmallMessage&);

	virtual TRawMsg* Duplicate() const;

	virtual void* Pack(); // May throw

	const SmallHeader& GetHeader() const       { return *(static_cast<const SmallHeader*>(GetDataPtr())); }
	unsigned long      GetHeaderLength() const { return sizeof(SmallHeader); }

	unsigned char GetHeaderType(void) const    { return GetHeader()._HeaderType;    }
	unsigned char GetClearHeaderType() const   { return HeaderService2Message2; }
	unsigned char GetEncryptedHeaderType() const { return SmallEncryptedService; }
	unsigned long GetServiceType(void) const   { return GetHeader()._ServiceType;   }
	unsigned long GetMessageType(void) const   { return GetHeader()._MessageType;   }
	void SetServiceType(unsigned long theServiceType) { GetHeaderRef()._ServiceType = theServiceType; }
	void SetMessageType(unsigned long theMessageType) { GetHeaderRef()._MessageType = theMessageType; }

	virtual void Dump(std::ostream& os) const;

private:
	void SetHeaderType(unsigned char theHeaderType)   { GetHeaderRef()._HeaderType = theHeaderType;  }
	// Non-const version of getheader for set methods
	// MUST RETURN A NON-CONST REF!!!
	SmallHeader& GetHeaderRef() { return *(static_cast<SmallHeader*>(GetDataPtr())); }
};


class LargeMessage : public BaseMessage {
public:
	LargeMessage();
	LargeMessage(const LargeMessage& theRSMsg);
	LargeMessage(unsigned long theDataLen, const void *theData =NULL);
	virtual ~LargeMessage();

	LargeMessage& operator=(const LargeMessage&);

	virtual TRawMsg* Duplicate() const;

	virtual void* Pack(); // May throw

	const LargeHeader& GetHeader() const       { return *(static_cast<const LargeHeader*>(GetDataPtr())); }
	unsigned long      GetHeaderLength() const { return sizeof(LargeHeader); }

	unsigned char GetHeaderType(void) const    { return GetHeader()._HeaderType;    }
	unsigned char GetClearHeaderType() const   { return HeaderService4Message4; }
	unsigned char GetEncryptedHeaderType() const { return LargeEncryptedService; }
	unsigned long GetServiceType(void) const   { return GetHeader()._ServiceType;   }
	unsigned long GetMessageType(void) const   { return GetHeader()._MessageType;   }
	void SetServiceType(unsigned long theServiceType) { GetHeaderRef()._ServiceType = theServiceType; }
	void SetMessageType(unsigned long theMessageType) { GetHeaderRef()._MessageType = theMessageType; }

	virtual void Dump(std::ostream& os) const;

private:
	void SetHeaderType(unsigned char theHeaderType)   { GetHeaderRef()._HeaderType = theHeaderType;  }
	// Non-const version of getheader for set methods
	// MUST RETURN A NON-CONST REF!!!
	LargeHeader& GetHeaderRef() { return *(static_cast<LargeHeader*>(GetDataPtr())); }
};


// **OBSOLETE**
// TMessage - old style titan messages without header type field
// ---------------------------------------------------------------------------
class TMessage : public BaseMessage {
public:
	TMessage();
	TMessage(const TMessage& Tmsg);
	explicit TMessage(unsigned long theDataLen, const void *theData=NULL);
	virtual ~TMessage();

	TMessage& operator= (const TMessage&);

	virtual TRawMsg * Duplicate() const;

	virtual void * Pack();          // May throw

	// inlined
	const TitanHeader& GetHeader() const { return *(static_cast<const TitanHeader*>(GetDataPtr())); }
	unsigned long      GetHeaderLength() const { return sizeof(TitanHeader); }

	unsigned char GetHeaderType(void) const    { return (unsigned char)GetServiceType(); }
	unsigned char GetClearHeaderType() const   { return GetHeaderType(); }
	unsigned char GetEncryptedHeaderType() const { return EncryptedService; }
	unsigned long GetServiceType(void) const   { return GetHeader()._ServiceType;   }
	unsigned long GetMessageType(void) const   { return GetHeader()._MessageType;   }
	void SetServiceType(unsigned long theServiceType ) { GetHeaderRef()._ServiceType   = theServiceType; }
	void SetMessageType(unsigned long theMessageType ) { GetHeaderRef()._MessageType   = theMessageType; }

	virtual void Dump(std::ostream& os) const;
private:
	// Non-const version of getheader for set methods
	// MUST RETURN A NON-CONST REF!!!
	TitanHeader& GetHeaderRef() { return *(static_cast<TitanHeader*>(GetDataPtr())); }
};
// **END OBSOLETE**


};  // namespace WONMsg

inline std::ostream& 
operator<<(std::ostream& os, const WONMsg::BaseMessage& theMsgR)
{ theMsgR.Dump(os); return os; }

#endif // H_TMessage

