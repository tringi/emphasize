#ifndef WINDOWS_GETTRAYPOPUPRECT_HPP
#define WINDOWS_GETTRAYPOPUPRECT_HPP

/* Windows GetTrayPopupRect 
// Windows_GetTrayPopupRect.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      07.06.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetTrayPopupRect
    //  - determines proper location of a helper popup window tracked from
    //    the anchor point, based by the taskbar dock border, tray position,
    //    and exact notification icon position (if possible to determine)
    //    fixed to fit into active monitor boundaries
    //  - parameters: HWND - notification icon parent window handle
    //                     - can be NULL to ignore exact icon position and is
    //                       ignored on Windows Vista and previous
    //                UINT - notification icon identifier
    //                     - not used if HWND is NULL (or otherwise ignored)
    //                POINT - anchor point
    //                SIZE - the size (heigh/width) of the moving rectangle
    //                int  - margin, distance from the notification icon
    //  - return value: new top/left coordinates of the window
    
    POINT GetTrayPopupRect (HWND, UINT, POINT, SIZE, int);
    
    // IsPointInTray
    //  - returns true if POINT (mouse cursor) is inside/above Notification
    //    Icons Taskbar window
    
    bool IsPointInTray (POINT);
};

#endif

