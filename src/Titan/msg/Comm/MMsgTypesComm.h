#ifndef _MMsgTypesComm_H_
#define _MMsgTypesComm_H_

//
// MiniMesage Common message types
//

// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONMsg
{
	enum MsgTypeMiniComm
	{
		// Common Messages
		MiniCommMin                = 0,
		MiniCommUndefined          = MiniCommMin,

		MiniCommGetNumUsers        = MiniCommMin+1,
		MiniCommGetNumUsersReply   = MiniCommMin+2,
		MiniCommIsUserPresent      = MiniCommMin+3,
		MiniCommIsUserPresentReply = MiniCommMin+4,
		MiniCommPing               = MiniCommMin+5,
		MiniCommPingReply          = MiniCommMin+6,
		MiniCommNoOp               = MiniCommMin+7,
		MiniCommGetNetStat         = MiniCommMin+8,
		MiniCommGetNetStatReply    = MiniCommMin+9,

		// Last Message type.  Don't use
		MiniCommMax = 255
	};

};

#endif