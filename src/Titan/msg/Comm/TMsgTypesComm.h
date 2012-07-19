#ifndef _TMsgTypesComm_H_
#define _TMsgTypesComm_H_

//
// Titan Common message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONMsg
{
	enum MsgTypeComm
	{
		// Common Messages
		CommMin               = 0,
		CommDebugLevel        = 1,
		CommTracing           = 2,
		CommMsgTimeout        = 3,
		CommRehupOptions      = 4,
		CommPing              = 5,
		CommPingReply         = 6,
		CommProtocol          = 7,
        CommProtocolReject    = 8,
		CommQueryOptions      = 9,
		CommQueryOptionsReply = 10,
		CommShutDownRequest   = 11,
		CommShutDownConfirm   = 12,
		CommShutDownConfReply = 13,
		CommRegisterRequest   = 14,
		CommStatusReply       = 15,
		CommNoOp              = 16,

		// Last Message type.  Don't use
		CommMax
	};

};

#endif