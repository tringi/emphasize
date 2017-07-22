#ifndef WINDOWS_GRADIENTFILL_HPP
#define WINDOWS_GRADIENTFILL_HPP

/* Emphasize Windows GradientFill GDI wrapper
// Windows_GradientFill.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      21.11.2011 - initial version
*/

#include <windows.h>

namespace Windows {

    // GradientFill
    //  - 
    //  - parameters: HDC - handle to device context
    //                RECT - rectangle to fill
    //                UINT - fill mode: use HS_HORIZONTAL or HS_VERTICAL
    //                COLORREF - top-left and bottom-right colors

    bool GradientFill (HDC, const RECT &, UINT, COLORREF, COLORREF);
};

#endif
