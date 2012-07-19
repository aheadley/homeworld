#if !defined(MMsgRoutingRenewDataObject_H)
#define MMsgRoutingRenewDataObject_H

// MMsgRoutingRenewDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingRenewDataObject
//
class MMsgRoutingRenewDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingRenewDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingRenewDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingRenewDataObject(const MMsgRoutingRenewDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingRenewDataObject(void);

    // Assignment
    MMsgRoutingRenewDataObject& operator=(const MMsgRoutingRenewDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	unsigned short GetNewLifespan() const;
	void           SetNewLifespan(unsigned short theNewLifespan);
private:
	unsigned short mNewLifespan;
};

// Inlines
inline TRawMsg* MMsgRoutingRenewDataObject::Duplicate(void) const
    { return new MMsgRoutingRenewDataObject(*this); }

inline unsigned short MMsgRoutingRenewDataObject::GetNewLifespan() const
	{ return mNewLifespan; }
inline void MMsgRoutingRenewDataObject::SetNewLifespan(unsigned short theNewLifespan)
	{ mNewLifespan = theNewLifespan; }

};  // Namespace WONMsg

#endif // MMsgRoutingRenewDataObject_H