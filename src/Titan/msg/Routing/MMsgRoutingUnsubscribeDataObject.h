#if !defined(MMsgRoutingUnsubscribeDataObject_H)
#define MMsgRoutingUnsubscribeDataObject_H

// MMsgRoutingUnsubscribeDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingUnsubscribeDataObject
//
class MMsgRoutingUnsubscribeDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingUnsubscribeDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingUnsubscribeDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingUnsubscribeDataObject(const MMsgRoutingUnsubscribeDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingUnsubscribeDataObject(void);

    // Assignment
    MMsgRoutingUnsubscribeDataObject& operator=(const MMsgRoutingUnsubscribeDataObject& theMsgR);

    // Virtual Duplicate from MMsgRoutingBaseDataObject
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const WONCommon::RawBuffer& GetData() const;
	WONCommon::RawBuffer& GetData();

	bool GetExactOrRecursiveFlag() const;
	void SetExactOrRecursiveFlag(bool theExactOrRecursiveFlag);

	bool GetGroupOrMembersFlag() const;
	void SetGroupOrMembersFlag(bool theGroupOrMembersFlag);
private:
	WONCommon::RawBuffer mData;
	bool                 mExactOrRecursiveFlag;
	bool                 mGroupOrMembersFlag;
};

// Inlines
inline TRawMsg* MMsgRoutingUnsubscribeDataObject::Duplicate(void) const
    { return new MMsgRoutingUnsubscribeDataObject(*this); }

inline const WONCommon::RawBuffer& MMsgRoutingUnsubscribeDataObject::GetData() const
	{ return mData; }
inline WONCommon::RawBuffer& MMsgRoutingUnsubscribeDataObject::GetData()
	{ return mData; }
inline bool MMsgRoutingUnsubscribeDataObject::GetExactOrRecursiveFlag() const
	{ return mExactOrRecursiveFlag; }
inline void MMsgRoutingUnsubscribeDataObject::SetExactOrRecursiveFlag(bool theExactOrRecursiveFlag)
	{ mExactOrRecursiveFlag = theExactOrRecursiveFlag; }
inline bool MMsgRoutingUnsubscribeDataObject::GetGroupOrMembersFlag() const
	{ return mGroupOrMembersFlag; }
inline void MMsgRoutingUnsubscribeDataObject::SetGroupOrMembersFlag(bool theGroupOrMembersFlag)
	{ mGroupOrMembersFlag = theGroupOrMembersFlag; }

};  // Namespace WONMsg

#endif // MMsgRoutingUnsubscribeDataObject_H