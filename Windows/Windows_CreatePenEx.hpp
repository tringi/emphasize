#ifndef WINDOWS_CREATEPENEX_HPP
#define WINDOWS_CREATEPENEX_HPP

/* Emphasize Windows CreatePen GDI extension
// Windows_CreatePenEx.hpp
//
// Author: Jan Ringos, http://tringi.trimcore.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      11.12.2017 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreatePenEx
    //  - creates solid GDI pen object with optional alpha channel
    //  - parameters: style - PS_JOIN_BEVEL, PS_JOIN_MITER, PS_JOIN_ROUND (default)
    //                      - PS_ENDCAP_FLAT, PS_ENDCAP_SQUARE, PS_ENDCAP_ROUND (default)
    //                width - line width in pixels
    //                COLORREF - BGRA color value
    //                unsigned char - alpha, overrides 'color' alpha component
    //                              - alpha: 0x00 means fully transparent
    //                                       0xFF means opaque
    //  - returns: handle to new brush
    //             NULL on error (call GetLastError to get more details)
    
    HPEN CreatePenEx (DWORD style, DWORD width, COLORREF color);
    HPEN CreatePenEx (DWORD style, DWORD width, COLORREF color, unsigned char alpha);
};

#endif
