#ifndef _MServiceTypes_H_
#define _MServiceTypes_H_

//
// MiniMessage Service Types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg 
{
	enum MiniServiceType
	{

		// Since we're limited to 256 types, we need to worry about running out
		// of ServiceType ids.  For now, feel free to pick a unique id for a new server.
		// However, in the future we may force multiple servers to use the same id.
		// Do not assume that two servers will always have different service types.

		MiniUndefined       = 0,

		MiniCommonService   = 1,
		MiniRoutingServer   = 2,

		MiniCreamServer		= 3,

		MiniContestServer	= 4,		

		MiniObsServer		= 6,

        MiniAuth1PeerToPeer = 203,

		// Let's reserve some services for clients (game use)
		MaxMiniServiceType  = 225
	};
};

#endif