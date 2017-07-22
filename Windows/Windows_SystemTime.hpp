#ifndef WINDOWS_SYSTEMTIME_HPP
#define WINDOWS_SYSTEMTIME_HPP

/* Emphasize Windows SYSTEMTIME (and local time) functions
// Windows_SystemTime.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      30.12.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // SystemTime
    //  - returns SYSTEMTIME structure containing current time
    //  - or converts provided unix time to SYSTEMTIME structure
    
    SYSTEMTIME SystemTime ();
    SYSTEMTIME SystemTime (long long); // NOTE: defined in Windows_UnixTime.cpp

    // LocalTime
    //  - counterpart for SystemTime above

    SYSTEMTIME LocalTime ();
    
    // DiffTime
    //  - 
    
    bool DiffTime (const SYSTEMTIME & begin, const SYSTEMTIME & end, SYSTEMTIME *);
    
    // AddTime
    //  - adds length to begin and returns
    
    bool AddTime (const SYSTEMTIME & begin, const SYSTEMTIME & length, SYSTEMTIME *);
    
    
};

#endif

