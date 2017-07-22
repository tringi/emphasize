#ifndef WINDOWS_CREATETITLEFONT_HPP
#define WINDOWS_CREATETITLEFONT_HPP

/* Windows CreateTitleFont 
// Windows_CreateTitleFont.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      07.06.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreateTitleFont
    //  - creates nice title font
    //  - parameters: HWND - themed window handle
    //                COLORREF - if not null, title font color is returned
    //  - return value: HFONT as created (or not) by CreateFontIndirect
    
    HFONT CreateTitleFont (HWND, COLORREF * = NULL);
    
};

#endif

