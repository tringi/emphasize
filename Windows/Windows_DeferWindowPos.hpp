#ifndef WINDOWS_DEFERWINDOWPOS_HPP
#define WINDOWS_DEFERWINDOWPOS_HPP

/* Emphasize Windows simpler DeferWindowPos routine
// Windows_DeferWindowPos.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: DeferWindowPos that is more conscise and defaulted to WM_SIZE
//              behavior.
//
// Changelog:
//      05.07.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // DeferWindowPos
    //  - simplifies call to system DeferWindowPos and fallbacks to SetWindowPos
    //    if DeferWindowPos fails
    //     - although does not "unwind" defering
    //  - parameters: HDWP - defering handle (see BeginDeferWindowPos)
    //                HWND - window handle
    //                (UINT - child ID; when used, HWND is parent window)
    //                RECT - requested position and width and height
    //                     - optionally 4 long parameters 
    //                flags - optional SWP_xxx flags
    //  - returns false if DeferWindowPos failed and attempt to do the same
    //                  with SetWindowPos also fails
    
    bool DeferWindowPos (HDWP &, HWND, const RECT &,
                         UINT flags = SWP_NOZORDER | SWP_NOACTIVATE,
                         HWND hAfter = NULL);

    bool DeferWindowPos (HDWP &, HWND, UINT, const RECT &,
                         UINT flags = SWP_NOZORDER | SWP_NOACTIVATE,
                         HWND hAfter = NULL);
    
    bool DeferWindowPos (HDWP &, HWND, long, long, long, long);
    bool DeferWindowPos (HDWP &, HWND, UINT, long, long, long, long);


    // SetDeferWindowPosAaaa
    
};

#endif
