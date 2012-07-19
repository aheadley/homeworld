#ifndef _TMsgDirRemoveDirectory_H
#define _TMsgDirRemoveDirectory_H

// TMsgDirRemoveDirectory.h

// DirectoryServer Remove SubDirectory message.  Defines a subdirectory to remove.

#include "msg/TMessage.h"
#include "TMsgDirDirectoryBase.h"

namespace WONMsg {

class TMsgDirRemoveDirectory : public TMsgDirDirectoryBase
{
public:
	// Default ctor
	TMsgDirRemoveDirectory(void);

	// TMessage ctor
	explicit TMsgDirRemoveDirectory(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirRemoveDirectory(const TMsgDirRemoveDirectory& theMsgR);

	// Destructor
	~TMsgDirRemoveDirectory(void);

	// Assignment
	TMsgDirRemoveDirectory& operator=(const TMsgDirRemoveDirectory& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

private:
};


// Inlines
inline TRawMsg*
TMsgDirRemoveDirectory::Duplicate(void) const
{ return new TMsgDirRemoveDirectory(*this); }

};  // Namespace WONMsg

#endif