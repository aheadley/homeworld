#ifndef __ALLMSGSTARTPROCESSBASE_H__
#define __ALLMSGSTARTPROCESSBASE_H__

#include "common/won.h"

namespace WONMsg
{

typedef std::set<std::string> FACT_SERV_TRUSTED_ADDRESS_SET;
typedef std::set<unsigned short> FACT_SERV_PORT_RANGE_SET;

class AllMsgStartProcessBase
{
public:

	AllMsgStartProcessBase() : mWaitTime(0), mAddCmdLine(true), mAbortRegFail(false), mTotalPorts(0) { }
	// Member access
	const std::string& GetProcessName(void) const;
	bool GetAddCmdLine(void) const;
	const std::wstring& GetCmdLine(void) const;
	long GetWaitTime(void) const;
	const std::string& GetDirServerAddress(void) const;
	const std::wstring& GetDisplayName(void) const;
	const std::wstring& GetRegisterDir(void) const;
	bool GetAbortRegFail(void) const;
	unsigned char GetTotalPorts(void) const;
	const FACT_SERV_PORT_RANGE_SET& GetPortSet(void) const;
	const FACT_SERV_TRUSTED_ADDRESS_SET& GetAuthorizedIPSet(void) const;
	bool IsDirServerAddressEmpty(void) const;

	void SetCmdLine(const std::wstring& theCmdLine);
	void SetCmdLine(const std::string& theCmdLine);
	void SetProcessName(const std::string& theProcessName);
	void SetAddCmdLine(bool theAddCmdLine);
	void SetWaitTime(long theWaitTime);
	void SetDirServerAddress(const std::string& theDirServerAdress);
	void SetDisplayName(const std::wstring& theDisplayName);
	void SetRegisterDir(const std::wstring& theRegisterDir);
	void SetAbortRegFail(bool theAbortRegFail);
	void SetTotalPorts(unsigned char theTotalPorts);
	void SetPortSet(const FACT_SERV_PORT_RANGE_SET& thePortSet);
	void SetAuthorizedIPSet(const FACT_SERV_TRUSTED_ADDRESS_SET& theAuthorizedIPSet);

protected:

	std::string                   mProcessName;
	bool                          mAddCmdLine;
	std::wstring                  mCmdLine;
	long                          mWaitTime;
	std::string                   mDirServerAdress;
	std::wstring                  mDisplayName;
	std::wstring                  mRegisterDir;
	bool                          mAbortRegFail;
	unsigned char                 mTotalPorts;
	FACT_SERV_PORT_RANGE_SET      mPortSet;
	FACT_SERV_TRUSTED_ADDRESS_SET mAuthorizedIPSet;
	bool                          mEmptyDirServerAddress;

};

// Inlines
inline bool AllMsgStartProcessBase::IsDirServerAddressEmpty(void) const 
{ return mEmptyDirServerAddress; }

inline const std::string& AllMsgStartProcessBase::GetProcessName(void) const
{ return mProcessName; }

inline bool AllMsgStartProcessBase::GetAddCmdLine(void) const
{ return mAddCmdLine; }

inline const std::wstring& AllMsgStartProcessBase::GetCmdLine(void) const
{ return mCmdLine; }

inline long AllMsgStartProcessBase::GetWaitTime(void) const
{ return mWaitTime; }

inline const std::string& AllMsgStartProcessBase::GetDirServerAddress(void) const
{ return mDirServerAdress; }

inline const std::wstring& AllMsgStartProcessBase::GetDisplayName(void) const
{ return mDisplayName; }

inline const std::wstring& AllMsgStartProcessBase::GetRegisterDir(void) const
{ return mRegisterDir; }

inline bool AllMsgStartProcessBase::GetAbortRegFail(void) const
{ return mAbortRegFail; }

inline unsigned char AllMsgStartProcessBase::GetTotalPorts(void) const
{ return mTotalPorts; }

inline const FACT_SERV_PORT_RANGE_SET& AllMsgStartProcessBase::GetPortSet(void) const
{ return mPortSet; }

inline const FACT_SERV_TRUSTED_ADDRESS_SET& AllMsgStartProcessBase::GetAuthorizedIPSet(void) const
{ return mAuthorizedIPSet; }

inline void AllMsgStartProcessBase::SetProcessName(const std::string& theProcessName)
{ mProcessName = theProcessName; }

inline void AllMsgStartProcessBase::SetAddCmdLine(bool theAddCmdLine)
{ mAddCmdLine = theAddCmdLine; }

inline void AllMsgStartProcessBase::SetCmdLine(const std::string& theCmdLine)
{ mCmdLine = WONCommon::StringToWString(theCmdLine); }

inline void AllMsgStartProcessBase::SetCmdLine(const std::wstring& theCmdLine)
{ mCmdLine = theCmdLine; }

inline void AllMsgStartProcessBase::SetWaitTime(long theWaitTime)
{ mWaitTime = theWaitTime; }

inline void AllMsgStartProcessBase::SetDirServerAddress(const std::string& theDirServerAdress)
{ mDirServerAdress = theDirServerAdress; }

inline void AllMsgStartProcessBase::SetDisplayName(const std::wstring& theDisplayName)
{ mDisplayName = theDisplayName; }

inline void AllMsgStartProcessBase::SetRegisterDir(const std::wstring& theRegisterDir)
{ mRegisterDir = theRegisterDir; }

inline void AllMsgStartProcessBase::SetAbortRegFail(bool theAbortRegFail)
{ mAbortRegFail = theAbortRegFail; }

inline void AllMsgStartProcessBase::SetTotalPorts(unsigned char theTotalPorts)
{ mTotalPorts = theTotalPorts; }

inline void AllMsgStartProcessBase::SetPortSet(const FACT_SERV_PORT_RANGE_SET& thePortSet)
{ mPortSet = thePortSet; }

inline void AllMsgStartProcessBase::SetAuthorizedIPSet(const FACT_SERV_TRUSTED_ADDRESS_SET& theAuthorizedIPSet)
{ mAuthorizedIPSet = theAuthorizedIPSet; }

} // Namespace WONMsg
#endif