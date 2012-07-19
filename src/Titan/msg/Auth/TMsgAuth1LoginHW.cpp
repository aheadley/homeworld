// TMsgAuth1LoginHW.h

// AuthServer messages implementing the Auth1 Family login protocol.
// This header/source implements messages:
//	Auth1LoginRequest
//	Auth1NewLoginRequest
// It also includes the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequest begins login for an existing user and is sent from the Client to
// the AuthServer.

// Auth1NewLoginRequest begins login for a new user and is sent from the Client to
// the AuthServer.

// Auth1LoginReply is sent in response to both Auth1LoginRequest and
// Auth1NewLoginRequest from the AuthServer to the client and completes the login.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1LoginHW.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgAuth1LoginRequestHW;
	using WONMsg::TMsgAuth1ChallengeHW;
	using WONMsg::TMsgAuth1ConfirmHW;
	using WONMsg::TMsgAuth1RefreshHW;
};


// ** TMsgAuth1LoginRequestHW **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginRequestHW::TMsgAuth1LoginRequestHW(void) :
	TMsgAuth1LoginBase2(), mNeedKeyFlg(false), mCreateAcctFlg(false), mKeyBuf(NULL), mDataBuf(NULL)
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginRequestHW);
}


// TMessage ctor
TMsgAuth1LoginRequestHW::TMsgAuth1LoginRequestHW(const TMessage& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR), mKeyBuf(NULL), mDataBuf(NULL)
{
	Unpack();
}


// Copy ctor
TMsgAuth1LoginRequestHW::TMsgAuth1LoginRequestHW(const TMsgAuth1LoginRequestHW& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR), mNeedKeyFlg(theMsgR.mNeedKeyFlg), mCreateAcctFlg(theMsgR.mCreateAcctFlg),
		mUserName(theMsgR.mUserName), mCommunityName(theMsgR.mCommunityName), mNicknameKey(theMsgR.mNicknameKey),
		mPassword(theMsgR.mPassword), mNewPassword(theMsgR.mNewPassword), mCDKey(theMsgR.mCDKey),
		mLoginKey(theMsgR.mLoginKey), mKeyBuf(NULL), mDataBuf(NULL)
{
}


// Destructor
TMsgAuth1LoginRequestHW::~TMsgAuth1LoginRequestHW(void)
{
	delete [] mKeyBuf;
	delete [] mDataBuf;
}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginRequestHW&
TMsgAuth1LoginRequestHW::operator=(const TMsgAuth1LoginRequestHW& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
	    TMsgAuth1LoginBase2::operator=(theMsgR);

			delete [] mKeyBuf; mKeyBuf = NULL;
			delete [] mDataBuf; mDataBuf = NULL;

			mNeedKeyFlg = theMsgR.mNeedKeyFlg;
			mCreateAcctFlg = theMsgR.mCreateAcctFlg;
			mUserName = theMsgR.mUserName;
			mCommunityName = theMsgR.mCommunityName;
			mNicknameKey = theMsgR.mNicknameKey;
			mPassword = theMsgR.mPassword;
			mNewPassword = theMsgR.mNewPassword;
			mCDKey = theMsgR.mCDKey;
			mLoginKey = theMsgR.mLoginKey;
	}
	return *this;
}

// Build the raw encrypted buffers from class data members
bool TMsgAuth1LoginRequestHW::BuildBuffer(const WONAuth::Auth1PublicKeyBlock &thePubKeyBlock, 
																					const WONCrypt::BFSymmetricKey &theSessionKey)
{
	SetRawKeyBuf(NULL,0,false);
	SetRawDataBuf(NULL,0,false);
	delete [] mKeyBuf; mKeyBuf = NULL;
	delete [] mDataBuf; mDataBuf = NULL;

	try {
		if(!thePubKeyBlock.IsValid()) 
			return false;

		SetKeyBlockId(thePubKeyBlock.GetBlockId());
		WONCrypt::CryptKeyBase::CryptReturn aCryptRet(NULL,0);
		
		aCryptRet = thePubKeyBlock.EncryptRawBuffer(theSessionKey.GetKey(),theSessionKey.GetKeyLen());
		if(aCryptRet.first==NULL) 
			return false;

		mKeyBuf = aCryptRet.first;
		SetRawKeyBuf(aCryptRet.first,aCryptRet.second,false);
		aCryptRet.first = NULL;

		TRawMsg aBuf;
		aBuf.AppendShort(thePubKeyBlock.GetBlockId());
		aBuf.AppendByte(mNeedKeyFlg);
		aBuf.AppendByte(mCreateAcctFlg);
		aBuf.Append_PW_STRING(mUserName);
		aBuf.Append_PW_STRING(mCommunityName);
		aBuf.Append_PW_STRING(mNicknameKey);
		aBuf.Append_PW_STRING(mPassword);
		aBuf.Append_PW_STRING(mNewPassword);
		aBuf.AppendShort(mCDKey.size());
		aBuf.AppendBytes(mCDKey.size(),mCDKey.data());
		aBuf.AppendShort(mLoginKey.size());
		aBuf.AppendBytes(mLoginKey.size(),mLoginKey.data());

		aCryptRet = theSessionKey.Encrypt(aBuf.GetDataPtr(),aBuf.GetDataLen());
		if(aCryptRet.first==NULL)
			return false;

		mDataBuf = aCryptRet.first;
		SetRawDataBuf(aCryptRet.first,aCryptRet.second,false);
		aCryptRet.first = NULL;
	}
	catch(WONCommon::WONException&) {
		return false;
	}

	return true;
}


bool TMsgAuth1LoginRequestHW::ExtractBuffer(const WONCrypt::EGPrivateKey &thePrivateKey, 
																		 WONCrypt::BFSymmetricKey *theSessionKey)
{
	try {
		WONCrypt::CryptKeyBase::CryptReturn aCryptRet(NULL,0);

		aCryptRet = thePrivateKey.Decrypt(GetRawKeyBuf(),GetRawKeyBufLen());
		auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

		if(aCryptRet.first==NULL) 
			return false;

		theSessionKey->Create(aCryptRet.second,aCryptRet.first);
		
		aCryptRet.first=NULL;
		aCryptRet = theSessionKey->Decrypt(GetRawDataBuf(),GetRawDataBufLen());
		if(aCryptRet.first==NULL)
			return false;

		TRawMsg aBuf(aCryptRet.second, aCryptRet.first);
		int aBlockId = aBuf.ReadShort();
		if(aBlockId!=GetKeyBlockId())
			return false;

		mNeedKeyFlg = aBuf.ReadByte()!=0;
		mCreateAcctFlg = aBuf.ReadByte()!=0;
		aBuf.ReadWString(mUserName);
		aBuf.ReadWString(mCommunityName);
		aBuf.ReadWString(mNicknameKey);
		aBuf.ReadWString(mPassword);
		aBuf.ReadWString(mNewPassword);
	
		unsigned short aLen;
		aLen = aBuf.ReadShort();
		mCDKey.assign((const unsigned char*)aBuf.ReadBytes(aLen),aLen);
		
		aLen = aBuf.ReadShort();
		mLoginKey.assign((const unsigned char*)aBuf.ReadBytes(aLen),aLen);
	}
	catch(WONCommon::WONException&) {
		return false;
	}

	return true;
}


// TMsgAuth1LoginRequestHW::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginRequestHW::Pack(void)
{
	WTRACE("TMsgAuth1LoginRequestHW::Pack");
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginRequestHW);
	TMsgAuth1LoginBase2::Pack();

	return GetDataPtr();
}


// TMsgAuth1LoginRequest::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
// Note: call ForceRawBufOwn() to force ownership of the data buffers.
void
TMsgAuth1LoginRequestHW::Unpack(void)
{
	WTRACE("TMsgAuth1LoginRequestHW::Unpack");
	TMsgAuth1LoginBase2::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	    (GetMessageType() != WONMsg::Auth1LoginRequestHW))
	{
		WDBG_AH("TMsgAuth1LoginRequestHW::Unpack Not a Auth1LoginRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginRequestHW message.");
	}

}

// ** TMsgAuth1ChallengeHW **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1ChallengeHW::TMsgAuth1ChallengeHW(void) :
	TMsgAuthRawBufferBase()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginChallengeHW);
}


// TMessage ctor
TMsgAuth1ChallengeHW::TMsgAuth1ChallengeHW(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1ChallengeHW::TMsgAuth1ChallengeHW(const TMsgAuth1ChallengeHW& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{}


// Destructor
TMsgAuth1ChallengeHW::~TMsgAuth1ChallengeHW(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1ChallengeHW&
TMsgAuth1ChallengeHW::operator=(const TMsgAuth1ChallengeHW& theMsgR)
{
	TMsgAuthRawBufferBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1ChallengeHW::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1ChallengeHW::Pack(void)
{
	WTRACE("TMsgAuth1ChallengeHW::Pack");
	SetServiceType(WONMsg::Auth1Login);
//	SetMessageType(WONMsg::Auth1LoginChallengeHL);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1ChallengeHW::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1ChallengeHW::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1ChallengeHW::Unpack(void)
{
	WTRACE("TMsgAuth1ChallengeHW::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if (GetServiceType() != WONMsg::Auth1Login
	 || GetMessageType() != WONMsg::Auth1LoginChallengeHW )
	{
		WDBG_AH("TMsgAuth1ChallengeHW::Unpack Not a TMsgAuth1ChallengeHW message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a TMsgAuth1ChallengeHW message.");
	}

	WDBG_LL("TMsgAuth1ChallengeHW::Unpack Reading message data");
	UnpackRawBuf();
}


// ** TMsgAuth1ConfirmHW **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1ConfirmHW::TMsgAuth1ConfirmHW(void) :
	TMsgAuthRawBufferBase()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginConfirmHW);
}


// TMessage ctor
TMsgAuth1ConfirmHW::TMsgAuth1ConfirmHW(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1ConfirmHW::TMsgAuth1ConfirmHW(const TMsgAuth1ConfirmHW& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{}


// Destructor
TMsgAuth1ConfirmHW::~TMsgAuth1ConfirmHW(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1ConfirmHW&
TMsgAuth1ConfirmHW::operator=(const TMsgAuth1ConfirmHW& theMsgR)
{
	TMsgAuthRawBufferBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1ConfirmHW::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1ConfirmHW::Pack(void)
{
	WTRACE("TMsgAuth1ConfirmHW::Pack");
	SetServiceType(WONMsg::Auth1Login);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1ConfirmHW::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1ConfirmHW::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1ConfirmHW::Unpack(void)
{
	WTRACE("TMsgAuth1ConfirmHW::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if (GetServiceType() != WONMsg::Auth1Login
	 || GetMessageType() != WONMsg::Auth1LoginConfirmHW )
	{
		WDBG_AH("TMsgAuth1ConfirmHW::Unpack Not a Auth1LoginConfirmHW message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginConfirmHW message.");
	}

	WDBG_LL("TMsgAuth1ConfirmHW::Unpack Reading message data");
	UnpackRawBuf();
}


// ** TMsgAuth1RefreshHW **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1RefreshHW::TMsgAuth1RefreshHW(void) :
	TMsgAuth1LoginBase2()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1RefreshHW);
}


// TMessage ctor
TMsgAuth1RefreshHW::TMsgAuth1RefreshHW(const TMessage& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1RefreshHW::TMsgAuth1RefreshHW(const TMsgAuth1RefreshHW& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR)
{
}


// Destructor
TMsgAuth1RefreshHW::~TMsgAuth1RefreshHW(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1RefreshHW&
TMsgAuth1RefreshHW::operator=(const TMsgAuth1RefreshHW& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
	    TMsgAuth1LoginBase2::operator=(theMsgR);
	}
	return *this;
}


// TMsgAuth1RefreshHW::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1RefreshHW::Pack(void)
{
	WTRACE("TMsgAuth1RefreshHW::Pack");
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1RefreshHW);
	TMsgAuth1LoginBase2::Pack();

	return GetDataPtr();
}


// TMsgAuth1RefreshHW::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
// Note: call ForceRawBufOwn() to force ownership of the data buffers.
void
TMsgAuth1RefreshHW::Unpack(void)
{
	WTRACE("TMsgAuth1RefreshHW::Unpack");
	TMsgAuth1LoginBase2::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	    (GetMessageType() != WONMsg::Auth1RefreshHW))
	{
		WDBG_AH("TMsgAuth1RefreshHW::Unpack Not a Auth1LoginRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginRequest2 message.");
	}

}

