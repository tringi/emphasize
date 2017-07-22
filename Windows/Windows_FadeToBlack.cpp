#include "Windows_FadeToBlack.hpp"

/* Windows API Fading screen rectangle
// Windows_FadeToBlack.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      01.11.2011 - initial version
//      25.01.2013 - added FadeToColor
*/

#include <stdio.h>

namespace {
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM); 
};

HWND Windows::FadeToBlack (HINSTANCE hInstance, const RECT & r, BOOL always_on_top) {
    return Windows::FadeToColor (hInstance, r, 0x000000, always_on_top);
};

HWND Windows::FadeToColor (HINSTANCE hInstance, const RECT & r,
                           COLORREF cr, BOOL always_on_top) {

    
    UINT style = WS_VISIBLE | WS_POPUP;
    UINT extra = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    if (always_on_top) {
        extra |= WS_EX_TOPMOST;
    };
    
    if (HWND hWnd = CreateWindowEx (extra, L"BLANK", NULL, style,
                                    r.left, r.top, r.right - r.left, r.bottom - r.top,
                                    NULL, NULL, hInstance, (LPVOID) Procedure)) {
        
        SetWindowLongPtr (hWnd, GWLP_USERDATA, cr & 0x00FFFFFF);

        MSG message = { NULL, 0u, 0u, 0u, 0u, { 0, 0 }};
        do switch (GetMessage (&message, NULL, 0u, 0u)) {
            default:
                DispatchMessage (&message);
                break;
            
            case  0: // WM_QUIT
                if (message.wParam == ERROR_SUCCESS) {
                    return hWnd;
                } else {
                    DestroyWindow (hWnd);
                    return NULL;
                };
                    
            case -1: // Error
                DestroyWindow (hWnd);
                return NULL;
        } while (true);
    };
    
    return NULL;
};

namespace {
LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetTimer (hWnd, 1u, 10u, NULL);
            break;
        
        case WM_SETCURSOR:
            SetCursor (NULL);
            return TRUE;

        case WM_TIMER:
            switch (wParam) {
                case 1u:
                    auto data = GetWindowLongPtr (hWnd, GWLP_USERDATA);
                    auto alpha = ((unsigned int) data) >> 24u;
                    
                    if (alpha < 255u) {
        
                        // allowed increments are
                        //  - 1 - for 2.5 seconds long intro
                        //  - 3 - for 850 ms long into
                        //  - 5 - for 510 ms long into
                        //  - 15 - for 170 ms long into
        
                        alpha += 15u;
                        
                        SetLayeredWindowAttributes (hWnd, 0, alpha, LWA_ALPHA);
                        SetWindowLongPtr (hWnd, GWLP_USERDATA,
                                          (alpha << 24u) | (data & 0x00FFFFFFu));
                    } else {
                        PostQuitMessage (ERROR_SUCCESS);
                    };
                    break;
            };
            break;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            if (HDC hDC = BeginPaint (hWnd, &ps)) {
                SetDCBrushColor (hDC, GetWindowLongPtr (hWnd, GWLP_USERDATA) & 0x00FFFFFFu);
                FillRect (hDC, &ps.rcPaint,
                          (HBRUSH) GetStockObject (DC_BRUSH));
                EndPaint (hWnd, &ps);
            };
        } break;
            
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    return 0;
};
};

