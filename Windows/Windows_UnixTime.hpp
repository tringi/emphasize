#ifndef WINDOWS_UNIXTIME_HPP
#define WINDOWS_UNIXTIME_HPP

/* Emphasize Windows SYSTEMTIME <-> unit time conversion
// Windows_UnixTime.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      04.09.2011 - initial version
//      30.12.2012 - moved SystemTime() to Windows_SystemTime.cpp
*/

#include <windows.h>

namespace Windows {
    
    // UnitTime
    //  - returns 64b unadjusted unix time for given SYSTEMTIME (or current)
    
    long long UnixTime ();
    long long UnixTime (const SYSTEMTIME &);
    
    // SystemTime
    //  - converts provided (or current) unix time to SYSTEMTIME structure
    
    SYSTEMTIME SystemTime (); // defined in Windows_SystemTime.cpp
    SYSTEMTIME SystemTime (long long);
};

#endif
