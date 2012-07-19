#if !defined(TMsgFactStartProcessBase_H)
#define TMsgFactStartProcessBase_H

// TMsgFactStartProcessBase.h

// Message that is used to start a process via the Factory Server


#include "msg/TMessage.h"
#include "SET"


namespace WONMsg {

    typedef std::set<std::string> FACT_SERV_TRUSTED_ADDRESS_SET;
    typedef std::set<unsigned short> FACT_SERV_PORT_RANGE_SET;


class TMsgFactStartProcessBase : public TMessage {

public:
    // Default ctor
    TMsgFactStartProcessBase(void);

    // TMessage ctor
    explicit TMsgFactStartProcessBase(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactStartProcessBase(const TMsgFactStartProcessBase& theMsgR);

    // Destructor
    virtual ~TMsgFactStartProcessBase(void);

    // Assignment
    TMsgFactStartProcessBase& operator=(const TMsgFactStartProcessBase& theMsgR);

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const std::string& GetProcessName(void) const;
    bool GetAddCmdLine(void) const;
    const std::string& GetCmdLine(void) const;
    long GetWaitTime(void) const;
    const std::string& GetDirServerAddress(void) const;
    const std::wstring& GetDisplayName(void) const;
    const std::wstring& GetRegisterDir(void) const;
    bool GetAbortRegFail(void) const;
    unsigned char GetTotalPorts(void) const;
    const FACT_SERV_PORT_RANGE_SET& GetPortSet(void) const;
    const FACT_SERV_TRUSTED_ADDRESS_SET& GetAuthorizedIPSet(void) const;
    bool IsDirServerAddressEmpty(void) const;
    const void* GetBinaryBlob(unsigned short* size = 0) const;
    const void* GetBinaryBlob(unsigned short& size) const;

    virtual void SetProcessName(const std::string& theProcessName);
    virtual void SetAddCmdLine(bool theAddCmdLine);
    virtual void SetCmdLine(const std::string& theCmdLine);
    virtual void SetWaitTime(long theWaitTime);
    virtual void SetDirServerAddress(const std::string& theDirServerAdress);
    virtual void SetDisplayName(const std::wstring& theDisplayName);
    virtual void SetRegisterDir(const std::wstring& theRegisterDir);
    virtual void SetAbortRegFail(bool theAbortRegFail);
    virtual void SetTotalPorts(unsigned char theTotalPorts);
    virtual void SetPortSet(const FACT_SERV_PORT_RANGE_SET& thePortSet);
    virtual void SetAuthorizedIPSet(const FACT_SERV_TRUSTED_ADDRESS_SET& theAuthorizedIPSet);
    virtual void SetBinaryBlob(const void* blob, unsigned short size);

protected:
    virtual void PackCommandLine() =0;
    virtual void UnpackCommandLine() =0;

    std::string                   mProcessName;
    bool                          mAddCmdLine;
    std::string                   mCmdLine;
    long                          mWaitTime;
    std::string                   mDirServerAdress;
    std::wstring                  mDisplayName;
    std::wstring                  mRegisterDir;
    bool                          mAbortRegFail;
    unsigned char                 mTotalPorts;
    FACT_SERV_PORT_RANGE_SET      mPortSet;
    FACT_SERV_TRUSTED_ADDRESS_SET mAuthorizedIPSet;
    bool                          mEmptyDirServerAddress;
    const void*                   mBinaryBlob;
    unsigned short                 mBinaryBlobSize;
    TMessage                      mSaveMsg; // temp copy, so mBinaryBlob isn't invalidated
};


// Inlines
inline bool TMsgFactStartProcessBase::IsDirServerAddressEmpty(void) const
{ return mEmptyDirServerAddress; }

inline const std::string& TMsgFactStartProcessBase::GetProcessName(void) const
{ return mProcessName; }

inline bool TMsgFactStartProcessBase::GetAddCmdLine(void) const
{ return mAddCmdLine; }

inline const std::string& TMsgFactStartProcessBase::GetCmdLine(void) const
{ return mCmdLine; }

inline long TMsgFactStartProcessBase::GetWaitTime(void) const
{ return mWaitTime; }

inline const std::string& TMsgFactStartProcessBase::GetDirServerAddress(void) const
{ return mDirServerAdress; }

inline const std::wstring& TMsgFactStartProcessBase::GetDisplayName(void) const
{ return mDisplayName; }

inline const std::wstring& TMsgFactStartProcessBase::GetRegisterDir(void) const
{ return mRegisterDir; }

inline bool TMsgFactStartProcessBase::GetAbortRegFail(void) const
{ return mAbortRegFail; }

inline unsigned char TMsgFactStartProcessBase::GetTotalPorts(void) const
{ return mTotalPorts; }

inline const FACT_SERV_PORT_RANGE_SET& TMsgFactStartProcessBase::GetPortSet(void) const
{ return mPortSet; }

inline const FACT_SERV_TRUSTED_ADDRESS_SET& TMsgFactStartProcessBase::GetAuthorizedIPSet(void) const
{ return mAuthorizedIPSet; }

inline const void* TMsgFactStartProcessBase::GetBinaryBlob(unsigned short* size) const
{ if (size) *size = mBinaryBlobSize; return mBinaryBlob; }

inline const void* TMsgFactStartProcessBase::GetBinaryBlob(unsigned short& size) const
{ size = mBinaryBlobSize; return mBinaryBlob; }

inline void TMsgFactStartProcessBase::SetProcessName(const std::string& theProcessName)
{ mProcessName = theProcessName; }

inline void TMsgFactStartProcessBase::SetAddCmdLine(bool theAddCmdLine)
{ mAddCmdLine = theAddCmdLine; }

inline void TMsgFactStartProcessBase::SetCmdLine(const std::string& theCmdLine)
{ mCmdLine = theCmdLine; }

inline void TMsgFactStartProcessBase::SetWaitTime(long theWaitTime)
{ mWaitTime = theWaitTime; }

inline void TMsgFactStartProcessBase::SetDirServerAddress(const std::string& theDirServerAdress)
{ mDirServerAdress = theDirServerAdress; }

inline void TMsgFactStartProcessBase::SetDisplayName(const std::wstring& theDisplayName)
{ mDisplayName = theDisplayName; }

inline void TMsgFactStartProcessBase::SetRegisterDir(const std::wstring& theRegisterDir)
{ mRegisterDir = theRegisterDir; }

inline void TMsgFactStartProcessBase::SetAbortRegFail(bool theAbortRegFail)
{ mAbortRegFail = theAbortRegFail; }

inline void TMsgFactStartProcessBase::SetTotalPorts(unsigned char theTotalPorts)
{ mTotalPorts = theTotalPorts; }

inline void TMsgFactStartProcessBase::SetPortSet(const FACT_SERV_PORT_RANGE_SET& thePortSet)
{ mPortSet = thePortSet; }

inline void TMsgFactStartProcessBase::SetAuthorizedIPSet(const FACT_SERV_TRUSTED_ADDRESS_SET& theAuthorizedIPSet)
{ mAuthorizedIPSet = theAuthorizedIPSet; }

inline void TMsgFactStartProcessBase::SetBinaryBlob(const void* blob, unsigned short size)
{ mBinaryBlob = blob; mBinaryBlobSize = size; }



};  // Namespace WONMsg

#endif