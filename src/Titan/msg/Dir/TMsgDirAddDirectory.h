#ifndef _TMsgDirAddDirectory_H
#define _TMsgDirAddDirectory_H

// TMsgDirAddDirectory.h

// DirectoryServer Add SubDirectory message.  Defines a subdirectory to add.

#include "STRING"
#include "msg/TMessage.h"
#include "TMsgDirDirectoryBase.h"

namespace WONMsg {

class TMsgDirAddDirectory : public TMsgDirDirectoryBase
{
public:
    // Default ctor
    TMsgDirAddDirectory(void);

    // TMessage ctor
    explicit TMsgDirAddDirectory(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirAddDirectory(const TMsgDirAddDirectory& theMsgR);

    // Destructor
    ~TMsgDirAddDirectory(void);

    // Assignment
    TMsgDirAddDirectory& operator=(const TMsgDirAddDirectory& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Member access
    const std::wstring& GetName(void) const;
    const std::wstring& GetDisplayName(void) const;
    bool                GetVisible(void) const;
    unsigned long       GetLifespan(void) const;

    void SetName(const std::wstring& theName);
    void SetDisplayName(const std::wstring& theName);
    void SetVisible(bool theFlag);
    void SetLifespan(unsigned long theLifespan);

private:
    std::wstring  mName;
    std::wstring  mDisplayName;
    bool          mVisible;
    unsigned long mLifespan;
};


// Inlines
inline TRawMsg*
TMsgDirAddDirectory::Duplicate(void) const
{ return new TMsgDirAddDirectory(*this); }

inline const std::wstring&
TMsgDirAddDirectory::GetName(void) const
{ return mName; }

inline const std::wstring&
TMsgDirAddDirectory::GetDisplayName(void) const
{ return mDisplayName; }

inline bool
TMsgDirAddDirectory::GetVisible(void) const
{ return mVisible; }

inline unsigned long
TMsgDirAddDirectory::GetLifespan(void) const
{ return mLifespan; }

inline void
TMsgDirAddDirectory::SetName(const std::wstring& theName)
{ mName = theName; }

inline void
TMsgDirAddDirectory::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline void
TMsgDirAddDirectory::SetVisible(bool theFlag)
{ mVisible = theFlag; }

inline void
TMsgDirAddDirectory::SetLifespan(unsigned long theLifespan)
{ mLifespan = theLifespan; }

};  // Namespace WONMsg

#endif