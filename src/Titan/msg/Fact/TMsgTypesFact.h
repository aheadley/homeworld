#if !defined(TMsgTypesFact_H)
#define TMsgTypesFact_H

//
// Titan Factory Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum MsgTypeFact
	{
		// First message type.  Don't use
		FactMsgMin = 0,

		// Factory Server Messages
		FactStatusReply           = FactMsgMin+1,
		FactStartProcess          = FactMsgMin+2,
		FactStopProcess           = FactMsgMin+3,
		FactGetProcessConfig      = FactMsgMin+4,
		FactGetProcessList        = FactMsgMin+5,
		FactProcessListReply      = FactMsgMin+6,
		FactProcessConfigReply    = FactMsgMin+7,
		FactStartProcessUnicode   = FactMsgMin+8,
		FactGetProcessPorts       = FactMsgMin+9,
		FactGetAllProcesses		  = FactMsgMin+10,
		FactGetAllProcessesReply  = FactMsgMin+11,
		FactKillProcess			  = FactMsgMin+12,

		// Last Message type.  Don't use
		FactMsgMax
	};

};

#endif