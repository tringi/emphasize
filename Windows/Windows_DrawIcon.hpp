#ifndef WINDOWS_DRAWICON_HPP
#define WINDOWS_DRAWICON_HPP

/* Windows DrawIcon 
// Windows_DrawIcon.hpp
//
// Author: Jan Ringos, http://Tringi.TrimCore.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      02.04.2015 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetIconSize
    //  - 
    
    bool GetIconSize (HICON, SIZE *, UINT * bpp = NULL);
    
    // DrawIcon
    //  - 
    
    bool DrawIcon (HDC, HICON, int x, int y, UCHAR alpha = 255);
    bool DrawIcon (HDC, HICON, const RECT &, UCHAR alpha = 255);
    
};

#endif

