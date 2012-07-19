#ifndef _TMsgDirServiceBase_H
#define _TMsgDirServiceBase_H

// TMsgDirServiceBase.h

// Base class for directory server entry messages.  This class is not a
// directory server message itself.  It is just further refinement of
// TMessage for use in directory messages.


#include "STRING"
#include "TMsgDirPeerDataBase.h"

namespace WONMsg {

class TMsgDirServiceBase : public TMsgDirPeerDataBase
{
public:
    // Default ctor
    TMsgDirServiceBase(void);

    // TMessage ctor
    explicit TMsgDirServiceBase(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirServiceBase(const TMsgDirServiceBase& theMsgR);

    // Destructor
    virtual ~TMsgDirServiceBase(void);

    // Assignment
    TMsgDirServiceBase& operator=(const TMsgDirServiceBase& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const std::wstring& GetDirectoryPath(void) const;
    const std::wstring& GetName(void) const;
    const std::wstring& GetDisplayName(void) const;
    const std::wstring& GetVersion(void) const;
    const std::wstring& GetProtoName(void) const;
    const std::wstring& GetProtoVersion(void) const;
    const std::wstring& GetNetAddress(void) const;

    virtual void SetDirectoryPath(const std::wstring& thePath);
    virtual void SetName(const std::wstring& theName);
    virtual void SetDisplayName(const std::wstring& theName);
    virtual void SetVersion(const std::wstring& theVersion);
    virtual void SetProtoName(const std::wstring& theName);
    virtual void SetProtoVersion(const std::wstring& theVersion);
    virtual void SetNetAddress(const std::wstring& theAddr);

    // Are any extended fields defined?
    bool HasExtendedInfo() const;

protected:
    std::wstring mDirectoryPath;
    std::wstring mName;
    std::wstring mDisplayName;
    std::wstring mVersion;
    std::wstring mProtoName;
    std::wstring mProtoVersion;
    std::wstring mNetAddress;
};


// Inlines
inline TRawMsg*
TMsgDirServiceBase::Duplicate(void) const
{ return new TMsgDirServiceBase(*this); }

inline const std::wstring&
TMsgDirServiceBase::GetDirectoryPath(void) const
{ return mDirectoryPath; }

inline const std::wstring&
TMsgDirServiceBase::GetName(void) const
{ return mName; }

inline const std::wstring&
TMsgDirServiceBase::GetDisplayName(void) const
{ return mDisplayName; }

inline const std::wstring&
TMsgDirServiceBase::GetVersion(void) const
{ return mVersion; }

inline const std::wstring&
TMsgDirServiceBase::GetProtoName(void) const
{ return mProtoName; }

inline const std::wstring&
TMsgDirServiceBase::GetProtoVersion(void) const
{ return mProtoVersion; }

inline const std::wstring&
TMsgDirServiceBase::GetNetAddress(void) const
{ return mNetAddress; }

inline void
TMsgDirServiceBase::SetDirectoryPath(const std::wstring& thePath)
{ mDirectoryPath = thePath; }

inline void
TMsgDirServiceBase::SetName(const std::wstring& theName)
{ mName = theName; }

inline void
TMsgDirServiceBase::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline void
TMsgDirServiceBase::SetVersion(const std::wstring& theVersion)
{ mVersion = theVersion; }

inline void
TMsgDirServiceBase::SetProtoName(const std::wstring& theName)
{ mProtoName = theName; }

inline void
TMsgDirServiceBase::SetProtoVersion(const std::wstring& theVersion)
{ mProtoVersion = theVersion; }

inline void
TMsgDirServiceBase::SetNetAddress(const std::wstring& theAddr)
{ mNetAddress = theAddr; }

inline bool
TMsgDirServiceBase::HasExtendedInfo() const
{ return ((! mVersion.empty()) || (! mProtoName.empty()) || (! mProtoVersion.empty())); }

};  // Namespace WONMsg

#endif