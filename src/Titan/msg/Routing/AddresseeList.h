#ifndef ADDRESSEELIST_H
#define ADDRESSEELIST_H

// MiniMessageWithAddresseeList.h

#include "LIST"
#include "RoutingServerMessage.h"

namespace WONMsg {

class AddresseeList {
public:
    // Default ctor
    AddresseeList(void);

    // Copy ctor
    AddresseeList(const AddresseeList& theCopyR);

    // Destructor
    virtual ~AddresseeList(void);

    // Assignment
    AddresseeList& operator=(const AddresseeList& theCopyR);

    // Debug output
    void Dump(std::ostream& os) const;

    void AppendAddresseeList(RoutingServerMessage* theMsgP);
    void ReadAddresseeList(RoutingServerMessage* theMsgP);

    typedef std::list<ClientOrGroupId> IdList;
    // Member access
    bool GetIncludeExcludeFlag() const                     { return mIncludeExcludeFlag; }
    const IdList& GetAddresseeList() const                 { return mAddresseeList; }

    void SetIncludeExcludeFlag(bool theIncludeExcludeFlag) { mIncludeExcludeFlag = theIncludeExcludeFlag; }
    void SetAddresseeList(const IdList& theAddresseeListR) { mAddresseeList = theAddresseeListR; }
    void AddAddressee(ClientOrGroupId theAddressee)        { mAddresseeList.push_back(theAddressee); }
protected:
    bool   mIncludeExcludeFlag;
    IdList mAddresseeList;
};

};  // Namespace WONMsg

#endif // ADDRESSEELIST_H