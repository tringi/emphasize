#ifndef WINDOWS_GETDESKTOPDPI_HPP
#define WINDOWS_GETDESKTOPDPI_HPP

/* Windows GetDesktopDPI 
// Windows_GetDesktopDPI.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      10.12.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetDesktopDPI/GetDesktopScale
    //  - returns desktop DPI or scale against default DPI of 96
    //  - parameters: x - horizontal dpi/scale, can be NULL if not required
    //                y - vertical dpi/scale, can be NULL if not required
    //  - return value: true - 
    //                  false - on failure, still initializes x and y to defaults
    
    bool GetDesktopDPI (unsigned int * x, unsigned int * y);
    bool GetDesktopScale (float * x, float * y);
    
};

#endif

