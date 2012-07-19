#ifndef TMsgEventRecordEvent_H
#define TMsgEventRecordEvent_H

// SMsgEventRecordEvent.h

#include "LIST"
#include <msg/TMessage.h>
#include <db/DBTypes.h>

namespace WONMsg {

class SMsgEventRecordEvent : public SmallMessage {
public:
    enum DataType { NUMBER, STRING, DATE, UNKNOWN };
    struct Detail {
        WONDatabase::DBSeqSmall    mDetailType;  // Type of detail.
        DataType                   mDataType;
        WONDatabase::DBNumber10_2  mNumber;
        WONDatabase::DBVarCharWide mString;
        time_t                     mDate;

        Detail() : mDetailType(0), mDataType(UNKNOWN), mNumber(0), mDate(0) {}
        Detail(WONDatabase::DBSeqSmall theDetailType, WONDatabase::DBNumber10_2 theValue) : mDetailType(theDetailType), mDataType(NUMBER), mNumber(theValue) {}
        Detail(WONDatabase::DBSeqSmall theDetailType, const WONDatabase::DBVarCharWide& theValueR) : mDetailType(theDetailType), mDataType(STRING), mString(theValueR) {}
        //Detail(WONDatabase::DBSeqSmall theDetailType, time_t theValue) : mDetailType(theDetailType), mDataType(DATE), mDate(theValue) {}
    };
    struct Attachment {
        WONDatabase::DBNumber8     mSize;        // Size of the attachment's binary blob.
        WONDatabase::DBVarCharWide mDescription; // Description of the attachment.
        WONDatabase::DBSeqTiny     mContentType; // Identifies what general class of stuff is attached.
        WONDatabase::DBBlob        mBody;        // The actual attachment.  Array of AttachmentSize bytes of data.
    };
    typedef std::list<Attachment> AttachmentList;
    typedef std::list<Detail>     DetailList;

    // Default ctor
    SMsgEventRecordEvent(void);

    // SmallMessage ctor
    explicit SMsgEventRecordEvent(const SmallMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    SMsgEventRecordEvent(const SMsgEventRecordEvent& theMsgR);

    // Destructor
    virtual ~SMsgEventRecordEvent(void);

    // Assignment
    SMsgEventRecordEvent& operator=(const SMsgEventRecordEvent& theMsgR);

    // Virtual Duplicate from SmallMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    void SimplePack(void);
    void SimpleUnpack(void);

    // Dumping
    virtual void Dump(std::ostream& os) const;

    // Member access
    void SetActivityType(WONDatabase::DBSeqSmall theActivityType);
    void SetHasRelatedServer(bool hasRelatedServer);
    void SetHasRelatedClient(bool hasRelatedClient);
    void SetHasRelatedUser(bool hasRelatedUser);
    void SetHasDateTime(bool hasDateTime);

    void AddDetail(const Detail& aDetailR);
    void AddDetail(WONDatabase::DBSeqSmall theType, WONDatabase::DBNumber10_2 theValue);
    void AddDetail(WONDatabase::DBSeqSmall theType, const WONDatabase::DBVarCharWide& theValueR);
//  void AddDetail(WONDatabase::DBSeqSmall theType, time_t theValue);
    void AddAttachment(const Attachment& anAttachmentR);
    void AddAttachment(WONDatabase::DBNumber8 theSize, const WONDatabase::DBVarCharWide& theDescriptionR, WONDatabase::DBSeqTiny theContentType, const WONDatabase::DBBlob& theBodyR);

    void SetActivityDateTime(time_t theActivityDateTime);

    void SetClientName(const WONDatabase::DBNameStandard& theClientNameR);
    void SetClientNetAddress(const WONDatabase::DBDescStandard& theClientNetAddressR);

    void SetServerType(WONDatabase::DBSeqSmall theServerType);
    void SetServerLogicalName(const WONDatabase::DBNameStandard& theLogicalNameR);
    void SetServerNetAddress(const WONDatabase::DBDescStandard& theServerNetAddressR);

    void SetUserAuthenticationMethod(WONDatabase::DBSeqSmall theUserAuthenticationMethod);
    void SetUserId(WONDatabase::DBSeqIdentifier theUserId);
    void SetUserName(const WONDatabase::DBNameStandard& theUserNameR);

    WONDatabase::DBSeqSmall GetActivityType() const;
    bool HasRelatedServer() const;
    bool HasRelatedClient() const;
    bool HasRelatedUser() const;
    bool HasDateTime() const;

    const DetailList& GetDetailList() const;
    const AttachmentList& GetAttachmentList() const;

    time_t GetActivityDateTime() const;

    const WONDatabase::DBNameStandard& GetClientName() const;
    const WONDatabase::DBDescStandard& GetClientNetAddress() const;

    WONDatabase::DBSeqSmall GetServerType() const;
    const WONDatabase::DBNameStandard& GetServerLogicalName() const;
    const WONDatabase::DBDescStandard& GetServerNetAddress() const;

    WONDatabase::DBSeqSmall GetUserAuthenticationMethod() const;
    WONDatabase::DBSeqIdentifier GetUserId() const;
    const WONDatabase::DBNameStandard& GetUserName() const;

protected:
    WONDatabase::DBSeqSmall mActivityType;     // Indicates type of activity (server startup, client connect, etc.)
    bool                    mHasRelatedServer; // TRUE if data regarding a server is included in this message.
    bool                    mHasRelatedClient; // TRUE if data regarding a client is included in this message.
    bool                    mHasRelatedUser;   // TRUE if data regarding a user is included in this message.
    bool                    mHasDateTime;      // TRUE if a date-time is included in this message

    DetailList      mDetailList; // Details related to this event
    AttachmentList  mAttachmentList;  // Attachments related to this event

    time_t mActivityDateTime; // Indicates the date and time when the event occurred.

    WONDatabase::DBNameStandard mClientName;
    WONDatabase::DBDescStandard mClientNetAddress;

    WONDatabase::DBSeqSmall     mServerType;
    WONDatabase::DBNameStandard mServerLogicalName;
    WONDatabase::DBDescStandard mServerNetAddress;

    WONDatabase::DBSeqSmall      mUserAuthenticationMethod; // Indicates the user authentication method related to this activity entry.
    WONDatabase::DBSeqIdentifier mUserId;                   // Id of the user corresponding to this activity entry.
    WONDatabase::DBNameStandard  mUserName;                 // Name of the user corresponding to this activity entry.
protected:
    void AppendNumber8(WONDatabase::DBNumber8 theNumber8);
    void AppendNumber10_2(WONDatabase::DBNumber10_2 theNumber10_2);
    void AppendSeqSmall(WONDatabase::DBSeqSmall theSeqSmall);
    void AppendSeqTiny(WONDatabase::DBSeqTiny theSeqTiny);
    void AppendSeqIdentifier(WONDatabase::DBSeqIdentifier theSeqId);

    WONDatabase::DBNumber8       ReadNumber8() const;
    WONDatabase::DBNumber10_2    ReadNumber10_2() const;
    WONDatabase::DBSeqSmall      ReadSeqSmall() const;
    WONDatabase::DBSeqTiny       ReadSeqTiny() const;
    WONDatabase::DBSeqIdentifier ReadSeqIdentifier() const;
};


// Inlines
inline TRawMsg* SMsgEventRecordEvent::Duplicate(void) const
    { return new SMsgEventRecordEvent(*this); }
inline void SMsgEventRecordEvent::SetActivityType(WONDatabase::DBSeqSmall theActivityType)
    { mActivityType = theActivityType; }
inline void SMsgEventRecordEvent::SetActivityDateTime(time_t theActivityDateTime)
    { mActivityDateTime = theActivityDateTime; }
inline void SMsgEventRecordEvent::SetHasRelatedServer(bool hasRelatedServer)
    { mHasRelatedServer = hasRelatedServer; }
inline void SMsgEventRecordEvent::SetHasRelatedClient(bool hasRelatedClient)
    { mHasRelatedClient = hasRelatedClient; }
inline void SMsgEventRecordEvent::SetHasRelatedUser(bool hasRelatedUser)
    { mHasRelatedUser = hasRelatedUser; }
inline void SMsgEventRecordEvent::SetHasDateTime(bool hasDateTime)
    { mHasDateTime = hasDateTime; }
inline void SMsgEventRecordEvent::AddDetail(const Detail& aDetailR)
    { mDetailList.push_back(aDetailR); }
inline void SMsgEventRecordEvent::AddDetail(WONDatabase::DBSeqSmall theType, WONDatabase::DBNumber10_2 theValue)
    { AddDetail(Detail(theType, theValue)); }
inline void SMsgEventRecordEvent::AddDetail(WONDatabase::DBSeqSmall theType, const WONDatabase::DBVarCharWide& theValueR)
    { AddDetail(Detail(theType, theValueR)); }
inline void SMsgEventRecordEvent::AddAttachment(const Attachment& anAttachmentR)
    { mAttachmentList.push_back(anAttachmentR); }
inline void SMsgEventRecordEvent::SetClientName(const WONDatabase::DBNameStandard& theClientNameR)
    { mClientName = theClientNameR; }
inline void SMsgEventRecordEvent::SetClientNetAddress(const WONDatabase::DBDescStandard& theClientNetAddressR)
    { mClientNetAddress = theClientNetAddressR; }
inline void SMsgEventRecordEvent::SetServerType(WONDatabase::DBSeqSmall theServerType)
    { mServerType = theServerType; }
inline void SMsgEventRecordEvent::SetServerLogicalName(const WONDatabase::DBNameStandard& theServerLogicalNameR)
    { mServerLogicalName = theServerLogicalNameR; }
inline void SMsgEventRecordEvent::SetServerNetAddress(const WONDatabase::DBDescStandard& theServerNetAddressR)
    { mServerNetAddress = theServerNetAddressR; }
inline void SMsgEventRecordEvent::SetUserAuthenticationMethod(WONDatabase::DBSeqSmall theUserAuthenticationMethod)
    { mUserAuthenticationMethod = theUserAuthenticationMethod; }
inline void SMsgEventRecordEvent::SetUserId(WONDatabase::DBSeqIdentifier theUserId)
    { mUserId = theUserId; }
inline void SMsgEventRecordEvent::SetUserName(const WONDatabase::DBNameStandard& theUserNameR)
    { mUserName = theUserNameR; }
inline WONDatabase::DBSeqSmall SMsgEventRecordEvent::GetActivityType() const
    { return mActivityType; }
inline time_t SMsgEventRecordEvent::GetActivityDateTime() const
    { return mActivityDateTime; }
inline bool SMsgEventRecordEvent::HasRelatedServer() const
    { return mHasRelatedServer; }
inline bool SMsgEventRecordEvent::HasRelatedClient() const
    { return mHasRelatedClient; }
inline bool SMsgEventRecordEvent::HasRelatedUser() const
    { return mHasRelatedUser; }
inline bool SMsgEventRecordEvent::HasDateTime() const
    { return mHasDateTime; }
inline const SMsgEventRecordEvent::DetailList& SMsgEventRecordEvent::GetDetailList() const
    { return mDetailList; }
inline const SMsgEventRecordEvent::AttachmentList& SMsgEventRecordEvent::GetAttachmentList() const
    { return mAttachmentList; }
inline const WONDatabase::DBNameStandard& SMsgEventRecordEvent::GetClientName() const
    { return mClientName; }
inline const WONDatabase::DBDescStandard& SMsgEventRecordEvent::GetClientNetAddress() const
    { return mClientNetAddress; }
inline WONDatabase::DBSeqSmall SMsgEventRecordEvent::GetServerType() const
    { return mServerType; }
inline const WONDatabase::DBNameStandard& SMsgEventRecordEvent::GetServerLogicalName() const
    { return mServerLogicalName; }
inline const WONDatabase::DBDescStandard& SMsgEventRecordEvent::GetServerNetAddress() const
    { return mServerNetAddress; }
inline WONDatabase::DBSeqSmall SMsgEventRecordEvent::GetUserAuthenticationMethod() const
    { return mUserAuthenticationMethod; }
inline WONDatabase::DBSeqIdentifier SMsgEventRecordEvent::GetUserId() const
    { return mUserId; }
inline const WONDatabase::DBNameStandard& SMsgEventRecordEvent::GetUserName() const
    { return mUserName; }


inline void SMsgEventRecordEvent::AppendNumber8(WONDatabase::DBNumber8 theNumber8)
    { AppendLong(theNumber8); }
inline void SMsgEventRecordEvent::AppendNumber10_2(WONDatabase::DBNumber10_2 theNumber10_2)
    { AppendBytes(sizeof(WONDatabase::DBNumber10_2), &theNumber10_2); }
inline void SMsgEventRecordEvent::AppendSeqSmall(WONDatabase::DBSeqSmall theSeqSmall)
    { AppendShort(theSeqSmall); }
inline void SMsgEventRecordEvent::AppendSeqTiny(WONDatabase::DBSeqTiny theSeqTiny)
    { AppendByte(theSeqTiny); }
inline void SMsgEventRecordEvent::AppendSeqIdentifier(WONDatabase::DBSeqIdentifier theSeqId)
    { AppendLong(theSeqId); }

inline WONDatabase::DBNumber8       SMsgEventRecordEvent::ReadNumber8() const
    { return ReadLong(); }
inline WONDatabase::DBNumber10_2    SMsgEventRecordEvent::ReadNumber10_2() const
    { double aDouble = *(double*)ReadBytes(sizeof(double)); return aDouble; }
inline WONDatabase::DBSeqSmall      SMsgEventRecordEvent::ReadSeqSmall() const
    { return ReadShort(); }
inline WONDatabase::DBSeqTiny       SMsgEventRecordEvent::ReadSeqTiny() const
    { return ReadByte(); }
inline WONDatabase::DBSeqIdentifier SMsgEventRecordEvent::ReadSeqIdentifier() const
    { return ReadLong(); }


};  // Namespace WONMsg

#endif // TMsgEventRecordEvent_H