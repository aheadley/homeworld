#if !defined(TMsgFactProcessConfigurationReply_H)
#define TMsgFactProcessConfigurationReply_H

// TMsgFactProcessConfigurationReply.h

// Message that is used to return a process configuration from the Factory Server


#include "msg/TMessage.h"
#include "MAP"


namespace WONMsg {

    struct FACT_SERV_FIELD_REPLY {

        FACT_SERV_FIELD_REPLY(void);
        FACT_SERV_FIELD_REPLY(const FACT_SERV_FIELD_REPLY& theCopyField);
        FACT_SERV_FIELD_REPLY& operator =(const FACT_SERV_FIELD_REPLY& theCopyField);

        unsigned long mFieldStatus;
        std::string   mFieldData;

    };

    typedef std::map<unsigned char, FACT_SERV_FIELD_REPLY> FACT_SERV_FIELD_REPLY_MAP;


class TMsgFactProcessConfigurationReply : public TMessage {

public:
    // Default ctor
    TMsgFactProcessConfigurationReply(void);

    // TMessage ctor
    explicit TMsgFactProcessConfigurationReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactProcessConfigurationReply(const TMsgFactProcessConfigurationReply& theMsgR);

    // Destructor
    virtual ~TMsgFactProcessConfigurationReply(void);

    // Assignment
    TMsgFactProcessConfigurationReply& operator=(const TMsgFactProcessConfigurationReply& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    short GetStatus(void) const;
    const FACT_SERV_FIELD_REPLY_MAP& GetConfigFieldMap(void) const;

    virtual void SetStatus(short theStatus);
    virtual void SetConfigFieldMap(const FACT_SERV_FIELD_REPLY_MAP& theConfigFieldMap);

protected:
    short                     mStatus;
    FACT_SERV_FIELD_REPLY_MAP mConfigFieldMap;

};


// Inlines
inline TRawMsg* TMsgFactProcessConfigurationReply::Duplicate(void) const
{ return new TMsgFactProcessConfigurationReply(*this); }

inline short TMsgFactProcessConfigurationReply::GetStatus(void) const
{ return mStatus; }

inline const FACT_SERV_FIELD_REPLY_MAP& TMsgFactProcessConfigurationReply::GetConfigFieldMap(void) const
{ return mConfigFieldMap; }

inline void TMsgFactProcessConfigurationReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

inline void TMsgFactProcessConfigurationReply::SetConfigFieldMap(const FACT_SERV_FIELD_REPLY_MAP& theConfigFieldMap)
{ mConfigFieldMap = theConfigFieldMap; }

};  // Namespace WONMsg

#endif