#include "Shadow_Editor_Index.hpp"

/* Emphasize Shadow Controls Library - Editor Index
// Shadow_Editor_Index.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      04.02.2012 - initial version
*/

#include "../Windows/Windows_GetChildRect.hpp"
#include "../Windows/Windows_UxTheme.hpp"

#include <cstdio>
#include <cwchar>

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    
    struct Data {
        HFONT hFont;
        
        struct {
            unsigned int left;
            unsigned int right;
        } padding;
        struct {
            unsigned int row;
            unsigned int fraction;
        } offset;
        
        unsigned int rowheight;
        unsigned int numbers;
    };
};

ATOM Shadow::Editor::Index::Initialize (HINSTANCE hInstance) {
    if (!atom) {
        WNDCLASS wc;

        wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = Procedure;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof (Data *);
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TEXT ("Shadow::Editor::Index");

        atom = RegisterClass (&wc);
    };
    return atom;
};

HWND Shadow::Editor::Index::Create (HINSTANCE hInstance,
                                    HWND hParent, UINT style, UINT id) {

    return CreateWindow ((LPCTSTR) (INT) atom, TEXT (""), WS_CHILD | style,
                         0,0,0,0, hParent, (HMENU) id, hInstance, NULL);
};

namespace {
using namespace Windows;
using namespace Shadow::Editor;

LRESULT OnPaint (HWND, HDC, RECT);
LRESULT OnClick (HWND, UINT, USHORT, SHORT, SHORT);
LRESULT OnMouseMove (HWND, SHORT, SHORT);
LRESULT OnMouseHover (HWND, SHORT, SHORT);

LRESULT SendRequest (HWND, UINT);
LRESULT CustomDrawNotify (HWND, DWORD, HDC, LPCRECT, UINT = 0u, LPARAM = 0u);
unsigned int GetRowHeight (HWND, unsigned int, unsigned int);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            if (void * ptr = HeapAlloc (GetProcessHeap (),
                                        HEAP_ZERO_MEMORY, sizeof (Data))) {
                SetWindowLongPtr (hWnd, 0, reinterpret_cast <LONG_PTR> (ptr));
                
                static_cast <Data *> (ptr) ->hFont = (HFONT) GetStockObject (ANSI_VAR_FONT);
                return 0;
            } else
                return -1;
        
        case WM_DESTROY:
            HeapFree (GetProcessHeap (), 0,
                      reinterpret_cast <void *> (GetWindowLongPtr (hWnd, 0)));
            break;
        
        case WM_SETFONT:
            reinterpret_cast <Data *>
                             (GetWindowLongPtr (hWnd, 0)) ->hFont = (HFONT) wParam;
            if (LOWORD (lParam))
                InvalidateRect (hWnd, NULL, FALSE);
            break;
        case WM_GETFONT:
            return (LRESULT) reinterpret_cast <Data *>
                                              (GetWindowLongPtr (hWnd, 0)) ->hFont;
            
        case Shadow::Editor::Index::Message::SetOffset:
            if (Data * data = reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0))) {
                data->offset.row = wParam;
                data->offset.fraction = lParam;
                RedrawWindow (hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            };
            break;
        case Shadow::Editor::Index::Message::SetPadding:
            if (Data * data = reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0))) {
                data->padding.left = wParam;
                data->padding.right = lParam;
            };
            break;
        case Shadow::Editor::Index::Message::SetDefaultRowHeight:
            if (Data * data = reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0))) {
                data->rowheight = wParam;
            };
            break;
        case Shadow::Editor::Index::Message::SetNumbersCount:
            if (Data * data = reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0))) {
                data->numbers = wParam;
            };
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
        
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };

    return 0;
};

LRESULT OnPaint (HWND hWnd, HDC _hDC, RECT clip) {
    const Data * data = reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0));
    HWND hParent = GetParent (hWnd);

    RECT rc;
    GetClientRect (hWnd, &rc);

    RECT rr = {
        (long) data->padding.left,
        (long) -data->offset.fraction,
        (long) rc.right - (long) data->padding.right,
        (long) 0
    };
    RECT rStatic = {
        0,0,
        (long) rc.right - (long) data->padding.right + 2,
        (long) rc.bottom
    };
    RECT rEdit = {
        rStatic.right, 0,
        rc.right, rc.bottom
    };
    
    HDC hDC = NULL;
    HANDLE hBuffered = UxTheme::BeginBufferedPaint (_hDC, &rc, &hDC);

    if (!hBuffered)
        hDC = _hDC;

    FillRect (hDC, &rStatic, (HBRUSH)
              SendMessage (hParent, WM_CTLCOLORSTATIC,
                           (WPARAM) hDC, (LPARAM) hWnd));
    FillRect (hDC, &rEdit, (HBRUSH)
              SendMessage (hParent, WM_CTLCOLOREDIT,
                           (WPARAM) hDC, (LPARAM) hWnd));
    
    SetBkMode (hDC, TRANSPARENT);
    
    DWORD cdnPrePaint = CustomDrawNotify (hWnd, CDDS_PREPAINT, hDC, &rc);
    if (!(cdnPrePaint & CDRF_SKIPDEFAULT)) {

        unsigned int row = data->offset.row;
        const unsigned int rows = SendRequest (hWnd, Request::Rows);
    
        while ((row < rows) && (rr.top < rc.bottom)) {
            unsigned int height = GetRowHeight (hWnd, data->rowheight, row);
            if (height) {
                rr.bottom = rr.top + height;

                RECT rtemp;
                if (IntersectRect (&rtemp, &rr, &clip)) {
                    SelectObject (hDC, data->hFont);

                    DWORD cdnPreItemPaint = 0;
                    if (cdnPrePaint & CDRF_NOTIFYITEMDRAW) {
                        cdnPreItemPaint = CustomDrawNotify (hWnd,
                                                            CDDS_ITEMPREPAINT,
                                                            hDC, &rr, row);
                    };
                    if (!(cdnPreItemPaint & CDRF_SKIPDEFAULT)) {
                        wchar_t szFormat [16u];
                        wchar_t szNumbers [16u];
                        
                        szFormat [0] = L'%';
                        szFormat [1] = L'\0';
                        
                        if (GetWindowLongPtr (hWnd, GWL_STYLE) & Index::Style::ZerosPrefix) {
                            _snwprintf (szFormat, 16u, L"%%0%u", data->numbers);
                        };
                        
                        std::wcscat (szFormat, L"u");
                        _snwprintf (szNumbers, 16u, szFormat, row + 1u);

                        DrawText (hDC, szNumbers, -1, &rr,
                                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
                    };
                    if (cdnPreItemPaint & CDRF_NOTIFYPOSTPAINT) {
                        CustomDrawNotify (hWnd, CDDS_ITEMPOSTPAINT,
                                          hDC, &rr, row);
                    };
                };
                rr.top += height;
            };
            ++row;
        };
    };

    if (cdnPrePaint & CDRF_NOTIFYPOSTPAINT) {
        CustomDrawNotify (hWnd, CDDS_POSTPAINT, hDC, &rc);
    };

    if (hBuffered)
        UxTheme::EndBufferedPaint (hBuffered);

    return 0;
};

unsigned int GetRowHeight (HWND hWnd, unsigned int value, unsigned int row) {
    NMValue nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Height },
        row, value
    };
    if (SendMessage (GetParent (hWnd), WM_NOTIFY,
                     nm.hdr.idFrom, (LPARAM) &nm)) {
        return nm.value;
    } else
        return value;
};
LRESULT SendRequest (HWND hWnd, UINT code) {
    NMHDR nm = {
        hWnd, (UINT) GetDlgCtrlID (hWnd), code
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.idFrom, (LPARAM) &nm);
};
LRESULT CustomDrawNotify (HWND hWnd, DWORD stage, HDC hDC, LPCRECT rc,
                          UINT item, LPARAM lParam) {
    NMCUSTOMDRAW nmCustomDraw = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_CUSTOMDRAW },
        stage, hDC, *rc, item, CDIS_DEFAULT, lParam
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmCustomDraw.hdr.idFrom, (LPARAM) &nmCustomDraw);
};
};
