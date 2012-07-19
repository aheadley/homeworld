#if !defined(TMsgFactStopProcess_H)
#define TMsgFactStopProcess_H

// TMsgFactStopProcess.h

// Message that is used to stop a process via the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class TMsgFactStopProcess : public TMessage {

public:
	// Default ctor
	TMsgFactStopProcess(void);

	// TMessage ctor
	explicit TMsgFactStopProcess(const TMessage& theMsgR);

	// Copy ctor
	TMsgFactStopProcess(const TMsgFactStopProcess& theMsgR);

	// Destructor
	virtual ~TMsgFactStopProcess(void);

	// Assignment
	TMsgFactStopProcess& operator=(const TMsgFactStopProcess& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	const std::string& GetProcessName(void) const;
	unsigned short GetProcessPortID(void) const;

	virtual void SetProcessName(const std::string& theProcessName);
	virtual void SetProcessPortID(unsigned short theProcessPortID);

protected:
	std::string    mProcessName;
	unsigned short mProcessPortID;

};


// Inlines
inline TRawMsg* TMsgFactStopProcess::Duplicate(void) const
{ return new TMsgFactStopProcess(*this); }

inline const std::string& TMsgFactStopProcess::GetProcessName(void) const
{ return mProcessName; }

inline unsigned short TMsgFactStopProcess::GetProcessPortID(void) const
{ return mProcessPortID; }

inline void TMsgFactStopProcess::SetProcessName(const std::string& theProcessName)
{ mProcessName = theProcessName; }

inline void TMsgFactStopProcess::SetProcessPortID(unsigned short theProcessPortID)
{ mProcessPortID = theProcessPortID; }

};  // Namespace WONMsg

#endif