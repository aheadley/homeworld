#if !defined(MMsgRoutingSubscribeDataObject_H)
#define MMsgRoutingSubscribeDataObject_H

// MMsgRoutingSubscribeDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingSubscribeDataObject
//
class MMsgRoutingSubscribeDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingSubscribeDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSubscribeDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSubscribeDataObject(const MMsgRoutingSubscribeDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSubscribeDataObject(void);

    // Assignment
    MMsgRoutingSubscribeDataObject& operator=(const MMsgRoutingSubscribeDataObject& theMsgR);

    // Virtual Duplicate from MMsgRoutingBaseDataObject
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool GetExactOrRecursiveFlag() const                       { return mExactOrRecursiveFlag; }
	bool GetGroupOrMembersFlag() const                         { return mGroupOrMembersFlag; }

	void SetExactOrRecursiveFlag(bool theExactOrRecursiveFlag) { mExactOrRecursiveFlag = theExactOrRecursiveFlag; }
	void SetGroupOrMembersFlag(bool theGroupOrMembersFlag)     { mGroupOrMembersFlag = theGroupOrMembersFlag; }
private:
	bool mExactOrRecursiveFlag;
	bool mGroupOrMembersFlag;
};

// Inlines
inline TRawMsg* MMsgRoutingSubscribeDataObject::Duplicate(void) const
    { return new MMsgRoutingSubscribeDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingSubscribeDataObject_H