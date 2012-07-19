#if !defined(TMsgTypesEvent_H)
#define TMsgTypesEvent_H

//
// Titan Event Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum MsgTypeEvent
	{
		// First message type.  Don't use
		EventMsgMin = 0,

		// Event Server Messages
		EventRecordEvent       = EventMsgMin + 1,
		EventStatusReply       = EventMsgMin + 2,
		EventTaggedRecordEvent = EventMsgMin + 3,
		EventTaggedStatusReply = EventMsgMin + 4,

		// Last Message type.  Don't use
		EventMsgMax
	};

};

#endif // TMsgTypesEvent_H