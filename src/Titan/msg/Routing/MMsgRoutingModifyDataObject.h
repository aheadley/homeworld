#if !defined(MMsgRoutingModifyDataObject_H)
#define MMsgRoutingModifyDataObject_H

// MMsgRoutingModifyDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingModifyDataObject
//
class MMsgRoutingModifyDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingModifyDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingModifyDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingModifyDataObject(const MMsgRoutingModifyDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingModifyDataObject(void);

    // Assignment
    MMsgRoutingModifyDataObject& operator=(const MMsgRoutingModifyDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	unsigned short GetOffset() const                             { return mOffset; }
	bool IsInsert() const                                        { return mIsInsert; }
	const WONCommon::RawBuffer& GetData() const                  { return mData; }

	void SetOffset(unsigned short theOffset)                     { mOffset = theOffset; }
	void SetIsInsert(bool isInsert)                              { mIsInsert = isInsert; }
	void SetData(const WONCommon::RawBuffer& theDataR)           { mData = theDataR; }
	void AppendToData(const WONCommon::RawBuffer& theAppendandR) { mData += theAppendandR; }
private:
	unsigned short       mOffset;
	bool                 mIsInsert;
	WONCommon::RawBuffer mData;
};

// Inlines
inline TRawMsg* MMsgRoutingModifyDataObject::Duplicate(void) const
    { return new MMsgRoutingModifyDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingModifyDataObject_H