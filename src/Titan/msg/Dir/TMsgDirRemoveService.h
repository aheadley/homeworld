#ifndef _TMsgDirRemoveService_H
#define _TMsgDirRemoveService_H

// TMsgDirRemoveService.h

// DirectoryServer Remove Entry message.  Defines an entry to remove.


#include "msg/TMessage.h"
#include "TMsgDirServiceBase.h"

namespace WONMsg {

class TMsgDirRemoveService : public TMsgDirServiceBase
{
public:
	// Default ctor
	TMsgDirRemoveService(void);

	// TMessage ctor
	explicit TMsgDirRemoveService(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirRemoveService(const TMsgDirRemoveService& theMsgR);

	// Destructor
	~TMsgDirRemoveService(void);

	// Assignment
	TMsgDirRemoveService& operator=(const TMsgDirRemoveService& theMsgR);

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
TMsgDirRemoveService::Duplicate(void) const
{ return new TMsgDirRemoveService(*this); }

};  // Namespace WONMsg

#endif