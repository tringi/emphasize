#include "Windows_GetTrayPopupRect.hpp"

/* Windows GetTrayPopupRect 
// Windows_GetTrayPopupRect.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      07.06.2012 - initial version
*/

#include "Windows_Symbol.hpp"

POINT Windows::GetTrayPopupRect (HWND hWnd, UINT id,
                                 POINT anchor, SIZE size, int margin) {
    
    // Update anchor point
    //  - according to taskbar position (top / left / right / bottom (default))
    //  - and notification icons parent window position
 
    APPBARDATA taskbar;
    if (HWND hTaskBar = FindWindow (L"Shell_TrayWnd", NULL)) {
        taskbar.hWnd = hTaskBar;
        taskbar.cbSize = sizeof taskbar;
        
        // Taskbar edge
        //  - if this fails, assume bottom
        
        if (!SHAppBarMessage (ABM_GETTASKBARPOS, &taskbar)) {
            GetWindowRect (hTaskBar, &taskbar.rc);
            taskbar.uEdge = ABE_BOTTOM;
        };

        // Distance correctly near to tray icons
        //  - this is just auxiliary

        if (HWND hTray = FindWindowEx (hTaskBar, NULL,
                                       L"TrayNotifyWnd", NULL)) {
            RECT rTray = { 0,0,0,0 };
            
            // ToolbarWindow32 on Windows Vi/7, otherwise just the window

            if (HWND hTrayIcons = FindWindowEx (hTray, NULL,
                                                L"ToolbarWindow32", NULL)) {
                GetWindowRect (hTrayIcons, &rTray);
            } else {
                GetWindowRect (hTray, &rTray);
            };

            if (hWnd) {
    
                // Center anchor inside the icon
                //  - Windows 7 and later
    
                typedef struct _NOTIFYICONIDENTIFIER {
                    DWORD cbSize;
                    HWND hWnd;
                    UINT uID;
                    GUID guidItem;
                } NOTIFYICONIDENTIFIER, *PNOTIFYICONIDENTIFIER;
    
                HRESULT (WINAPI * Shell_NotifyIconGetRect)
                        (const NOTIFYICONIDENTIFIER *, RECT *) = NULL;
                if (Windows::Symbol (L"shell32", Shell_NotifyIconGetRect,
                                                "Shell_NotifyIconGetRect")) {
                    RECT rIcon;
                    NOTIFYICONIDENTIFIER nii = {
                        sizeof (NOTIFYICONIDENTIFIER), hWnd, id,
                        { 0, 0, 0, { 0 } } // not using GUID
                    };
                    if (Shell_NotifyIconGetRect (&nii, &rIcon) == S_OK) {
                        switch (taskbar.uEdge) {
                            case ABE_LEFT:
                            case ABE_RIGHT:
                                anchor.y = (rIcon.bottom + rIcon.top) / 2;
                                break;
                            case ABE_TOP:
                            case ABE_BOTTOM:
                                anchor.x = (rIcon.right + rIcon.left) / 2;
                                break;
                        };
                    };
                };
            };

            // Dock at the tray window border

            switch (taskbar.uEdge) {
                case ABE_TOP: anchor.y = rTray.bottom + margin; break;
                case ABE_LEFT: anchor.x = rTray.right + margin; break;
                case ABE_RIGHT: anchor.x = rTray.left - margin; break;
                case ABE_BOTTOM: anchor.y = rTray.top - margin; break;
            };
        };
    } else {
        // No Taskbar, dock at the bottom
        taskbar.uEdge = ABE_BOTTOM;
    };

    // Position near the anchor
    //  - conveniently to minimize required mouse distance

    switch (taskbar.uEdge) {
        case ABE_TOP:
            anchor.x -= size.cx / 2;
            break;
        case ABE_BOTTOM:
            anchor.y -= size.cy;
            anchor.x -= size.cx / 2;
            break;
        case ABE_LEFT:
            anchor.y -= size.cy / 2;
            break;
        case ABE_RIGHT:
            anchor.x -= size.cx;
            anchor.y -= size.cy / 2;
            break;
    };

    // Contain within the nearest monitor
    //  - of course considering required margin

    MONITORINFO monitor;
    monitor.cbSize = sizeof monitor;
    if (GetMonitorInfo (MonitorFromPoint (anchor, MONITOR_DEFAULTTONEAREST),
                        &monitor)) {
        if (anchor.y < monitor.rcMonitor.top + margin)
            anchor.y = monitor.rcMonitor.top + margin;
        if (anchor.x < monitor.rcMonitor.left + margin)
            anchor.x = monitor.rcMonitor.left + margin;

        if (anchor.y > monitor.rcMonitor.bottom - margin - size.cy)
            anchor.y = monitor.rcMonitor.bottom - margin - size.cy;
        if (anchor.x > monitor.rcMonitor.right - margin - size.cx)
            anchor.x = monitor.rcMonitor.right - margin - size.cx;
    };

    return anchor;
};

bool Windows::IsPointInTray (POINT pt) {
    if (HWND hTaskBar = FindWindow (L"Shell_TrayWnd", NULL)) {
        if (HWND hTray = FindWindowEx (hTaskBar, NULL,
                                       L"TrayNotifyWnd", NULL)) {
            RECT rTray;
            if (GetWindowRect (hTray, &rTray))
                return PtInRect (&rTray, pt);
        };
    };
    
    return false;
};
