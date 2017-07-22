#include "Windows_ChildrenContainer.hpp"

/* Windows ChildrenContainer 
// Windows_ChildrenContainer.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      28.03.2014 - initial version
*/

#include "Windows_GetChildRect.hpp"
#include "Windows_W2kCompatibility.hpp"
#include "Windows_GetSystemParameter.hpp"
#include <cstdio>

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
};

ATOM Windows::InitializeChildrenContainer (HINSTANCE hInstance) {
    if (!atom) {
        WNDCLASS wc;
        
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = Procedure;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = NULL; // LoadCursor (NULL, IDC_ARROW); // TODO: query parent
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TEXT ("ChildrenContainer");

        atom = RegisterClass (&wc);
    };
    return atom;
};

HWND Windows::CreateChildrenContainer (HINSTANCE hInstance, HWND hParent,
                                       UINT id, DWORD style) {
    return CreateWindowEx (WS_EX_CONTROLPARENT, (LPCTSTR) (INT) atom, TEXT(""),
                           WS_CHILD | WS_HSCROLL | WS_VSCROLL | style, 0,0,0,0,
                           hParent, (HMENU) id, hInstance, NULL);
};

namespace {
void UpdateScrollbars (HWND);

LRESULT OnHorizontalWheel (HWND, SHORT, USHORT);
LRESULT OnHorizontalScroll (HWND, UINT, int = WHEEL_DELTA);
LRESULT OnVerticalWheel (HWND, SHORT, USHORT);
LRESULT OnVerticalScroll (HWND, UINT, int = WHEEL_DELTA);

BOOL CALLBACK GetChildRectProc (HWND, LPARAM);
BOOL CALLBACK SetChildFontProc (HWND, LPARAM);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            UpdateScrollbars (hWnd);
            break;
        
        case WM_PARENTNOTIFY:
            switch (LOWORD (wParam)) {
                case WM_CREATE:
                case WM_DESTROY:
                    UpdateScrollbars (hWnd);
                    break;
            };
            break;
        
        case WM_SETFONT:
            EnumChildWindows (hWnd, SetChildFontProc, wParam);
            if (LOWORD (lParam)) {
                RedrawWindow (hWnd, NULL, NULL,
                              RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INVALIDATE);
            };
            break;
            
        case WM_ERASEBKGND:
            if (HWND hParent = GetParent(hWnd)) {
                POINT pt = { 0, 0 };
                MapWindowPoints (hWnd, hParent, &pt, 1);
                OffsetWindowOrgEx ((HDC) wParam, pt.x, pt.y, &pt);
                
                auto result = SendMessage (hParent, WM_ERASEBKGND, wParam, 0L);
                
                SetWindowOrgEx ((HDC) wParam, pt.x, pt.y, NULL);
                return result;
            } else
                return 0;
        
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc (hWnd, message, wParam, lParam);
            switch (hit) {
                case HTCLIENT:
                    return HTTRANSPARENT;
                default:
                    return hit;
            };
        } break;
        
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_NOTIFY:
        case WM_COMMAND:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_COMPAREITEM:
            return SendMessage (GetParent (hWnd), message, wParam, lParam);

        case WM_MOUSEWHEEL:
            return OnVerticalWheel (hWnd, HIWORD (wParam), LOWORD (wParam));
        case WM_MOUSEHWHEEL:
            return OnHorizontalWheel (hWnd, HIWORD (wParam), LOWORD (wParam));

        case WM_HSCROLL:
        case WM_VSCROLL:
            if (lParam) {
                return SendMessage (GetParent (hWnd), message, wParam, lParam);
            } else {
                switch (message) {
                    case WM_VSCROLL:
                        return OnVerticalScroll (hWnd, LOWORD (wParam));
                    case WM_HSCROLL:
                        return OnHorizontalScroll (hWnd, LOWORD (wParam));
                };
            };
            break;

        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

BOOL CALLBACK SetChildFontProc (HWND hChild, LPARAM lParam) {
    SendMessage (hChild, WM_SETFONT, lParam, FALSE);
    return TRUE;
};
BOOL CALLBACK GetChildRectProc (HWND hChild, LPARAM lParam) {
    RECT rc;
    RECT * r = reinterpret_cast <RECT *> (lParam);
    
    if (Windows::GetChildRect (hChild, &rc)) {
        if (IsWindowVisible (hChild)) {
            UnionRect (r, r, &rc);
        };
    };
    return TRUE;
};

void UpdateScrollbars (HWND hWnd) {
    RECT rc;
    if (GetClientRect (hWnd, &rc)) {

        RECT r = { 0,0,0,0 };
        EnumChildWindows (hWnd, GetChildRectProc, reinterpret_cast <LPARAM> (&r));
    
        if (r.top > 0)
            r.top = 0;
        if (r.left > 0)
            r.left = 0;
        
        SCROLLINFO hsi = { sizeof (SCROLLINFO), SIF_POS, 0,0,0,0,0 };
        SCROLLINFO vsi = { sizeof (SCROLLINFO), SIF_POS, 0,0,0,0,0 };
    
        if (!GetScrollInfo (hWnd, SB_HORZ, &hsi)) {
            hsi.nPos = 0;
        };
        if (!GetScrollInfo (hWnd, SB_VERT, &vsi)) {
            vsi.nPos = 0;
        };
            
        hsi.fMask = SIF_ALL;
        hsi.nMin = r.left;
        hsi.nMax = r.right;
        hsi.nPage = rc.right;
        
        vsi.fMask = SIF_ALL;
        vsi.nMin = r.top;
        vsi.nMax = r.bottom;
        vsi.nPage = rc.bottom;
        
        SetScrollInfo (hWnd, SB_HORZ, &hsi, TRUE);
        SetScrollInfo (hWnd, SB_VERT, &vsi, TRUE);
    };
    return;
};

LRESULT OnHorizontalScroll (HWND hWnd, UINT event, int n) {
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_TRACKPOS | SIF_RANGE | SIF_PAGE | SIF_POS;

    if (GetScrollInfo (hWnd, SB_HORZ, &si)) {
        
        const int maximum = si.nMax - si.nPage + 1;
        const int previous = si.nPos;
        
        switch (LOWORD (event)) {
            case SB_LEFT:
                si.nPos = 0u;
                break;
            case SB_RIGHT:
                si.nPos = maximum;
                break;
                
            case SB_PAGELEFT:
                si.nPos -= n * si.nPage / WHEEL_DELTA;
                break;
            case SB_PAGERIGHT:
                si.nPos += n * si.nPage / WHEEL_DELTA;
                break;
                
            case SB_LINELEFT:
                si.nPos -= n * GetSystemMetrics (SM_CXHSCROLL) / WHEEL_DELTA;
                break;
            case SB_LINERIGHT:
                si.nPos += n * GetSystemMetrics (SM_CXHSCROLL) / WHEEL_DELTA;
                break;
            
            case SB_THUMBTRACK:
/*                if (si.nPos == si.nTrackPos) {
                    int target = GetScrollPos (hWnd, SB_HORZ);
                    if (std::abs (si.nPos - target) > 1) {
                        si.nPos = target;
                    };
                } else*/
                    si.nPos = si.nTrackPos;
                break;
                
            case SB_THUMBPOSITION:
                break;

            default:
                return 0;
        };

        // ScrollBar bounds check
        //  - in order to keep the switch above clean
        
        if (si.nPos < 0)
            si.nPos = 0;
        if (si.nPos > maximum)
            si.nPos = maximum;
        
        // Update scrollbar position

        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_HORZ, &si, TRUE);
        
        // Scrolling window content
        
        ScrollWindowEx (hWnd, previous - si.nPos, 0, NULL, NULL,
                        NULL, NULL, SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN);
    };
    return 0;
};
LRESULT OnVerticalScroll (HWND hWnd, UINT event, int n) {
    LRESULT result = 0;
    
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_TRACKPOS | SIF_RANGE | SIF_PAGE | SIF_POS;

    if (GetScrollInfo (hWnd, SB_VERT, &si)) {

        RECT r;
        GetClientRect (hWnd, &r);
            
        const int maximum = si.nMax - si.nPage + 1;
        const int previous = si.nPos;

        switch (LOWORD (event)) {
            case SB_TOP:
                si.nPos = 0u;
                break;
            case SB_BOTTOM:
                si.nPos = maximum;
                break;

            case SB_PAGEUP:
                si.nPos -= n * r.bottom / WHEEL_DELTA;
                break;
            case SB_PAGEDOWN:
                si.nPos += n * r.bottom / WHEEL_DELTA;
                break;

            case SB_LINEUP:
                si.nPos -= n * GetSystemMetrics (SM_CYVSCROLL) / WHEEL_DELTA;
                break;
            case SB_LINEDOWN:
                si.nPos += n * GetSystemMetrics (SM_CYVSCROLL) / WHEEL_DELTA;
                break;

            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                si.nPos = si.nTrackPos;
                break;
            
            default:
                return 0;
        };

        // ScrollBar bounds check
        //  - in order to keep the switch above clean

        if (si.nPos < 0)
            si.nPos = 0;
        if (si.nPos > maximum)
            si.nPos = maximum;

        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_VERT, &si, TRUE);

        // Scrolling window content

        ScrollWindowEx (hWnd, 0, previous - si.nPos, NULL, NULL,
                        NULL, NULL, SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN);
    };
    return result;
};

LRESULT OnHorizontalWheel (HWND hWnd, SHORT distance, USHORT flags) {
    int chars = Windows::GetSystemParameter <SPI_GETWHEELSCROLLCHARS> ();
    return OnHorizontalScroll (hWnd,
                               (flags & MK_CONTROL) ? SB_PAGERIGHT : SB_LINERIGHT,
                               chars * distance);
};
LRESULT OnVerticalWheel (HWND hWnd, SHORT distance, USHORT flags) {
    
    // SPI_GETWHEELSCROLLLINES
    //  - user setting on how many lines should scroll wheel actually scroll
    //  - -1u (WHEEL_PAGESCROLL == UINT_MAX) is specified to scroll the whole
    //    page and the same usually does holding CTRL, so do the same
    
    int lines = Windows::GetSystemParameter <SPI_GETWHEELSCROLLLINES> ();
    return OnVerticalScroll (hWnd,
                             (lines == -1 || flags & MK_CONTROL) ? SB_PAGEUP : SB_LINEUP,
                             lines * distance);
};

};
