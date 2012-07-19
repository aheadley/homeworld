#ifndef _T_MSG_PROTOCOL_H
#define _T_MSG_PROTOCOL_H

// TMsgCommProtocol.h

// Common install protocol request.

#include <tmessage.h>
#include <msg/TServiceTypes.h>
#include <msg/TMsgTypesComm.h>

#include <msg/TMsgProtocolTypes.h>

namespace WONMsg {

//-------------------------------------------------------------------
//
class TMsgCommProtocol : public TMessage
{
public:
    // constructors
	TMsgCommProtocol();      // default ctor
    explicit TMsgCommProtocol( const TMessage& theMsgR ); // TMessage ctor
    TMsgCommProtocol( const TMsgCommProtocol & theMessage );  // copy ctor
   ~TMsgCommProtocol(); // dtor
	TMsgCommProtocol& operator=(const TMsgCommProtocol & theMsgR);

    TRawMsg * Duplicate(void) const;

	// virtual functions Pack and Unpack
	// Unpack will throw a BadMsgException if message is not of this type
    void * Pack(void);
    void Unpack(void);

    MsgProtocolType  GetProtocolType()  const;
    const TMessage & GetNestedMessage() const;

	void SetProtocolType( MsgProtocolType theProtType );
	void SetNestedMessage( TMessage & theNestedMessage ); // packs theNestedMessage

private:
    MsgProtocolType mProtocolType;
    TMessage        mNestedMessage; // packed in Set function.
};

class TMsgCommProtocolReject : public TMessage
{
public:
    // constructors
	TMsgCommProtocolReject();      // default ctor
    explicit TMsgCommProtocolReject( const TMessage& theMsgR ); // TMessage ctor
    TMsgCommProtocolReject( const TMsgCommProtocolReject & theMessage );  // copy ctor
   ~TMsgCommProtocolReject(); // dtor
	TMsgCommProtocolReject& operator=(const TMsgCommProtocolReject & theMsgR);

    TRawMsg * Duplicate(void) const;

	// virtual functions Pack and Unpack
	// Unpack will throw a BadMsgException if message is not of this type
    void * Pack(void);
    void Unpack(void);

private:
    MsgProtocolType mProtocolType;

};

// inlines
inline TRawMsg * 
TMsgCommProtocolReject::Duplicate(void) const
{ return( new TMsgCommProtocolReject(*this) ); }

inline TRawMsg * 
TMsgCommProtocol::Duplicate(void) const
{ return( new TMsgCommProtocol(*this) ); }

inline MsgProtocolType
TMsgCommProtocol::GetProtocolType()  const 
{ return( mProtocolType  ); }

inline const TMessage &
TMsgCommProtocol::GetNestedMessage() const 
{ return( mNestedMessage ); }

inline void
TMsgCommProtocol::SetProtocolType( MsgProtocolType theProtocolType )
{ mProtocolType = theProtocolType; }

inline void
TMsgCommProtocol::SetNestedMessage( TMessage & theNestedMessage )
{
    //
    // Gotta pack theFirstMessage here because the assignment
    // to mFirstMessage drops all derived attributes.
    //
    // Alternatives would be to use a ref - or an allocated
    // theFirstMessage.Duplicate() pointer.  The problem with using a
    // ref is that the refed object could go out of scope and be destroyed
    // before this object instance is destroyed.  The problem with using
    // a duplicate() copy is that it results in unnecessary memory
    // allocation, and unneccessary copying of data members.
    // 

    theNestedMessage.Pack();
    mNestedMessage = theNestedMessage;
}


}; // namespace WONMsg

#endif
