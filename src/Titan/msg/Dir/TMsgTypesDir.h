#ifndef _TMsgTypesDir_H_
#define _TMsgTypesDir_H_

//
// Titan Directory Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum MsgTypeDir
	{
		// First message type.  Don't use
		DirMin = 0,

		// Directory Server Messages
		DirStatusReply           = 1,
		DirGetDirContents        = 2,
		DirGetDirContentsReply   = 3,
		DirAddService            = 4,
		DirRenewService          = 5,
		DirRemoveService         = 6,
		DirChangeService         = 7,
		DirAddDirectory          = 8,
		DirRenewDirectory        = 9,
		DirRemoveDirectory       = 10,
		DirChangeDirectory       = 11,
		DirDumpTree              = 12,
		DirLoadFromSource        = 13,
		DirPeerConnect           = 14,
		DirPeerConnectReply      = 15,
		DirPeerSynch             = 16,
		DirSynchDirTree          = 17,
		DirFindService           = 18,
		DirFindServiceReply      = 19,
		DirCopyDirectory         = 20,
		DirGetNumDirEntries      = 21,
		DirGetNumDirEntriesReply = 22,

		// Last Message type.  Don't use
		DirMsgMax
	};

};

#endif