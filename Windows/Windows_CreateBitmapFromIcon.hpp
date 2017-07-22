#ifndef WINDOWS_CREATEBITMAPFROMICON_HPP
#define WINDOWS_CREATEBITMAPFROMICON_HPP

/* Windows CreateBitmapFromIcon 
// Windows_CreateBitmapFromIcon.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      31.10.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreateBitmapFromIcon
    //  - creates premultiplied-ARGB bitmap of the icon
    //  - HICON - icon to convert
    //    HBRUSH - used to fill the background
    //           - defaults to BLACK_BRUSH if on NT6+ otherwise to WHITE_BRUSH
    //             (because black brush is completely transparent, and white
    //             brush can safely be used as menu item background or NT5.x)
    //  - returns: HBITMAP representing the icon
    //             NULL on error (call GetLastError ())
    
    HBITMAP CreateBitmapFromIcon (HICON, HBRUSH = NULL);
    
};

#endif

