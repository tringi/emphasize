#include "Windows_CreateControl.hpp"

/* Windows CreateControl 
// Windows_CreateControl.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      12.06.2012 - initial version
*/

#include "Windows_GetCurrentModuleHandle.hpp"

HWND Windows::CreateControl (LPCTSTR wndclass,
                             DWORD style, HWND parent, USHORT id,
                             HINSTANCE instance, LPVOID parameter) {
    return Windows::CreateControlEx (0u, wndclass, TEXT (""), style, parent,
                                     id, instance, parameter);
};

HWND Windows::CreateControl (LPCTSTR wndclass, LPCTSTR name,
                             DWORD style, HWND parent, USHORT id,
                             HINSTANCE instance, LPVOID parameter) {
    return Windows::CreateControlEx (0u, wndclass, name, style, parent,
                                     id, instance, parameter);
};

HWND Windows::CreateControlEx (DWORD extra, LPCTSTR wndclass,
                               DWORD style, HWND parent, USHORT id,
                               HINSTANCE instance, LPVOID parameter) {
    return Windows::CreateControlEx (extra, wndclass, TEXT (""), style, parent,
                                     id, instance, parameter);
};

HWND Windows::CreateControlEx (DWORD extra, LPCTSTR wndclass, LPCTSTR name,
                               DWORD style, HWND parent, USHORT id,
                               HINSTANCE instance, LPVOID parameter) {
    if (instance == NULL)
        instance = Windows::GetCurrentModuleHandle ();
    
    return CreateWindowEx (extra, wndclass, name,
                           style | WS_CHILD, 0,0,0,0,
                           parent, (HMENU) (UINT_PTR) id,
                           instance, parameter);
};
