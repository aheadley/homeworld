#ifndef _SServiceTypes_H_
#define _SServiceTypes_H_

//
// SmallMessage Service Types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

#include <limits.h>

namespace WONMsg 
{
	enum SmallServiceType
	{
		SmallUndefined        = 0,

		SmallCommonService    = 1,
		SmallDirServerG2      = 2,
		SmallFirewallDetector = 3,
		SmallEventServer      = 4,
		SmallFactoryServer	  = 5,
		SmallPingServer       = 6,
		SmallProfileServer	  = 7,
		SmallParamServer      = 8,

		// Let's reserve some services for clients (game use)
		MaxSmallServiceType  = USHRT_MAX-1000
	};
};

#endif