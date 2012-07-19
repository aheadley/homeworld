#if !defined(TMsgFactGetProcessConfiguration_H)
#define TMsgFactGetProcessConfiguration_H

// TMsgFactGetProcessConfiguration.h

// Message that is used to get a process configuration from the Factory Server


#include "msg/TMessage.h"
#include "SET"


namespace WONMsg {

    typedef std::set<unsigned char> FACT_SERV_FIELD_SET;

class TMsgFactGetProcessConfiguration : public TMessage {

public:
    // Default ctor
    TMsgFactGetProcessConfiguration(void);

    // TMessage ctor
    explicit TMsgFactGetProcessConfiguration(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactGetProcessConfiguration(const TMsgFactGetProcessConfiguration& theMsgR);

    // Destructor
    virtual ~TMsgFactGetProcessConfiguration(void);

    // Assignment
    TMsgFactGetProcessConfiguration& operator=(const TMsgFactGetProcessConfiguration& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const std::string& GetConfigName(void) const;
    const FACT_SERV_FIELD_SET& GetFieldSet(void) const;

    virtual void SetConfigName(const std::string& theConfigName);
    virtual void SetFieldSet(const FACT_SERV_FIELD_SET& theFieldSet);

protected:
    std::string         mConfigName;
    FACT_SERV_FIELD_SET mFieldSet;

};


// Inlines
inline TRawMsg* TMsgFactGetProcessConfiguration::Duplicate(void) const
{ return new TMsgFactGetProcessConfiguration(*this); }

inline const std::string& TMsgFactGetProcessConfiguration::GetConfigName(void) const
{ return mConfigName; }

inline const FACT_SERV_FIELD_SET& TMsgFactGetProcessConfiguration::GetFieldSet(void) const
{ return mFieldSet; }

inline void TMsgFactGetProcessConfiguration::SetConfigName(const std::string& theConfigName)
{ mConfigName = theConfigName; }

inline void TMsgFactGetProcessConfiguration::SetFieldSet(const FACT_SERV_FIELD_SET& theFieldSet)
{ mFieldSet = theFieldSet; }

};  // Namespace WONMsg

#endif