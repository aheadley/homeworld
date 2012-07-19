#if !defined(TMsgFactGetProcessList_H)
#define TMsgFactGetProcessList_H

// TMsgFactGetProcessList.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgFactGetProcessList : public TMessage {

public:
	// Default ctor
	TMsgFactGetProcessList(void);

	// TMessage ctor
	explicit TMsgFactGetProcessList(const TMessage& theMsgR);

	// Copy ctor
	TMsgFactGetProcessList(const TMsgFactGetProcessList& theMsgR);

	// Destructor
	virtual ~TMsgFactGetProcessList(void);

	// Assignment
	TMsgFactGetProcessList& operator=(const TMsgFactGetProcessList& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access

protected:

};


// Inlines
inline TRawMsg* TMsgFactGetProcessList::Duplicate(void) const
{ return new TMsgFactGetProcessList(*this); }

};  // Namespace WONMsg

#endif