#if !defined(MMsgRoutingReplaceDataObject_H)
#define MMsgRoutingReplaceDataObject_H

// MMsgRoutingReplaceDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingReplaceDataObject
//
class MMsgRoutingReplaceDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingReplaceDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingReplaceDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingReplaceDataObject(const MMsgRoutingReplaceDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingReplaceDataObject(void);

    // Assignment
    MMsgRoutingReplaceDataObject& operator=(const MMsgRoutingReplaceDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const WONCommon::RawBuffer& GetData() const                  { return mData; }

	void SetData(const WONCommon::RawBuffer& theDataR)           { mData = theDataR; }
	void AppendToData(const WONCommon::RawBuffer& theAppendandR) { mData += theAppendandR; }
private:
	WONCommon::RawBuffer mData;
};

// Inlines
inline TRawMsg* MMsgRoutingReplaceDataObject::Duplicate(void) const
    { return new MMsgRoutingReplaceDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingReplaceDataObject_H