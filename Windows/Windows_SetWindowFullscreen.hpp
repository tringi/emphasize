#ifndef WINDOWS_SETWINDOWFULLSCREEN_HPP
#define WINDOWS_SETWINDOWFULLSCREEN_HPP

/* Windows SetWindowFullscreen 
// Windows_SetWindowFullscreen.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      05.03.2014 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // SetWindowFullscreen
    //  - alters window style and position to display fullscreen on its current
    //    display (if the bool parameter is false, calls RestoreFromFullscreen)
    //  - stores previous settings (style and position) as 'prop'
    //  - returns true on success, false otherwise
    
    bool SetWindowFullscreen (HWND, bool = true);
    
    // RestoreFromFullscreen
    //  - restores style and position saved by the SetWindowFullscreen
    //    and deletes the 'prop' stored with the window
    //  - returns true on success, false otherwise (e.g. when not fullscreen)
    
    bool RestoreFromFullscreen (HWND);
    
    // IsWindowFullscreen
    //  - checks window style and maximized state to determine if fullscreen
    //  - returns true when fullscreen, false otherwise
    
    bool IsWindowFullscreen (HWND);
    
    // ToggleWindowFullscreen
    //  - small helper function to toggle fullscreen status of the window
    
    bool ToggleWindowFullscreen (HWND hWnd);
    
    
};

#endif

