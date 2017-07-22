#include "Shadow_Splitter.hpp"

/* Emphasize Shadow Controls Library - Splitter bar
// Shadow_Splitter.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      19.08.2011 - initial version
*/

#include "../Windows/Windows_WindowExtraMemory.hpp"
#include "../Windows/Windows_W2kCompatibility.hpp"
#include "../Windows/Windows_GetChildRect.hpp"
#include "../Windows/Windows_UxTheme.hpp"

#include <algorithm>

using namespace Windows;
using namespace Shadow::Splitter;

namespace {
    ATOM atom = 0u;
    HCURSOR hCursor [4] = { NULL, NULL, NULL, NULL };
    
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);

    struct Data {
        LPARAM  position;
        int     destination_x;
        int     destination_y;
    };
};

ATOM Shadow::Splitter::Initialize (HINSTANCE hInstance) {
    if (!atom) {
        WNDCLASS wc;
        
        wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = Procedure;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = GetWindowExtraSize <Data> ();
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TEXT ("Shadow::Splitter");
    
        hCursor [0] = LoadCursor (NULL, IDC_NO);
        hCursor [1] = LoadCursor (NULL, IDC_SIZEWE);
        hCursor [2] = LoadCursor (NULL, IDC_SIZENS);
        hCursor [3] = LoadCursor (NULL, IDC_SIZEALL);
    
        atom = RegisterClass (&wc);
    };
    return atom;
};

HWND Shadow::Splitter::Create (HINSTANCE hInstance,
                               HWND hParent, UINT style, UINT id) {
    
    return CreateWindowEx (0, (LPCTSTR) (INT) atom, TEXT(""), style | WS_CHILD,
                           0,0,0,0, hParent, (HMENU) id, hInstance, NULL);
};

namespace {

LRESULT Translate (HWND, LONG, LONG);
LRESULT Animate (HWND);

LRESULT OnPaint (HWND, HDC, RECT);
LRESULT OnMouseMove (HWND, SHORT, SHORT);
LRESULT OnAction (HWND, WPARAM);
LRESULT OnKeyDown (HWND, WPARAM, LPARAM);

LRESULT SendBasicNotify (HWND, UINT);
LRESULT CustomDrawNotify (HWND, DWORD, HDC, LPCRECT,
                          UINT = 0, UINT = CDIS_DEFAULT, LPARAM = 0);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SendMessage (hWnd, WM_CHANGEUISTATE, UIS_INITIALIZE, 0u);
            break;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            if (HDC hDC = BeginPaint (hWnd, &ps)) {
                OnPaint (hWnd, hDC, ps.rcPaint);
            };
            EndPaint (hWnd, &ps);
        } break;
        case WM_PRINTCLIENT: {
            RECT rc;
            if (GetClientRect (hWnd, &rc)) {
                OnPaint (hWnd, (HDC) wParam, rc);
            };
        } break;
        
        case WM_SETFOCUS:
            InvalidateRect (hWnd, NULL, FALSE);
            return SendBasicNotify (hWnd, NM_SETFOCUS);
        case WM_KILLFOCUS:
            InvalidateRect (hWnd, NULL, FALSE);
            return SendBasicNotify (hWnd, NM_KILLFOCUS);
        
        case WM_MOUSEMOVE:
            return OnMouseMove (hWnd, LOWORD (lParam), HIWORD (lParam));
        
        case WM_RBUTTONDOWN:
            SendBasicNotify (hWnd, NM_RCLICK);
            break;
        case WM_LBUTTONDOWN:
            if (SendBasicNotify (hWnd, NM_CLICK) == 0) {
                SetFocus (hWnd);
                SetCapture (hWnd);
                SendBasicNotify (hWnd, NM_SETFOCUS);
                SetWindowExtra (hWnd, &Data::position, lParam);
            };
            break;
        case WM_LBUTTONUP:
            ReleaseCapture ();
            SendBasicNotify (hWnd, NM_RELEASEDCAPTURE);
            break;
            
        case WM_LBUTTONDBLCLK:
            return OnAction (hWnd, wParam);
        case WM_KEYDOWN:
            return OnKeyDown (hWnd, wParam, lParam);
        
        case WM_TIMER:
            switch (wParam) {
                case 1u:
                    return Animate (hWnd);
            };
            break;
        
        case WM_SIZE:
        case WM_UPDATEUISTATE:
        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS;

        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT Translate (HWND hWnd, LONG x, LONG y) {
    
    RECT original;
    if (Windows::GetChildRect (hWnd, &original)) {
        
        const UINT style = GetWindowLongPtr (hWnd, GWL_STYLE);
        const LPARAM position = GetWindowExtra (hWnd, &Data::position);

        // Request::Moving notify
        //  - first initialize with current position
        
        NMMoving nmMoving = {
            { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Moving },
            { original.left, original.top },
            { x, y },
            { LOWORD (position), HIWORD (position) },
            original
        };
        NMBounds nmBounds = {
            { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Bounds },
            { 0, 0, 0, 0 }
        };
        
        // update requested rectangle
        
        nmMoving.target.right  -= nmMoving.target.left;
        nmMoving.target.bottom -= nmMoving.target.top;
        
        if (style & Style::Horizontal) {
            nmMoving.target.left += x;
        };
        if (style & Style::Vertical) {
            nmMoving.target.top += y;
        };
        
        // default bounding rectangle is the client area of the parent window
        
        GetClientRect (GetParent (hWnd), &nmBounds.bounds);

        // confine the final position into bounds
        //  - also confine offset to real values
        
        if (SendMessage (GetParent (hWnd), WM_NOTIFY,
                         nmBounds.hdr.idFrom, (LPARAM) &nmBounds)) {
            if (style & Style::Horizontal) {
                if (nmMoving.target.left < nmBounds.bounds.left)
                    nmMoving.target.left = nmBounds.bounds.left;
                if (nmMoving.target.left > nmBounds.bounds.right - nmMoving.target.right)
                    nmMoving.target.left = nmBounds.bounds.right - nmMoving.target.right;

                nmMoving.offset.x = nmMoving.target.left - original.left;
            };
            if (style & Style::Vertical) {
                if (nmMoving.target.top < nmBounds.bounds.top)
                    nmMoving.target.top = nmBounds.bounds.top;
                if (nmMoving.target.top > nmBounds.bounds.bottom - nmMoving.target.bottom)
                    nmMoving.target.top = nmBounds.bounds.bottom - nmMoving.target.bottom;

                nmMoving.offset.y = nmMoving.target.top - original.top;
            };
        };

        // send the callback message
        //  - allowing user to suppress actual move by returning non-zero

        if (!SendMessage (GetParent (hWnd), WM_NOTIFY,
                          nmMoving.hdr.idFrom, (LPARAM) &nmMoving)) {
            MoveWindow (hWnd,
                        nmMoving.target.left, nmMoving.target.top,
                        nmMoving.target.right, nmMoving.target.bottom,
                        TRUE);
        };
    };
    return 0;
};

LRESULT OnPaint (HWND hWnd, HDC hDC, RECT) {
    RECT rc;
    GetClientRect (hWnd, &rc);
    
    DWORD rvPreErase = CustomDrawNotify (hWnd, CDDS_PREERASE, hDC, &rc);
    
    if (!(rvPreErase & CDRF_SKIPDEFAULT)) {
        FillRect (hDC, &rc, (HBRUSH)
                  SendMessage (GetParent (hWnd), WM_CTLCOLORSTATIC,
                               (WPARAM) hDC, (LPARAM) hWnd));
        
        if (rvPreErase & CDRF_NOTIFYPOSTERASE) {
            CustomDrawNotify (hWnd, CDDS_POSTERASE, hDC, &rc);
        };
    };

    DWORD rvPrePaint = CustomDrawNotify (hWnd, CDDS_PREPAINT, hDC, &rc);

    if (!(rvPrePaint & CDRF_SKIPDEFAULT)) {
        if (!(rvPrePaint & CDRF_SKIPPOSTPAINT)) { 
            if (GetFocus () == hWnd) {
                RECT r = {
                    rc.left + 1,
                    rc.top + 1,
                    rc.right - 1,
                    rc.bottom - 1
                };
                
                if (!(LOWORD (SendMessage (GetParent (hWnd), WM_QUERYUISTATE, 0,0))
                                                           & UISF_HIDEFOCUS)) {
                    DrawFocusRect (hDC, &r);
                };
            };
        };
    };

    if (rvPrePaint & CDRF_NOTIFYPOSTPAINT) {
        CustomDrawNotify (hWnd, CDDS_POSTPAINT, hDC, &rc);
    };
    return 0;
};

LRESULT OnMouseMove (HWND hWnd, SHORT x, SHORT y) {
    const UINT style = GetWindowLongPtr (hWnd, GWL_STYLE);
    
    if (HCURSOR hAppCursor = (HCURSOR) SendBasicNotify (hWnd, Request::Cursor))
        SetCursor (hAppCursor);
    else
        SetCursor (hCursor [style & (Style::Horizontal | Style::Vertical)]);
    
    if (GetCapture () == hWnd) {
        LPARAM position = GetWindowExtra (hWnd, &Data::position);
        return Translate (hWnd,
                          x - (SHORT) LOWORD (position),
                          y - (SHORT) HIWORD (position)); 
    } else
        return 0;
};

LRESULT OnKeyDown (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    NMKEY nmKey = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_KEYDOWN },
        wParam, (UINT) lParam
    };
    if (!SendMessage (GetParent (hWnd), WM_NOTIFY,
                      nmKey.hdr.idFrom, (LPARAM) &nmKey)) {
        switch (wParam) {
            case VK_UP:
                Translate (hWnd, 0, -GetSystemMetrics (SM_CYFRAME));
                break;
            case VK_DOWN:
                Translate (hWnd, 0, +GetSystemMetrics (SM_CYFRAME));
                break;
            case VK_LEFT:
                Translate (hWnd, -GetSystemMetrics (SM_CXFRAME), 0);
                break;
            case VK_RIGHT:
                Translate (hWnd, +GetSystemMetrics (SM_CXFRAME), 0);
                break;
            
            case VK_SPACE:
                OnAction (hWnd, (GetKeyState (VK_SHIFT) ? MK_SHIFT : 0)
                              | (GetKeyState (VK_CONTROL) ? MK_CONTROL : 0));
                break;
        };

        SendMessage (hWnd, WM_CHANGEUISTATE,
                     MAKELONG (UIS_CLEAR, UISF_HIDEFOCUS), 0);
        InvalidateRect (hWnd, NULL, FALSE);
    };
    return 0;
};

LRESULT OnAction (HWND hWnd, WPARAM) {
    NMHome nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Home },
        { 0, 0 }
    };
    
    SendMessage (GetParent (hWnd), WM_NOTIFY,
                 nm.hdr.idFrom, (LPARAM) &nm);

    RECT r;
    UINT style = GetWindowLongPtr (hWnd, GWL_STYLE);
    
    if (Windows::GetChildRect (hWnd, &r)) {
        
        if ((r.left == nm.home.x || !(style & Style::Horizontal)) &&
            (r.top  == nm.home.y || !(style & Style::Vertical))) {
            
            nm.hdr.code = Request::Away;
            SendMessage (GetParent (hWnd), WM_NOTIFY,
                         nm.hdr.idFrom, (LPARAM) &nm);
        };

        SetWindowExtra (hWnd, &Data::destination_x, nm.home.x);
        SetWindowExtra (hWnd, &Data::destination_y, nm.home.y);
        SetTimer (hWnd, 1, 20, NULL);
    };
    return 0;
};

LRESULT Animate (HWND hWnd) {
    RECT position;
    if (Windows::GetChildRect (hWnd, &position)) {
        
        POINT distance = { 0, 0 };
        UINT style = GetWindowLongPtr (hWnd, GWL_STYLE);
        
        if (style & Style::Horizontal) {
            distance.x = GetWindowExtra (hWnd, &Data::destination_x)
                       - position.left;
            if (std::abs (distance.x) > 1) {
                distance.x /= 2;
            };
        };
        if (style & Style::Vertical) {
            distance.y = GetWindowExtra (hWnd, &Data::destination_y)
                       - position.top;
            if (std::abs (distance.y) > 1) {
                distance.y /= 2;
            };
        };
        
        if (distance.x || distance.y) {
            Translate (hWnd, distance.x, distance.y);
        } else {
            KillTimer (hWnd, 1u);
        };
    };
    return 0;
};

LRESULT SendBasicNotify (HWND hWnd, UINT code) {
    NMHDR nm = {
        hWnd, (UINT) GetDlgCtrlID (hWnd), code
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.idFrom, (LPARAM) &nm);
};
LRESULT CustomDrawNotify (HWND hWnd, DWORD stage, HDC hDC, LPCRECT rc,
                          UINT item, UINT state, LPARAM lParam) {
    NMCUSTOMDRAW nmCustomDraw = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_CUSTOMDRAW },
        stage, hDC, *rc, item, state, lParam
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmCustomDraw.hdr.idFrom, (LPARAM) &nmCustomDraw);
};

};
