#if !defined(MMsgRoutingReadDataObject_H)
#define MMsgRoutingReadDataObject_H

// MMsgRoutingReadDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingReadDataObject
//
class MMsgRoutingReadDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingReadDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingReadDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingReadDataObject(const MMsgRoutingReadDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingReadDataObject(void);

    // Assignment
    MMsgRoutingReadDataObject& operator=(const MMsgRoutingReadDataObject& theMsgR);

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
inline TRawMsg* MMsgRoutingReadDataObject::Duplicate(void) const
    { return new MMsgRoutingReadDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingReadDataObject_H