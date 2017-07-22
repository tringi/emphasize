#ifndef WINDOWS_GETFIRSTCHILD_HPP
#define WINDOWS_GETFIRSTCHILD_HPP

/* Emphasize Windows GetFirstChild routine
// Windows_GetFirstChild.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: Retrieves first child window of the parent window
//
// Changelog:
//      07.10.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetFirstChild
    //  - retrieves handle of a child window
    //  - parameters: HWND - parent window handle
    //  - returns: - handle to the child window
    //             - NULL if there window has no child or on error (call
    //               GetLastError to get more details)
    //  - note: if the window has more than one child window, which handle
    //          is returned is undefined
    
    HWND GetFirstChild (HWND);
    
    // GetFirstVisibleChild
    //  - retrieves handle of a first visible child window
    //  - parameters: HWND - parent window handle
    //  - returns: - handle to the child window
    //             - NULL if there window has no visible child or on error (call
    //               GetLastError to get more details)
    //  - note: if the window has more than one visible child window, which
    //          handle is returned is undefined
    
    HWND GetFirstVisibleChild (HWND);
};

#endif
