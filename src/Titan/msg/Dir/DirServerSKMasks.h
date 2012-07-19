#ifndef _DirServerSKMasks_H_
#define _DirServerSKMasks_H_

//
// DirServerSKMasks.h
// Defines the codes used in Directories to mask off comparison fields of
// contained services.


namespace WONMsg
{
	enum DirServerSKMasks
	{
		SKMASK_ALL             = 0x00,
		SKMASK_NO_DISPLAYNAME  = 0x01,
		SKMASK_NO_VERSION      = 0x02,
		SKMASK_NO_PROTONAME    = 0x04,
		SKMASK_NO_PROTOVERSION = 0x08,
		SKMASK_NO_NETADDRESS   = 0x10,
		SKMASK_NONE            = 0xFF
	};
};

#endif