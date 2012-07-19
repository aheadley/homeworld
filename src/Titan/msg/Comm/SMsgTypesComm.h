#ifndef _SMsgTypesComm_H_
#define _SMsgTypesComm_H_

//
// SmallMessage Common message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONMsg
{
	enum MsgTypeSmallsComm
	{
		// Common Messages
		SmallCommMin         = 0,
		SmallCommUndefined   = SmallCommMin,

		SmallCommRegisterRequest   = SmallCommMin+1,
		SmallCommRegisterRequestEx = SmallCommMin+2,

		SmallCommStatusReply       = SmallCommMin+15, // Firewall Server had already used this value...

		// Last Message type.  Don't use
		SmallCommMax
	};

};

#endif