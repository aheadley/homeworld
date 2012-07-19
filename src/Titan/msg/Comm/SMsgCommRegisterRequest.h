#if !defined(SMsgCommRegisterRequest_H)
#define SMsgCommRegisterRequest_H

// SMsgCommRegisterRequest.h

// Message that is used to tell the server to register itself with a directory server

#include "LIST"
#include "msg/TMessage.h"
#include "common/DataObject.h"

namespace WONMsg {

class SMsgCommRegisterRequest : public SmallMessage {
public:
    // Default ctor
    SMsgCommRegisterRequest(bool isExtended=false);

    // TMessage ctor
    explicit SMsgCommRegisterRequest(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgCommRegisterRequest(const SMsgCommRegisterRequest& theMsgR);

    // Destructor
    virtual ~SMsgCommRegisterRequest(void);

    // Assignment
    SMsgCommRegisterRequest& operator=(const SMsgCommRegisterRequest& theMsgR);

    // Virtual Duplicate from BaseMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    bool IsExtended(void) const;
    const std::list<std::string>& GetDirServerAddressList(void) const;
    const std::wstring& GetDisplayName(void) const;
    bool RequireUniqueDisplayName(void) const;
    const std::wstring& GetPath(void) const;

    // Data Object access
    const WONCommon::DataObjectTypeSet& GetDataObjects() const;
    void SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR);
    void AddDataObject(const WONCommon::DataObject& theObjR);

    virtual void AddDirServerAddress(const std::string& theAddrR);
    virtual void SetDisplayName(const std::wstring& theDisplayName);
    virtual void SetRequireUniqueDisplayName(bool requireUniqueDisplayName);
    virtual void SetPath(const std::wstring& thePath);
protected:
    bool                         mIsExtended; // Is extended (data-object containing) version of message?
    std::list<std::string>       mDirServerAddressList;
    std::wstring                 mPath;
    std::wstring                 mDisplayName;
    bool                         mRequireUniqueDisplayName;
    WONCommon::DataObjectTypeSet mDataObjects;  // Set of data objects for extended version

    // Pack/Unpack data objects into raw buffer (call in Pack/Unpack()).  Is a NoOp if mExtended is false.
    virtual void PackExtended();
    virtual void UnpackExtended();
};


// Inlines
inline TRawMsg* SMsgCommRegisterRequest::Duplicate(void) const
{ return new SMsgCommRegisterRequest(*this); }

inline bool SMsgCommRegisterRequest::IsExtended() const
{ return mIsExtended; }

inline const std::list<std::string>& SMsgCommRegisterRequest::GetDirServerAddressList(void) const
{ return mDirServerAddressList; }

inline const std::wstring& SMsgCommRegisterRequest::GetDisplayName(void) const
{ return mDisplayName; }

inline bool SMsgCommRegisterRequest::RequireUniqueDisplayName(void) const
{ return mRequireUniqueDisplayName; }

inline const std::wstring& SMsgCommRegisterRequest::GetPath(void) const
{ return mPath; }

inline const WONCommon::DataObjectTypeSet& SMsgCommRegisterRequest::GetDataObjects() const
{ return mDataObjects; }

inline void SMsgCommRegisterRequest::SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR)
{ mDataObjects = theSetR; }

inline void SMsgCommRegisterRequest::AddDataObject(const WONCommon::DataObject& theObjR)
{ mDataObjects.insert(theObjR); }

inline void SMsgCommRegisterRequest::AddDirServerAddress(const std::string& theAddrR)
{ mDirServerAddressList.push_back(theAddrR); }

inline void SMsgCommRegisterRequest::SetDisplayName(const std::wstring& theDisplayName)
{ mDisplayName = theDisplayName; }

inline void SMsgCommRegisterRequest::SetRequireUniqueDisplayName(bool requireUniqueDisplayName)
{ mRequireUniqueDisplayName = requireUniqueDisplayName; }

inline void SMsgCommRegisterRequest::SetPath(const std::wstring& thePath)
{ mPath = thePath; }

};  // Namespace WONMsg

#endif