#ifndef WINDOWS_SETPIXELEX_HPP
#define WINDOWS_SETPIXELEX_HPP

/* Emphasize Windows SetPixelEx GDI extension
// Windows_SetPixelEx.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      15.10.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // SetPixelEx
    //  - paints the pixel with alpha channel
    //  - parameters: COLORREF - BGRA color value
    //                unsigned char - alpha, overrides 'color' alpha component
    //                              - alpha: 0x00 means fully transparent
    //                                       0xFF means opaque
    //  - returns: true on success
    //             false on error (call GetLastError to get more details)
    
    bool SetPixelEx (HDC, int, int, HBRUSH);
    bool SetPixelEx (HDC, int, int, COLORREF color);
    bool SetPixelEx (HDC, int, int, COLORREF color, unsigned char alpha);
};

#endif
