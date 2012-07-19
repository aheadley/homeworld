// SMsgCommStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesComm.h"
#include "SMsgCommStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgCommStatusReply;
};


// ** Constructors / Destructor

// Default ctor
SMsgCommStatusReply::SMsgCommStatusReply(void) :
	SmallMessage(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommStatusReply);
}


// SmallMessage ctor
SMsgCommStatusReply::SMsgCommStatusReply(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy ctor
SMsgCommStatusReply::SMsgCommStatusReply(const SMsgCommStatusReply& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{}


// Destructor
SMsgCommStatusReply::~SMsgCommStatusReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgCommStatusReply&
SMsgCommStatusReply::operator=(const SMsgCommStatusReply& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	return *this;
}


// SMsgCommStatusReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer and
// sets the new message length.
void*
SMsgCommStatusReply::Pack(void)
{
	WTRACE("SMsgCommStatusReply::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommStatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgCommStatusReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	return GetDataPtr();
}


// SMsgCommStatusReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgCommStatusReply::Unpack(void)
{
	WTRACE("SMsgCommStatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallCommonService) ||
	    (GetMessageType() != WONMsg::CommStatusReply))
	{
		WDBG_AH("SMsgCommStatusReply::Unpack Not a CommStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommStatusReply message.");
	}

	WDBG_LL("SMsgCommStatusReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
}
