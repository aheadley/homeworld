#ifndef TMsgEventTaggedRecordEvent_H
#define TMsgEventTaggedRecordEvent_H

// SMsgEventTaggedRecordEvent.h

#include "SMsgEventRecordEvent.h"

namespace WONMsg {

class SMsgEventTaggedRecordEvent : public SMsgEventRecordEvent {
public:
	// Default ctor
    SMsgEventTaggedRecordEvent(void);

    // SmallMessage ctor
    explicit SMsgEventTaggedRecordEvent(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgEventTaggedRecordEvent(const SMsgEventTaggedRecordEvent& theMsgR);

    // Destructor
    virtual ~SMsgEventTaggedRecordEvent(void);

    // Assignment
    SMsgEventTaggedRecordEvent& operator=(const SMsgEventTaggedRecordEvent& theMsgR);

    // Virtual Duplicate from SmallMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	// Dumping
	virtual void Dump(std::ostream& os) const;
    
	// Member access
	void SetTag(unsigned short theTag);
	unsigned short GetTag() const;
private:
	unsigned short mTag;
};


// Inlines
inline TRawMsg* SMsgEventTaggedRecordEvent::Duplicate(void) const
    { return new SMsgEventTaggedRecordEvent(*this); }

inline void SMsgEventTaggedRecordEvent::SetTag(unsigned short theTag)
	{ mTag = theTag; }
inline unsigned short SMsgEventTaggedRecordEvent::GetTag() const
	{ return mTag; }

};  // Namespace WONMsg

#endif // TMsgEventTaggedRecordEvent_H