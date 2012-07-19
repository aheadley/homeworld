#if !defined(MMsgRoutingDeleteDataObject_H)
#define MMsgRoutingDeleteDataObject_H

// MMsgRoutingDeleteDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingDeleteDataObject
//
class MMsgRoutingDeleteDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingDeleteDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingDeleteDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingDeleteDataObject(const MMsgRoutingDeleteDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingDeleteDataObject(void);

    // Assignment
    MMsgRoutingDeleteDataObject& operator=(const MMsgRoutingDeleteDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

// Inlines
inline TRawMsg* MMsgRoutingDeleteDataObject::Duplicate(void) const
    { return new MMsgRoutingDeleteDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingDeleteDataObject_H