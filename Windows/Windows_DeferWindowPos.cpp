#include "Windows_DeferWindowPos.hpp"

/* Emphasize Windows simpler DeferWindowPos routine
// Windows_DeferWindowPos.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      05.07.2011 - initial version
*/

bool Windows::DeferWindowPos (HDWP & hDwp, HWND hWnd,
                              const RECT & r, UINT flags,
                              HWND hAfter) {
    
    // Early check for NULL
    //  - allowing this to ::DeferWindowPos would destroy the hDwp
    
    if (hWnd == NULL)
        return false;
    
    // Calling the actual DeferWindowPos
    //  - hWndInsertAfter = NULL as controls never need to be reordered
    //  - r.right, r.bottom are used as width and height
    
    if (hDwp) {
        HDWP hNewDwp = ::DeferWindowPos (hDwp, hWnd, hAfter,
                                         r.left, r.top, r.right, r.bottom, flags);
        if (hNewDwp) {
            
            // on success, the HDWP might be reallocated, so update the reference
            hDwp = hNewDwp;
            return true;
        };
    };
        
    // on failure, try using SetWindowPos
    
    return SetWindowPos (hWnd, NULL,
                         r.left, r.top, r.right, r.bottom, flags);
};

bool Windows::DeferWindowPos (HDWP & hDwp, HWND hWnd, UINT child,
                              const RECT & r, UINT flags,
                              HWND hAfter) {
    if (HWND hChild = GetDlgItem (hWnd, child)) {
        return Windows::DeferWindowPos (hDwp, hChild, r, flags, hAfter);
    } else
        return false;
};

bool Windows::DeferWindowPos (HDWP & hDwp, HWND hWnd,
                              long left, long top, long right, long bottom) {
    return DeferWindowPos (hDwp, hWnd, { left, top, right, bottom });
};

bool Windows::DeferWindowPos (HDWP & hDwp, HWND hWnd, UINT child,
                              long left, long top, long right, long bottom) {
    return DeferWindowPos (hDwp, hWnd, child, { left, top, right, bottom });
};
