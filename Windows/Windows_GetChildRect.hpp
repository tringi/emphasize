#ifndef WINDOWS_GETCHILDRECT_HPP
#define WINDOWS_GETCHILDRECT_HPP

/* Emphasize Windows GetChildRect routine
// Windows_GetChildRect.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: Retrieves child window rectangle in parent coordinates
//
// Changelog:
//      26.06.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetChildRect
    //  - retrieves rectangle of a child window in its parent's coordinates
    //  - parameters: HWND - child window handle
    //                LPRECT - rectangle to receive window coordinates
    //                         relative to parent's client area
    //  - returns: non-zero value on success
    //             0 on error (call GetLastError to get more details)
    
    BOOL GetChildRect (HWND hChild, LPRECT);
    BOOL GetChildRect (HWND hAncestor, HWND hChild, LPRECT);
};

#endif
