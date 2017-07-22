#ifndef WINDOWS_CREATECONTROL_HPP
#define WINDOWS_CREATECONTROL_HPP

/* Windows CreateControl 
// Windows_CreateControl.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      12.06.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreateControl(Ex)
    //  - 
    //  - the WS_CHILD style is added to style parameter
    //  - if HINSTANCE is left NULL, then the GetCurrentModuleHandle is used
    
    HWND CreateControl (LPCTSTR wndclass, DWORD style, HWND parent, USHORT id,
                        HINSTANCE = NULL, LPVOID = NULL);
    HWND CreateControl (LPCTSTR wndclass, LPCTSTR name, DWORD style, HWND parent, USHORT id,
                        HINSTANCE = NULL, LPVOID = NULL);

    HWND CreateControlEx (DWORD exstyle, LPCTSTR, LPCTSTR, DWORD, HWND, USHORT id,
                          HINSTANCE = NULL, LPVOID = NULL);
    HWND CreateControlEx (DWORD exstyle, LPCTSTR, DWORD, HWND, USHORT id,
                          HINSTANCE = NULL, LPVOID = NULL);
};

#endif

