#ifndef _SMsgTypesDir_H_
#define _SMsgTypesDir_H_

// Titan Directory Server message types for SmallMessages
// These values must be fixed in order to communicate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always assign new values to
// unused value!!!!

namespace WONMsg
{
	enum SMsgTypeDir
	{
		// First message type.  Don't use
		SMsgDirMin = 0,

		// Common Replies
		DirG2StatusReply                  = 1,
		DirG2SingleEntityReply            = 2,
		DirG2MultiEntityReply             = 3,

		// Queries (and their replies where applicable)
		DirG2GetDirectoryContents         = 100,
		DirG2GetDirectoryContentsReply    = 101,
		DirG2GetDirectory                 = 102,
		DirG2GetDirectoryEx               = 103,
		DirG2GetService                   = 104,
		DirG2GetServiceEx                 = 105,
		DirG2FindDirectory                = 106,
		DirG2FindDirectoryEx              = 107,
		DirG2FindService                  = 108,
		DirG2FindServiceEx                = 109,
		DirG2GetNumEntities               = 110,
		DirG2GetNumEntitiesReply          = 111,

		// Update Requests
		DirG2AddDirectory                 = 200,
		DirG2AddDirectoryEx               = 201,
		DirG2AddService                   = 202,
		DirG2AddServiceEx                 = 203,
		DirG2RenewDirectory               = 204,
		DirG2RenewService                 = 205,
		DirG2NameDirectory                = 206,
		DirG2NameService                  = 207,
		DirG2RemoveDirectory              = 208,
		DirG2RemoveService                = 209,
		DirG2ModifyDirectory              = 210,
		DirG2ModifyDirectoryEx            = 211,
		DirG2ModifyService                = 212,
		DirG2ModifyServiceEx              = 213,

		// Data Object Requests
		DirG2DirectorySetDataObjects         = 300,
		DirG2ServiceSetDataObjects           = 301,
		DirG2DirectoryClearDataObjects       = 302,
		DirG2ServiceClearDataObjects         = 303,
		DirG2DirectoryModifyDataObjects      = 304,
		DirG2ServiceModifyDataObjects        = 305,
		DirG2DirectoryExplicitSetDataObjects = 306,
		DirG2ServiceExplicitSetDataObjects   = 307,

		// Administration and System Messages (and their replies where applicable)
		DirG2PeerSynch                    = 1000,
		DirG2PeerConnect                  = 1001,
		DirG2PeerConnectReply             = 1002,
		DirG2PeerAttach                   = 1003,
		DirG2LoadFromSrc                  = 1004,
		DirG2LoadFromSrcReply             = 1005,
		DirG2SynchTree                    = 1006,
		DirG2DumpTree                     = 1007,
		DirG2PeerSynchTest                = 1008,
		DirG2PeerSynchPath                = 1009,

		// Last Message type.  Don't use
		SMsgDirMax
	};

};

#endif