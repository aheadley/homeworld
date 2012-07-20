#include "common/WON.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "titanpacketmsg.h"
#include "titaninterfacec.h"

using WONMsg::MiniMessage;


// ** TitanPacketMsg **

// ** Constructors / Destructor

// Default ctor
TitanPacketMsg::TitanPacketMsg(unsigned char theType, bool encrypted) :
    mBlob(NULL),
    mBlobLen(0),
	mIncludeGameName(theType != TITANMSGTYPE_GAME)
{
	SetServiceType(encrypted ? MyEncryptServiceType : MyServiceType);
    SetMessageType(theType);
}


// TMessage ctor
TitanPacketMsg::TitanPacketMsg(const MiniMessage& theMsgR) :
    MiniMessage(theMsgR),
	mIncludeGameName(true),
    mBlob(NULL),
    mBlobLen(0)
{
    Unpack();
}


// Copy ctor
TitanPacketMsg::TitanPacketMsg(const TitanPacketMsg& theMsgR) :
    MiniMessage(theMsgR),
    mIncludeGameName(theMsgR.mIncludeGameName),
	mGameName(theMsgR.mGameName),
	mBlob(theMsgR.mBlob),
	mBlobLen(theMsgR.mBlobLen)
{

    SetBlob(theMsgR.GetBlob(), theMsgR.GetBlobLen());
}


// Destructor
TitanPacketMsg::~TitanPacketMsg(void)
{}


// ** Public Methods

// Assignment operator
TitanPacketMsg&
TitanPacketMsg::operator=(const TitanPacketMsg& theMsgR)
{
    if (this != &theMsgR)
    {
        MiniMessage::operator=(theMsgR);
        mIncludeGameName = theMsgR.mIncludeGameName;
		mGameName        = theMsgR.mGameName;
		mBlob            = theMsgR.mBlob;
		mBlobLen         = theMsgR.mBlobLen;
    }
    return *this;
}


// TitanPacketMsg::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
TitanPacketMsg::Pack(void)
{
    MiniMessage::Pack();
    
	// append game name if required
	if (mIncludeGameName)
		Append_PW_STRING(mGameName);
	
	// append data
	AppendBytes(mBlobLen, mBlob);

    return GetDataPtr();
}

// TitanPacketMsg::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
TitanPacketMsg::Unpack(void)
{
    MiniMessage::Unpack();

    if ((GetServiceType() != MyEncryptServiceType) &&
	    (GetServiceType() != MyServiceType))
    {
        throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
                                      "Not a TitanPacketMsg message.");
    }

	mIncludeGameName = (GetMessageType() != TITANMSGTYPE_GAME);
	if (mIncludeGameName)
		ReadWString(mGameName);
    
	mBlobLen = BytesLeftToRead();
    mBlob = ReadBytes(BytesLeftToRead());
}
