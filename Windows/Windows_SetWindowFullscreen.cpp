#include "Windows_SetWindowFullscreen.hpp"

/* Windows SetWindowFullscreen 
// Windows_SetWindowFullscreen.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      05.03.2014 - initial version
*/

namespace {
    bool UnsafeSetWindowFullscreen (HWND hWnd);
    bool UnsafeRestoreFromFullscreen (HWND hWnd);
    
    static const wchar_t pp [] = L"FULLSCREEN";
    static const DWORD maskadd = WS_POPUP;
    static const DWORD maskrem = WS_CAPTION | WS_DLGFRAME | WS_SIZEBOX
                               | WS_HSCROLL | WS_VSCROLL | WS_SYSMENU
                               | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
};

bool Windows::SetWindowFullscreen (HWND hWnd, bool set) {
    if (set) {
        if (!IsWindowFullscreen (hWnd)) {
            return UnsafeSetWindowFullscreen (hWnd);
        } else
            return false;
    } else
        return RestoreFromFullscreen (hWnd);
};

bool Windows::RestoreFromFullscreen (HWND hWnd) {
    if (IsWindowFullscreen (hWnd)) {
        return UnsafeRestoreFromFullscreen (hWnd);
    } else
        return false;
};

bool Windows::IsWindowFullscreen (HWND hWnd) {
    auto style = GetWindowLongPtr (hWnd, GWL_STYLE);
    if (style & maskadd)
        if (!(style & maskrem))
            if (IsZoomed (hWnd))
                return true;
    
    return false;
};

bool Windows::ToggleWindowFullscreen (HWND hWnd) {
    if (Windows::IsWindowFullscreen (hWnd)) {
        return UnsafeRestoreFromFullscreen (hWnd);
    } else {
        return UnsafeSetWindowFullscreen (hWnd);
    };
};


namespace {
    struct Data {
        DWORD style;
        RECT position;
    };
    
    bool UnsafeSetWindowFullscreen (HWND hWnd) {
        if (IsZoomed (hWnd)) {
            ShowWindow (hWnd, SW_RESTORE);
        };
        if (auto previous = RemoveProp (hWnd, pp)) {
            HeapFree (GetProcessHeap (), 0, previous);
        };
        
        if (auto data = static_cast <Data *> (HeapAlloc (GetProcessHeap (), 0, sizeof (Data)))) {
            
            if (GetWindowRect (hWnd, &data->position)) {
                data->style = GetWindowLongPtr (hWnd, GWL_STYLE);
                if (SetProp (hWnd, pp, data)) {
                    
                    SetWindowLongPtr (hWnd, GWL_STYLE, (data->style | maskadd) & ~maskrem);
                    ShowWindow (hWnd, SW_MAXIMIZE);
                    return true;
                };
            };
            
            HeapFree (GetProcessHeap (), 0, data);
        };
        
        return false;
    };
    bool UnsafeRestoreFromFullscreen (HWND hWnd) {
        if (auto previous = RemoveProp (hWnd, pp)) {
            
            Data data = *static_cast <Data *> (previous);
            HeapFree (GetProcessHeap (), 0, previous);
            
            SetWindowLongPtr (hWnd, GWL_STYLE, data.style);
            ShowWindow (hWnd, SW_RESTORE);

            return MoveWindow (hWnd, data.position.left, data.position.top,
                               data.position.right - data.position.left,
                               data.position.bottom - data.position.top, TRUE);
        } else
            return false;
    };
};

