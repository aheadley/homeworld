#ifndef _TMsgCommPing_H
#define _TMsgCommPing_H

// TMsgCommPing.h

// Common Ping message classes.  Supports ping requests and replys to
// WON servers.


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommPing : public TMessage
{
public:
	// Default ctor
	TMsgCommPing(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommPing(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommPing(const TMsgCommPing& theMsgR);

	// Destructor
	~TMsgCommPing(void);

	// Assignment
	TMsgCommPing& operator=(const TMsgCommPing& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	// Can only fetch the start tick, Pack generates startTick via GetTickCount.
	unsigned long GetStartTick() const;

private:
	unsigned long mStartTick;
};


class TMsgCommPingReply : public TMessage
{
public:
	// Default ctor
	TMsgCommPingReply(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommPingReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommPingReply(const TMsgCommPingReply& theMsgR);

	// Destructor
	~TMsgCommPingReply(void);

	// Assignment
	TMsgCommPingReply& operator=(const TMsgCommPingReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	const std::string& GetAppName() const;
	const std::string& GetLogicalName() const;
	const std::string& GetImage() const;
	unsigned long      GetVersion() const;
	int                GetPID() const;
	const std::string& GetPorts() const;
	unsigned long      GetStartTick() const;

	// Allow piece-wiese build of ports
	std::string& GetPorts();

	void SetAppName(const std::string& theName);
	void SetLogicalName(const std::string& theName);
	void SetImage(const std::string& theImage);
	void SetVersion(unsigned long theVersion);
	void SetPID(int thePID);
	void SetPorts(const std::string& thePorts);
	void SetStartTick(unsigned long theTick);

	// Can only fetch the lag.  Calculated as GetTickCount() - mStartTick.
	// Automatically generated when Unpack is called
	unsigned long GetLag() const;

private:
	std::string   mAppName;      // Application name
	std::string   mLogicalName;  // Logical name (may be null)
	std::string   mImage;        // Image (path to exe) (may be null)
	unsigned long mVersion;      // Server version as time_t
	int           mPID;          // PID (0 implies undefined)
	std::string   mPorts;        // Ports being monitored (may be null)
	unsigned long mStartTick;    // Tick count when ping was started
	unsigned long mLag;          // Lag buffer generated on unpack
};


// Inlines
inline TRawMsg*
TMsgCommPing::Duplicate(void) const
{ return new TMsgCommPing(*this); }

inline unsigned long
TMsgCommPing::GetStartTick() const
{ return mStartTick; }

inline TRawMsg*
TMsgCommPingReply::Duplicate(void) const
{ return new TMsgCommPingReply(*this); }

inline const std::string&
TMsgCommPingReply::GetAppName() const
{ return mAppName; }

inline const std::string&
TMsgCommPingReply::GetLogicalName() const
{ return mLogicalName; }

inline const std::string&
TMsgCommPingReply::GetImage() const
{ return mImage; }

inline unsigned long
TMsgCommPingReply::GetVersion() const
{ return mVersion; }

inline int
TMsgCommPingReply::GetPID() const
{ return mPID; }

inline const std::string&
TMsgCommPingReply::GetPorts() const
{ return mPorts; }

inline unsigned long
TMsgCommPingReply::GetStartTick() const
{ return mStartTick; }

inline unsigned long
TMsgCommPingReply::GetLag() const
{ return mLag; }

inline std::string&
TMsgCommPingReply::GetPorts()
{ return mPorts; }

inline void
TMsgCommPingReply::SetAppName(const std::string& theName)
{ mAppName = theName; }

inline void
TMsgCommPingReply::SetLogicalName(const std::string& theName)
{ mLogicalName = theName; }

inline void
TMsgCommPingReply::SetVersion(unsigned long theVersion)
{ mVersion = theVersion; }

inline void
TMsgCommPingReply::SetImage(const std::string& theImage)
{ mImage = theImage; }

inline void
TMsgCommPingReply::SetPID(int thePID)
{ mPID = thePID; }

inline void
TMsgCommPingReply::SetPorts(const std::string& thePorts)
{ mPorts = thePorts; }

inline void
TMsgCommPingReply::SetStartTick(unsigned long theStartTick)
{ mStartTick = theStartTick; }

};  // Namespace WONMsg

#endif