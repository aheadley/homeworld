#ifndef _REGKEY_H
#define _REGKEY_H

// RegKey

// Encapsulates access to a Registry Key.  Key must already exist.  Allows
// string and long values to created, read, updated, or deleted.


#include "STRING"
#include <windows.h>


namespace REG_CONST {

    const std::string REG_BACKSLASH("\\");
    const std::string REG_SOFTWARE_LEAF_NAME("SOFTWARE");
    const std::string REG_WON_LEAF_NAME("WON");
    const std::string REG_TITAN_LEAF_NAME("Titan");
    const std::string REG_WON_KEY_NAME(REG_SOFTWARE_LEAF_NAME+REG_BACKSLASH+REG_WON_LEAF_NAME);
    const std::string REG_TITAN_KEY_NAME(REG_WON_KEY_NAME+REG_BACKSLASH+REG_TITAN_LEAF_NAME);
    const std::string REG_AUTHVERIFIER_NAME("AuthVerifierKey");
};

// RegKey in WON namespace
namespace WONCommon
{

class RegKey
{
public:
    // Types
    enum GetResult { Ok, NotFound, BadType, NoMore, NoSize };
    enum DataType { Long, String, Binary, Other };

    // Constructors/Destructor
    explicit RegKey(const std::string& theKey, HKEY theRoot=HKEY_LOCAL_MACHINE, bool createKey = false, bool nonVolatile = true);
    RegKey(void);
    virtual ~RegKey(void);

    // Is Key open
    virtual bool IsOpen(void) const;

    // Open a new key (closes current key)
    virtual bool OpenNewKey(const std::string& theKey, HKEY theRoot=HKEY_LOCAL_MACHINE, bool createKey = false, bool nonVolatile = true);

    // Open a child key from the parent key, close the parent key (transforms key)
    //   Parent not closed if opening subkey fails
    virtual bool OpenSubKey(const std::string& theSubKey, bool createSubKey = false, bool nonVolatile = true);

    // Get, and open, a child key from the parent
    virtual bool GetSubKey(const std::string& theSubKeyName, RegKey& theSubKey, bool createSubKey = false, bool nonVolatile = true) const;

    // Enumerate through subkeys,  GetFirst primes the function and fills in the first subkey, if any
    virtual GetResult GetFirstSubKey(RegKey& theSubKey) const;
    virtual GetResult GetNextSubKey(RegKey& theSubKey) const;

    // Enumerate through value names,  GetFirst primes the function and fills in the first value name, if any
    virtual GetResult GetFirstValueName(std::string& theName, DataType& theType) const;
    virtual GetResult GetNextValueName(std::string& theName, DataType& theType) const;

    // Fetch values
    GetResult GetValue(const std::string& theName, std::string& theValR) const;
    GetResult GetValue(const std::string& theName, unsigned long& theValR) const;
    // Assigning value to theValPR, so make sure it isn't the only pointer to some allocated memory
    // TheValPR is dynamically allocated, the function caller OWNS this pointer (you clean it up when done)
    GetResult GetValue(const std::string& theName, unsigned char* &theValPR, unsigned long& theLengthR) const;

    // Set/Create values
    bool SetValue(const std::string& theName, const std::string& theValue);
    bool SetValue(const std::string& theName, unsigned long theValue);
    bool SetValue(const std::string& theName, const unsigned char* theValueP, unsigned long theLength);

    // Delete value
    bool DeleteValue(const std::string& theName);

    // Delete a subkey off this parent
    //   Note:  If subkey has subkeys, deletion will fail
    virtual bool DeleteSubKey(const std::string& theSubKey) const;
    virtual bool DeleteSubKey(RegKey& theSubKey) const;

    // Get name of leaf off of root
    const std::string& GetLeafName(void) const;

    // Close the current key if open
    void CloseKey(void);

protected:
    HKEY mKey;   // Open registry key
    bool mOpen;  // Is the key open?

    std::string mLeafName;  // Name of key off of root

    mutable unsigned char* mBufP;       // Buffer used to get values
    mutable unsigned long  mBufLen;     // Length of buffer
    mutable DWORD          mKeyIndex;   // Index for SubKey enumeration
    mutable DWORD          mValueIndex; // Index for Value enumeration

private:
    // Private Methods
    DWORD GetToBuf(const std::string& theName, unsigned long &theLengthR) const;

    // Disallow these methods
    RegKey(const RegKey& theKeyR);
    RegKey& operator=(const RegKey& theKeyR);


};


// Inlines
inline bool RegKey::IsOpen(void) const{ return mOpen; }
inline RegKey::RegKey(void) : mLeafName(), mBufLen(0), mBufP(NULL), mOpen(false), mKey(NULL), mKeyIndex(0), mValueIndex(0) {}
inline const std::string& RegKey::GetLeafName(void) const { return mLeafName; }

};  //namespace WON


#endif // _REGKEY_H