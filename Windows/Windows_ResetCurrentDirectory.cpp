#include "Windows_ResetCurrentDirectory.hpp"

/* Emphasize Windows current directory service helper
// Windows_ResetCurrentDirectory.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      29.07.2011 - initial version
*/

#include <windows.h>
#include "Windows_GetCurrentModuleHandle.hpp"

#include <cwchar>

bool Windows::ResetCurrentDirectory () {
    TCHAR path [32768u];
    path [0] = L'\0';
    
    if (GetModuleFileName (Windows::GetCurrentModuleHandle (),
                           path, sizeof path / sizeof path [0])) {
        if (wchar_t * last = std::wcsrchr (path, L'\\')) {
            last [1] = '\0';
        };
        return SetCurrentDirectory (path);
    } else
        return false;
};
