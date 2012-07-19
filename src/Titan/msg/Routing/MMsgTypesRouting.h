#if !defined(MMsgTypesRouting_H)
#define MMsgTypesRouting_H

//
// Titan Routing Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum MsgTypeRouting
	{
		// Unused/Invalid Routing Message Type
		RoutingMsgMin                = 0,
		RoutingUndefined             = 0,

		// Routing Server Messages
		RoutingAcceptClient             = 1,
		RoutingAcceptClientReply        = 2,
		RoutingAddToGroup               = 3,
		RoutingBanClient                = 4,
		RoutingBootClient               = 5,
		RoutingClientBooted             = 6,
		RoutingClientChange             = 7,
		RoutingCloseRegistration        = 8,
		RoutingCreateDataObject         = 9,
		RoutingCreateGroup              = 10,
		RoutingDeleteDataObject         = 11,
		RoutingDeleteGroup              = 12,
		RoutingDisconnectClient         = 13,
		RoutingGetClientInfo            = 14,
		RoutingGetClientList            = 15,
		RoutingGetClientListReply       = 16,
		RoutingGetGroupList             = 17,
		RoutingGetGroupListReply        = 18,
// REUSE:		RoutingGetUserList              = 19,
// REUSE:		RoutingGetUserListReply         = 20,
		RoutingInviteClient             = 21,
		RoutingKeepAlive                = 22,
		RoutingClientChangeEx           = 23,
		RoutingModifyDataObject         = 24,
		RoutingOpenRegistration         = 25,
		RoutingPeerData                 = 26,
		RoutingPeerDataMultiple         = 27,
		RoutingReadDataObject           = 28,
		RoutingReadDataObjectReply      = 29,
		RoutingReconnectClient          = 30,
		RoutingRegisterClient           = 31,
		RoutingRegisterClientReply      = 32,
		RoutingRemoveFromGroup          = 33,
		RoutingRenewDataObject          = 34,
		RoutingReplaceDataObject        = 35,
		RoutingSendData                 = 36,
		RoutingSendDataBroadcast        = 37,
		RoutingSendDataMultiple         = 38,
		RoutingSetPassword              = 39,
		RoutingStatusReply              = 40,
		RoutingSubscribeDataObject      = 41,
		RoutingMuteClient               = 42,
// REUSE:		RoutingUninviteUser             = 43,
		RoutingUnsubscribeDataObject    = 44,
		RoutingCreateGroupReply         = 45,
		RoutingAddSuccessor             = 46,
		RoutingRemoveSuccessor          = 47,
		RoutingHostChange               = 48,
		RoutingHostSuccessionInProgress = 49,
		RoutingGetMembersOfGroup        = 50,
		RoutingGroupChange              = 51,
		RoutingGroupChangeEx            = 52,
		RoutingSendChat                 = 53,
		RoutingPeerChat                 = 54,
		RoutingSpectatorCount           = 55,
		RoutingGroupSpectatorCount      = 56,
		RoutingGetSimpleClientList      = 57,
		RoutingGetSimpleClientListReply = 58,

		RoutingLargestValuePlusOne,
		RoutingLargestValueInUse     = RoutingLargestValuePlusOne-1,

		// Last Message type.  Don't use
		RoutingMsgMax = 255
	};

};

#endif // MMsgTypesRouting_H