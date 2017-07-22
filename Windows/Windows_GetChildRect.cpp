#include "Windows_GetChildRect.hpp"

/* Emphasize Windows GetChildRect routine
// Windows_GetChildRect.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      26.06.2011 - initial version
*/

BOOL Windows::GetChildRect (HWND hCtrl, LPRECT rcCtrl) {
    if (HWND hWnd = GetAncestor (hCtrl, GA_PARENT)) {
        if (GetClientRect (hCtrl, rcCtrl)) {
            MapWindowPoints (hCtrl, hWnd, reinterpret_cast <POINT *> (rcCtrl), 2u);
            return TRUE;
        } else
            return FALSE;
    } else {
        SetLastError (ERROR_NOT_CHILD_WINDOW);
        return FALSE;
    };
};

BOOL Windows::GetChildRect (HWND hParent, HWND hCtrl, LPRECT rcCtrl) {
    if (GetClientRect (hCtrl, rcCtrl)) {
        MapWindowPoints (hCtrl, hParent, reinterpret_cast <POINT *> (rcCtrl), 2u);
        return TRUE;
    } else
        return FALSE;
};
