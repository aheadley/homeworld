#ifndef _TMsgDirDirectoryBase_H
#define _TMsgDirDirectoryBase_H

// TMsgDirDirectoryBase.h

// Base class for directory server sub directory messages.  This class is
// not a directory server message itself.  It is just further refinement
// of TMessage for use in directory messages.


#include "STRING"
#include "TMsgDirPeerDataBase.h"

namespace WONMsg {

class TMsgDirDirectoryBase : public TMsgDirPeerDataBase
{
public:
    // Default ctor
    TMsgDirDirectoryBase(void);

    // TMessage ctor
    explicit TMsgDirDirectoryBase(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirDirectoryBase(const TMsgDirDirectoryBase& theMsgR);

    // Destructor
    virtual ~TMsgDirDirectoryBase(void);

    // Assignment
    TMsgDirDirectoryBase& operator=(const TMsgDirDirectoryBase& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const std::wstring& GetDirectoryPath(void) const;
    virtual void SetDirectoryPath(const std::wstring& thePath);

protected:
    std::wstring mDirectoryPath;
};


// Inlines
inline TRawMsg*
TMsgDirDirectoryBase::Duplicate(void) const
{ return new TMsgDirDirectoryBase(*this); }

inline const std::wstring&
TMsgDirDirectoryBase::GetDirectoryPath(void) const
{ return mDirectoryPath; }

inline void
TMsgDirDirectoryBase::SetDirectoryPath(const std::wstring& thePath)
{ mDirectoryPath = thePath; }

};  // Namespace WONMsg

#endif