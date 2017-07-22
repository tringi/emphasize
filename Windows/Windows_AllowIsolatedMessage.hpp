#ifndef WINDOWS_ALLOWISOLATEDMESSAGE_HPP
#define WINDOWS_ALLOWISOLATEDMESSAGE_HPP

/* Windows AllowIsolatedMessage 
// Windows_AllowIsolatedMessage.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      11.04.2013 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // AllowIsolatedMessage
    //  - 
    //  - parameters: HWND - window for which to allow the message
    //                     - if NULL the message is allowed for whole process
    //                     - ignored (as if NULL) on Vista or earlier
    //                UINT - the window message
    //                bool - true to allow the message, false to disallow
    //  - returns: true - on successful change to the filter
    //             false - on error
    
    bool AllowIsolatedMessage (HWND, UINT, bool);
    
};

#endif

