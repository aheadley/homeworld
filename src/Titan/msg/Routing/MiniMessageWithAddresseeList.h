#if !defined(RoutingServerMessageWithAddresseeList_H)
#define RoutingServerMessageWithAddresseeList_H

// MiniMessageWithAddresseeList.h

#include "LIST"
#include "RoutingServerMessage.h"

namespace WONMsg {

class MiniMessageWithAddresseeList : public RoutingServerMessage {
public:
    // Default ctor
    MiniMessageWithAddresseeList(void);

    // RoutingServerMessage ctor
    explicit MiniMessageWithAddresseeList(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MiniMessageWithAddresseeList(const MiniMessageWithAddresseeList& theMsgR);

    // Destructor
    virtual ~MiniMessageWithAddresseeList(void);

    // Assignment
    MiniMessageWithAddresseeList& operator=(const MiniMessageWithAddresseeList& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Debug output
    virtual void Dump(std::ostream& os) const;

    void AppendAddresseeList();
    void ReadAddresseeList();

    typedef std::list<ClientOrGroupId> AddresseeList;
    // Member access
    bool GetIncludeExcludeFlag() const                            { return mIncludeExcludeFlag; }
    const AddresseeList& GetAddresseeList() const                 { return mAddresseeList; }

    void SetIncludeExcludeFlag(bool theIncludeExcludeFlag)        { mIncludeExcludeFlag = theIncludeExcludeFlag; }
    void SetAddresseeList(const AddresseeList& theAddresseeListR) { mAddresseeList = theAddresseeListR; }
    void AddAddressee(ClientOrGroupId theAddressee)               { mAddresseeList.push_back(theAddressee); }
protected:
    bool          mIncludeExcludeFlag;
    AddresseeList mAddresseeList;
};


// Inlines
inline TRawMsg* MiniMessageWithAddresseeList::Duplicate(void) const
    { return new MiniMessageWithAddresseeList(*this); }

};  // Namespace WONMsg

#endif // RoutingServerMessageWithAddresseeList_H