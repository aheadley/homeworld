#if !defined(MMsgRoutingStatusReply_H)
#define MMsgRoutingStatusReply_H

// MMsgRoutingStatusReply.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingStatusReply : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingStatusReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingStatusReply(const RoutingServerMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    MMsgRoutingStatusReply(const MMsgRoutingStatusReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingStatusReply(void);

    // Assignment
    MMsgRoutingStatusReply& operator=(const MMsgRoutingStatusReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	short GetStatus() const;
	void  SetStatus(short theStatus);
protected:
	short mStatus;
};


// Inlines
inline TRawMsg* MMsgRoutingStatusReply::Duplicate(void) const
    { return new MMsgRoutingStatusReply(*this); }

inline short MMsgRoutingStatusReply::GetStatus() const
{ return mStatus; }
inline void MMsgRoutingStatusReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

};  // Namespace WONMsg

#endif // MMsgRoutingStatusReply_H