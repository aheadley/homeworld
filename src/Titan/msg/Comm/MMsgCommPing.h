#ifndef _MMsgCommPing_H
#define _MMsgCommPing_H

// MMsgCommPing.h

// Common Ping message class.  Supports ping requests and replys to
// WON servers.  Ping reply may return extended info if requested.


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgCommPing : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommPing(bool getExtended=false);

	// MiniMessage ctor - will throw if MiniMessage type is not CommPing
	explicit MMsgCommPing(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommPing(const MMsgCommPing& theMsgR);

	// Destructor
	~MMsgCommPing(void);

	// Assignment
	MMsgCommPing& operator=(const MMsgCommPing& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	// Can only fetch the start tick, Pack generates startTick via GetTickCount.
	unsigned long GetStartTick() const;

	// Extended access
	bool GetExtended() const;
	void SetExtended(bool flag);

private:
	unsigned long mStartTick;    // Tick when ping was sent
	bool          mGetExtended;  // Fetch extended info?
};


class MMsgCommPingReply : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommPingReply(bool isExtended=false);

	// MiniMessage ctor - will throw if MiniMessage type is not CommPingReply
	explicit MMsgCommPingReply(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommPingReply(const MMsgCommPingReply& theMsgR);

	// Destructor
	~MMsgCommPingReply(void);

	// Assignment
	MMsgCommPingReply& operator=(const MMsgCommPingReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// StartTick access - this is the tick from the ping mesage
	unsigned long GetStartTick() const;
	void SetStartTick(unsigned long theTick);

	// Can only fetch the lag.  Calculated as GetTickCount() - mStartTick.
	// Automatically generated when Unpack is called
	unsigned long GetLag() const;

	// Extended access
	bool IsExtended() const;
	void SetExtended(bool flag);

	// ** The following attributes are only defined in extened was set on ping **

	// AppName access
	const std::string& GetAppName() const;
	void SetAppName(const std::string& theName);

	// Logical name access
	const std::string& GetLogicalName() const;
	void SetLogicalName(const std::string& theName);

	// Image access
	const std::string& GetImage() const;
	void SetImage(const std::string& theImage);

	// Version access
	unsigned long GetVersion() const;
	void SetVersion(unsigned long theVersion);

	// PID access
	int GetPID() const;
	void SetPID(int thePID);

	// Monitor ports access
	const std::string& GetPorts() const;
	std::string& GetPorts();
	void SetPorts(const std::string& thePorts);

	// IsAuth Access
	bool IsAuth() const;
	void SetIsAuth(bool flag);

	// Dir reg info access
	const std::string& GetRegData() const;
	std::string& GetRegData();
	void SetRegData(const std::string& theData);

	// Server data access
	const std::string& GetServData() const;
	std::string& GetServData();
	void SetServData(const std::string& theData);

private:
	unsigned long mStartTick;    // Tick count when ping was started
	unsigned long mLag;          // Lag buffer generated on unpack
	bool          mExtended;     // Extended info present
	std::string   mAppName;      // Application name
	std::string   mLogicalName;  // Logical name (may be null)
	std::string   mImage;        // Image (path to exe) (may be null)
	unsigned long mVersion;      // Server version as time_t
	int           mPID;          // PID (0 implies undefined)
	std::string   mPorts;        // Ports being monitored (may be null)
	bool          mIsAuth;       // Is authentucated (always false if extended is false)
	std::string   mRegData;      // Dir registration data
	std::string   mServData;     // Server specific data
};


// Inlines
inline TRawMsg*
MMsgCommPing::Duplicate(void) const
{ return new MMsgCommPing(*this); }

inline unsigned long
MMsgCommPing::GetStartTick() const
{ return mStartTick; }

inline bool
MMsgCommPing::GetExtended() const
{ return mGetExtended; }

inline void
MMsgCommPing::SetExtended(bool flag)
{ mGetExtended = flag; }

inline TRawMsg*
MMsgCommPingReply::Duplicate(void) const
{ return new MMsgCommPingReply(*this); }

inline unsigned long
MMsgCommPingReply::GetStartTick() const
{ return mStartTick; }

inline void
MMsgCommPingReply::SetStartTick(unsigned long theStartTick)
{ mStartTick = theStartTick; }

inline unsigned long
MMsgCommPingReply::GetLag() const
{ return mLag; }

inline bool
MMsgCommPingReply::IsExtended() const
{ return mExtended; }

inline void
MMsgCommPingReply::SetExtended(bool flag)
{ mExtended = flag; }

inline const std::string&
MMsgCommPingReply::GetAppName() const
{ return mAppName; }

inline void
MMsgCommPingReply::SetAppName(const std::string& theName)
{ mAppName = theName; }

inline const std::string&
MMsgCommPingReply::GetLogicalName() const
{ return mLogicalName; }

inline void
MMsgCommPingReply::SetLogicalName(const std::string& theName)
{ mLogicalName = theName; }

inline const std::string&
MMsgCommPingReply::GetImage() const
{ return mImage; }

inline void
MMsgCommPingReply::SetImage(const std::string& theImage)
{ mImage = theImage; }

inline unsigned long
MMsgCommPingReply::GetVersion() const
{ return mVersion; }

inline void
MMsgCommPingReply::SetVersion(unsigned long theVersion)
{ mVersion = theVersion; }

inline int
MMsgCommPingReply::GetPID() const
{ return mPID; }

inline void
MMsgCommPingReply::SetPID(int thePID)
{ mPID = thePID; }

inline const std::string&
MMsgCommPingReply::GetPorts() const
{ return mPorts; }

inline std::string&
MMsgCommPingReply::GetPorts()
{ return mPorts; }

inline void
MMsgCommPingReply::SetPorts(const std::string& thePorts)
{ mPorts = thePorts; }

inline bool
MMsgCommPingReply::IsAuth() const
{ return mIsAuth; }

inline void
MMsgCommPingReply::SetIsAuth(bool flag)
{ mIsAuth = flag; }

inline const std::string&
MMsgCommPingReply::GetRegData() const
{ return mRegData; }

inline std::string&
MMsgCommPingReply::GetRegData()
{ return mRegData; }

inline void
MMsgCommPingReply::SetRegData(const std::string& theData)
{ mRegData = theData; }

inline const std::string&
MMsgCommPingReply::GetServData() const
{ return mServData; }

inline std::string&
MMsgCommPingReply::GetServData()
{ return mServData; }

inline void
MMsgCommPingReply::SetServData(const std::string& theData)
{ mServData = theData; }

};  // Namespace WONMsg

#endif