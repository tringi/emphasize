#ifndef WINDOWS_FADETOBLACK_HPP
#define WINDOWS_FADETOBLACK_HPP

/* Windows API Fading screen rectangle
// Windows_FadeToBlack.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      01.11.2011 - initial version
//      25.01.2013 - added FadeToColor
*/

#include <windows.h>

namespace Windows {

    // FadeToBlack
    //  - creates black fading in rectangle
    //     - calls FadeToColor with COLORREF == 0
    //  - the function returns when the rectangle gets fully opaque
    //  - parameters: RECT - in screen coordinates
    
    HWND FadeToBlack (HINSTANCE, const RECT &, BOOL always_on_top = FALSE);
    
    // FadeToColor
    //  - creates fading in rectangle of provided color
    //  - the function returns when the rectangle gets fully opaque
    //  - parameters: RECT - in screen coordinates
    
    HWND FadeToColor (HINSTANCE, const RECT &, COLORREF, BOOL always_on_top = FALSE);
    
};

#endif
