#if !defined(TMsgFactStartProcessUnicode_H)
#define TMsgFactStartProcessUnicode_H

// TMsgFactStartProcessUnicode.h

// Message that is used to start a process via the Factory Server


#include "TMsgFactStartProcessBase.h"
#include "SET"


namespace WONMsg {

    class TMsgFactStartProcessUnicode : public TMsgFactStartProcessBase {

public:
    // Default ctor
    TMsgFactStartProcessUnicode(void);

    // TMessage ctor
    explicit TMsgFactStartProcessUnicode(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactStartProcessUnicode(const TMsgFactStartProcessUnicode& theMsgR);

    // Destructor
    virtual ~TMsgFactStartProcessUnicode(void);

    // Assignment
    TMsgFactStartProcessUnicode& operator=(const TMsgFactStartProcessUnicode& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const std::wstring& GetCmdLine(void) const;
    virtual void SetCmdLine(const std::wstring& theCmdLine);
protected:
    virtual void PackCommandLine();
    virtual void UnpackCommandLine();

    std::wstring mCmdLine;
};


// Inlines
inline TRawMsg* TMsgFactStartProcessUnicode::Duplicate(void) const
{ return new TMsgFactStartProcessUnicode(*this); }

inline const std::wstring& TMsgFactStartProcessUnicode::GetCmdLine(void) const
{ return mCmdLine; }
inline void TMsgFactStartProcessUnicode::SetCmdLine(const std::wstring& theCmdLine)
{ mCmdLine = theCmdLine; }

};  // Namespace WONMsg

#endif