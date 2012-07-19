#ifndef _TMsgDirChangeDirectory_H
#define _TMsgDirChangeDirectory_H

// TMsgDirChangeDirectory.h

// DirectoryServer Change SubDirectory message.  Defines a subdirectory to change.

#include "STRING"
#include "msg/TMessage.h"
#include "TMsgDirDirectoryBase.h"

namespace WONMsg {

class TMsgDirChangeDirectory : public TMsgDirDirectoryBase
{
public:
    // Default ctor
    TMsgDirChangeDirectory(void);

    // TMessage ctor
    explicit TMsgDirChangeDirectory(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirChangeDirectory(const TMsgDirChangeDirectory& theMsgR);

    // Destructor
    ~TMsgDirChangeDirectory(void);

    // Assignment
    TMsgDirChangeDirectory& operator=(const TMsgDirChangeDirectory& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Member access
    const std::wstring& GetNewName(void) const;
    const std::wstring& GetNewDisplayName(void) const;
    unsigned char       GetNewVisible(void) const;
    unsigned long       GetNewLifespan(void) const;

    void SetNewName(const std::wstring& theName);
    void SetNewDisplayName(const std::wstring& theName);
    void SetNewVisible(unsigned char theFlag);
    void SetNewLifespan(unsigned long theLifespan);

private:
    std::wstring  mNewName;
    std::wstring  mNewDisplayName;
    int           mNewVisible;
    unsigned long mNewLifespan;
};


// Inlines
inline TRawMsg*
TMsgDirChangeDirectory::Duplicate(void) const
{ return new TMsgDirChangeDirectory(*this); }

inline const std::wstring&
TMsgDirChangeDirectory::GetNewName(void) const
{ return mNewName; }

inline const std::wstring&
TMsgDirChangeDirectory::GetNewDisplayName(void) const
{ return mNewDisplayName; }

inline unsigned char
TMsgDirChangeDirectory::GetNewVisible(void) const
{ return mNewVisible; }

inline unsigned long
TMsgDirChangeDirectory::GetNewLifespan(void) const
{ return mNewLifespan; }

inline void
TMsgDirChangeDirectory::SetNewName(const std::wstring& theName)
{ mNewName = theName; }

inline void
TMsgDirChangeDirectory::SetNewDisplayName(const std::wstring& theName)
{ mNewDisplayName = theName; }

inline void
TMsgDirChangeDirectory::SetNewVisible(unsigned char theFlag)
{ mNewVisible = theFlag; }

inline void
TMsgDirChangeDirectory::SetNewLifespan(unsigned long theLifespan)
{ mNewLifespan = theLifespan; }

};  // Namespace WONMsg

#endif