#ifndef _SMsgDirG2KeyedBase_H
#define _SMsgDirG2KeyedBase_H

// SMsgDirG2KeyedBase.h

// BAse class that handles packing and unpacking entity key fields.  Provides
// KeyType and key fields.  KeyType controls which key fields are active as follows:

//      KeyType         Appliable Key Fields
//      --------------  ---------------------
//      KT_DIRECTORY    Path
//      KT_SERVICE      Path, Name, NetAddress

// Provides PackKey() and UnpackKey() methods to pack/unpack the key.  These methods
// pack/unpack appropriate key fields based upon the KeyType.  These methods should
// be called be derived class Pack/Unpack at the appropriate point to pack/unpack
// key fields if needed.

// Note that this base class IS NOT derived from SmallMessage.  It is meant to be
// multiply inherited by classes already dervied fromn SmallMessage to add the keyed
// atributes to a message.


#include "STRING"
#include "common/won.h"


namespace WONMsg {

// Forwards
class BaseMessage;


class SMsgDirG2KeyedBase
{
public:
    // Types
    enum KeyType { KT_DIRECTORY, KT_SERVICE };

    // Default ctor
    explicit SMsgDirG2KeyedBase(KeyType theType);

    // Copy ctor
    SMsgDirG2KeyedBase(const SMsgDirG2KeyedBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2KeyedBase();

    // Assignment
    SMsgDirG2KeyedBase& operator=(const SMsgDirG2KeyedBase& theDataR);

    // KeyType access - Read Only
    KeyType GetKeyType() const;

    // Path access - appliable for all key types
    const std::wstring& GetPath() const;
    void SetPath(const std::wstring& thePath);

    // Name access - appliable to KT_SERVICE only
    const std::wstring& GetName() const;
    void SetName(const std::wstring& theName);

    // NetAddress access - appliable to KT_SERVICE only
    const WONCommon::RawBuffer& GetNetAddress() const;
    void SetNetAddress(const WONCommon::RawBuffer& theAddr);

protected:
    KeyType              mKeyType;     // KeyType
    std::wstring         mPath;        // Path to directory/service
    std::wstring         mName;        // Service name
    WONCommon::RawBuffer mNetAddress;  // Service net address

    // Pack key into raw buffer (call in Pack()).
    virtual void PackKey(BaseMessage& theMsgR);

    // Unpack key from raw buffer (call in Unpack()).
    virtual void UnpackKey(BaseMessage& theMsgR);

    // Set the KeyType - call in derived Unpack() based on the messsage type
    void SetKeyType(KeyType theType);
};


// Inlines
inline SMsgDirG2KeyedBase::KeyType
SMsgDirG2KeyedBase::GetKeyType() const
{ return mKeyType; }

inline void
SMsgDirG2KeyedBase::SetKeyType(KeyType theType)
{ mKeyType = theType; }

inline const std::wstring&
SMsgDirG2KeyedBase::GetPath() const
{ return mPath; }

inline void
SMsgDirG2KeyedBase::SetPath(const std::wstring& thePath)
{ mPath = thePath; }

inline const std::wstring&
SMsgDirG2KeyedBase::GetName() const
{ return mName; }

inline void
SMsgDirG2KeyedBase::SetName(const std::wstring& theName)
{ mName = theName; }

inline const WONCommon::RawBuffer&
SMsgDirG2KeyedBase::GetNetAddress() const
{ return mNetAddress; }

inline void
SMsgDirG2KeyedBase::SetNetAddress(const WONCommon::RawBuffer& theAddr)
{ mNetAddress = theAddr; }

};  // Namespace WONMsg

#endif