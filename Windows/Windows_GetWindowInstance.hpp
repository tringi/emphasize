#ifndef WINDOWS_GETWINDOWINSTANCE_HPP
#define WINDOWS_GETWINDOWINSTANCE_HPP

/* Windows GetWindowInstance 
// Windows_GetWindowInstance.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      19.11.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetWindowInstance
    //  - returns module instance for the window
    
    HINSTANCE GetWindowInstance (HWND);
    
};

#endif

