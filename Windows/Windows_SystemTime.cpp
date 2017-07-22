#include "Windows_SystemTime.hpp"

/* Emphasize Windows SYSTEMTIME (and local time) functions
// Windows_SystemTime.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      30.12.2012 - initial version
*/

SYSTEMTIME Windows::SystemTime () {
    SYSTEMTIME st;
    GetSystemTime (&st);
    return st;
};

SYSTEMTIME Windows::LocalTime () {
    SYSTEMTIME st;
    GetLocalTime (&st);
    return st;
};

bool Windows::DiffTime (const SYSTEMTIME & begin,
                        const SYSTEMTIME & end,
                        SYSTEMTIME * result) {
    union {
        FILETIME            ft;
        unsigned long long  ull;
    } b,
      e,
      r;
    
    if (SystemTimeToFileTime (&begin, &b.ft))
        if (SystemTimeToFileTime (&end, &e.ft)) {
            
            r.ull = e.ull - b.ull;
            return FileTimeToSystemTime (&r.ft, result);
        };
    
    return false;
};

bool Windows::AddTime (const SYSTEMTIME & begin,
                       const SYSTEMTIME & length,
                       SYSTEMTIME * result) {
    union {
        FILETIME            ft;
        unsigned long long  ull;
    } b,
      l,
      r;
    
    if (SystemTimeToFileTime (&begin, &b.ft))
        if (SystemTimeToFileTime (&length, &l.ft)) {
            
            r.ull = b.ull + l.ull;
            return FileTimeToSystemTime (&r.ft, result);
        };
    
    return false;
};


