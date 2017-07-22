#include "Windows_UnixTime.hpp"

/* Emphasize Windows SYSTEMTIME <-> unit time conversion
// Windows_UnixTime.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      04.09.2011 - initial version
//      30.12.2012 - moved SystemTime() to Windows_SystemTime.cpp
*/

namespace {
    
    // base
    //  - equivalent to 1.1.1970 00:00:00 - January 1, 1601 UTC in
    //    resolution of 100-nanosecond intervals
    
    static const long long base = 0x019db1ded53e8000LL; 
};

long long Windows::UnixTime () {
    return Windows::UnixTime (Windows::SystemTime ());
};

long long Windows::UnixTime (const SYSTEMTIME & st) {
    union {
        long long  ns;
        FILETIME   ft; // 100-nanosecond intervals since January 1, 1601
    };

    if (SystemTimeToFileTime (&st, &ft))
        return (ns - base) / 10000000LL;
    else
        return 0u;
};

SYSTEMTIME Windows::SystemTime (long long unix) {
    SYSTEMTIME st;

    union {
        long long  ns;
        FILETIME   ft; // 100-nanosecond intervals since January 1, 1601
    };
    
    ns = unix * 10000000LL + base;
    if (!FileTimeToSystemTime (&ft, &st))
        ZeroMemory (&st, sizeof st);
    
    return st;
};

