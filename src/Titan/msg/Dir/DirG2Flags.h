#ifndef _DirG2Flags_H
#define _DirG2Flags_H

// DirG2Flags

// Defines various enums for the flags used in 2nd generation DirServer messages.


// In the WONMsg namespace
namespace WONMsg {

// GetFlags (ulong) - Control which fields of a DirEntity are returned in query requests.
enum DirG2GetFlags
{
	GF_NOFLAGS = 0,

	// Bits 0-15 are for decomposition and common flags

	// Decomposition Flags - apply these decompositions for Directories
	GF_DECOMPROOT      = 0x00000001,  // Add the dir itself 
	GF_DECOMPSERVICES  = 0x00000002,  // Add dir services
	GF_DECOMPSUBDIRS   = 0x00000004,  // Add dir subdirs
	GF_DECOMPRECURSIVE = 0x00000008,  // Recursive into dir subdirs

	// Common flags - include these attributes for all entities
	GF_ADDTYPE         = 0x00000010,  // Add entity types
	GF_ADDDISPLAYNAME  = 0x00000020,  // Add display names
	GF_ADDCREATED      = 0x00000040,  // Add creation date/time
	GF_ADDTOUCHED      = 0x00000080,  // Add touched date/time
	GF_ADDLIFESPAN     = 0x00000100,  // Add lifespan
	GF_ADDDOTYPE       = 0x00000200,  // Add DataObject types
	GF_ADDDODATA       = 0x00000400,  // Add DataObject data
	GF_ADDDATAOBJECTS  = 0x00000800,  // Add all DataObjects
	GF_ADDPERMISSIONS  = 0x00001000,  // Add permissions
	GF_ADDCRC          = 0x00002000,  // Add entity CRC

	// Bits 16-23 are for Directory only fields

	// Directory Flags - include these attributes for directories
	GF_DIRADDPATH      = 0x00010000,  // Add dir paths (from root)
	GF_DIRADDNAME      = 0x00020000,  // Add service names
	GF_DIRADDVISIBLE   = 0x00040000,  // Add directory visibility

	// Bits 24-31 are for Service only fields

	// Service Flags - include these attributes for services
	GF_SERVADDPATH     = 0x01000000,  // Add dir paths (from root)
	GF_SERVADDNAME     = 0x02000000,  // Add service names
	GF_SERVADDNETADDR  = 0x04000000,  // Add service net addresses

	GF_ALLFLAGS = 0xffffffff
};


// Entity Flags (byte) - Control attributes in add/change requests
enum DirG2EntityFlags
{
	EF_NOFLAGS = 0,

	EF_UNIQUEDISPLAYNAME = 0x01,  // Display name must be unique
	EF_DIRVISIBLE        = 0x02,  // Directory is visible
	EF_DIRINVISIBLE      = 0x04,  // Directory is invisible
	EF_OVERWRITE         = 0x08,  // Overwrite existing entities
	EF_SERVRETURNADDR    = 0x10,  // Return service net address in reply

	EF_ALLFLAGS = 0xff
};


// DataObjectSetMode (byte) - Mode for setting data objects on a DirEntity
enum DirG2DataObjectSetMode
{
	DOSM_ADDREPLACE    = 0,  // Add on not exist, replace on exist
	DOSM_ADDIGNORE     = 1,  // Add on not exist, ignore on exist
	DOSM_ADDONLY       = 2,  // Add on not exist, error on exist
	DOSM_REPLACEIGNORE = 3,  // Replace on exist, ignore on not exist
	DOSM_REPLACEONLY   = 4,  // Replace on exist, error on not exist
	DOSM_RESETDELETE   = 5,  // Clear existing set first, then add all.

	DOSM_MAX
};


// FindMatchMode (byte) - Mode for find queries
enum DirG2FindMatchMode
{
	FMM_EXACT   = 0,  // Compared value must equal search value
	FMM_BEGIN   = 1,  // Compared value must begin with search value
	FMM_END     = 2,  // Compared value must end with search value
	FMM_CONTAIN = 3,  // Compared value must contain search value

	FMM_MAX
};


// FindFlags (byte) - Control flags for find queries
enum DirG2FindFlags
{
	FF_NOFLAGS   = 0,

	FF_MATCHALL  = 0x01,  // Return all valid matches
	FF_FULLKEY   = 0x02,  // Match only if all search field match
	FF_RECURSIVE = 0x04,  // Search directories recursively for matches

	FF_ALLFLAGS  = 0xff
};


};  // Namespace WONMsg

#endif