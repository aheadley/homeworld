#ifndef _SMsgDirG2AddService_H
#define _SMsgDirG2AddService_H

// SMsgDirG2AddService.h

// DirectoryServer add service message.  Adds a new service at specified path.

#include "STRING"
#include "SMsgDirG2UpdateExtendBase.h"


namespace WONMsg {

class SMsgDirG2AddService : public SMsgDirG2UpdateExtendBase
{
public:
    // Default ctor
    explicit SMsgDirG2AddService(bool isExtended=false);

    // SmallMessage ctor
    explicit SMsgDirG2AddService(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2AddService(const SMsgDirG2AddService& theMsgR);

    // Destructor
    ~SMsgDirG2AddService(void);

    // Assignment
    SMsgDirG2AddService& operator=(const SMsgDirG2AddService& theMsgR);

    // Virtual Duplicate
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // EntityFlags access
    unsigned char GetEntityFlags() const;
    void SetEntityFlags(unsigned char theFlags);

    // DisplayName access
    const std::wstring& GetDisplayName() const;
    void SetDisplayName(const std::wstring& theName);

    // Lifespan access
    unsigned long GetLifespan() const;
    void SetLifespan(unsigned long theSpan);

private:
    unsigned char mEntityFlags;  // Flags for add
    std::wstring  mDisplayName;  // Service display name
    unsigned long mLifespan;     // Lifespan in seconds
};


// Inlines
inline TRawMsg*
SMsgDirG2AddService::Duplicate(void) const
{ return new SMsgDirG2AddService(*this); }

inline unsigned char
SMsgDirG2AddService::GetEntityFlags(void) const
{ return mEntityFlags; }

inline void
SMsgDirG2AddService::SetEntityFlags(unsigned char theFlags)
{ mEntityFlags = theFlags; }

inline const std::wstring&
SMsgDirG2AddService::GetDisplayName(void) const
{ return mDisplayName; }

inline void
SMsgDirG2AddService::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline unsigned long
SMsgDirG2AddService::GetLifespan(void) const
{ return mLifespan; }

inline void
SMsgDirG2AddService::SetLifespan(unsigned long theSpan)
{ mLifespan = theSpan; }

};  // Namespace WONMsg

#endif