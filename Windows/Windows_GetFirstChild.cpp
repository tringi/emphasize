#include "Windows_GetFirstChild.hpp"

/* Emphasize Windows GetFirstChild routine
// Windows_GetFirstChild.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: Retrieves first child window of the parent window
//
// Changelog:
//      07.10.2011 - initial version
*/

namespace {
    BOOL CALLBACK EnumerationProcedure (HWND, LPARAM);
    BOOL CALLBACK VisibleEnumerationProcedure (HWND, LPARAM);
};

HWND Windows::GetFirstChild (HWND hParent) {
    HWND result = NULL;
    EnumChildWindows (hParent, EnumerationProcedure,
                      reinterpret_cast <LPARAM> (&result));
    
    return result;
};

HWND Windows::GetFirstVisibleChild (HWND hParent) {
    HWND result = NULL;
    EnumChildWindows (hParent, VisibleEnumerationProcedure,
                      reinterpret_cast <LPARAM> (&result));
    
    return result;
};

namespace {
    BOOL CALLBACK EnumerationProcedure (HWND hWnd, LPARAM lParam) {
        *reinterpret_cast <HWND *> (lParam) = hWnd;
        return FALSE;
    };
    BOOL CALLBACK VisibleEnumerationProcedure (HWND hWnd, LPARAM lParam) {
        if (IsWindowVisible (hWnd)) {
            *reinterpret_cast <HWND *> (lParam) = hWnd;
            return FALSE;
        } else
            return TRUE;
    };
};
