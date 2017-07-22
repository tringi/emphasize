#ifndef WINDOWS_CREATESOLIDBRUSHEX_HPP
#define WINDOWS_CREATESOLIDBRUSHEX_HPP

/* Emphasize Windows CreateSolidBrushEx GDI extension
// Windows_CreateSolidBrushEx.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      14.10.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreateSolidBrushEx
    //  - creates solid GDI brush object with optional alpha channel
    //  - parameters: COLORREF - BGRA color value
    //                unsigned char - alpha, overrides 'color' alpha component
    //                              - alpha: 0x00 means fully transparent
    //                                       0xFF means opaque
    //  - returns: handle to new brush
    //             NULL on error (call GetLastError to get more details)
    
    HBRUSH CreateSolidBrushEx (COLORREF color);
    HBRUSH CreateSolidBrushEx (COLORREF color, unsigned char alpha);
};

#endif
