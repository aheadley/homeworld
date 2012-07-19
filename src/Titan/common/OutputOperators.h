#ifndef OUTPUTOPERATORS_H
#define OUTPUTOPERATORS_H

#include "won.h"
#include "LIST"
#include <ctype.h>

// RawBuffer
// Outputs "[val val ... (ascii)]"
inline ostream&
operator<<(ostream& os, const WONCommon::RawBuffer& theBuffer)
{
    string aPrint;
    os << '[' << std::hex;

    for (int i=0; i < theBuffer.size(); i++)
    {
        if (i > 0) os << ' ';
        os << static_cast<unsigned short>(theBuffer[i]);
        if (isprint(theBuffer[i])) aPrint += static_cast<char>(theBuffer[i]);
    }

    if (! aPrint.empty()) os << " (" << aPrint << ')';
    os << std::dec << ']';
    return os;
}


// std::list<unsigned short>
// Outputs "[elt1,elt2,...,eltN]"
inline ostream&
operator<<(ostream& os, const std::list<unsigned short>& theList)
{
    os << "[";
    bool bFirstElement = true;
    std::list<unsigned short>::const_iterator itr = theList.begin();
    for (; itr != theList.end(); itr++)
    {
        if (!bFirstElement)
            os << ",";
        os << *itr;
        bFirstElement = false;
    }
    os << "]";
    return os;
}

#endif // OUTPUTOPERATORS_H