#include "Shadow_Grid.hpp"

/* Emphasize Shadow Controls Library - Grid 
// Shadow_Grid.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.9
//
// Changelog:
//      10.01.2014 - initial version
//      25.01.2014 - first usable version 0.9 for Konipas project
*/

#include "../Windows/Windows_GetSystemParameter.hpp"
#include "../Windows/Windows_WindowExtraMemory.hpp"
#include "../Windows/Windows_W2kCompatibility.hpp"
#include "../Windows/Windows_TrackMouseEvent.hpp"
#include "../Windows/Windows_DeferWindowPos.hpp"
#include "../Windows/Windows_CreateControl.hpp"
#include "../Windows/Windows_UxTheme.hpp"
#include "../Windows/Windows_Version.hpp"
#include "../Windows/Windows_Print.hpp"

#include <commctrl.h>

#include <algorithm>
#include <cwctype>
#include <cwchar>
#include <cstdio>

using namespace Windows;
using namespace Shadow::Grid;

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK HeaderSubclass (HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK EditorSubclass (HWND, UINT, WPARAM, LPARAM);
    
    HANDLE hHeap = NULL;
    HCURSOR hArrowCursor = NULL;
    HCURSOR hTextCursor = NULL;
    HCURSOR hHandCursor = NULL;
    HCURSOR hNoCursor = NULL;
    HCURSOR hDragCtrCursor = NULL;

    struct Data {
        UINT    item;
        UINT    column;
        
        unsigned short  row_height; // chached row height (short?)
        signed char     wheelH;
        signed char     wheelV;
        
        UINT    hot_item;
        UINT    hot_column;
        
        int     anchor_x;
        int     anchor_y;

        UINT    drag_item;
        UINT    drag_column;
        
        unsigned int change;
        
        DWORD   close_t;
        UINT    close_item;
        UINT    close_column;
    };
};

ATOM Shadow::Grid::Initialize (HINSTANCE hInstance) {
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
    wc.lpszClassName = TEXT ("Shadow::Grid");
    
    hArrowCursor = LoadCursor (NULL, IDC_ARROW);
    hTextCursor = LoadCursor (NULL, IDC_IBEAM);
    hHandCursor = LoadCursor (NULL, IDC_HAND);
    hNoCursor = LoadCursor (NULL, IDC_NO);
    
    static const unsigned char cursor [32] = {
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b00000000, 0b00000001,
        0b00000000, 0b00000001,
        0b00000000, 0b00000001,
        0b00000000, 0b00000001,
        0b00000000, 0b00000001,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111000, 0b00111111,
        0b11111111, 0b11111111
    };
    static const unsigned char mask [32] = {
        0b00000111, 0b11000000,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b11111100, 0b01111110,
        0b10000000, 0b00000010,
        0b10000000, 0b00000010,
        0b10000000, 0b00000010,
        0b11111100, 0b01111110,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b00000100, 0b01000000,
        0b00000111, 0b11000000,
        0b00000000, 0b00000000
    };
    
    // TODO: CreateCursorWithShadow
    //  - http://support.microsoft.com/kb/318876/en-us?fr=1
    
    hDragCtrCursor = CreateCursor (hInstance,  7, 7, 16, 16, cursor, mask);
    hHeap = GetProcessHeap ();
    atom = RegisterClass (&wc);
    return atom;
};

HWND Shadow::Grid::Create (HINSTANCE hInstance, HWND hParent,
                           UINT style, UINT id) {
    
    return CreateWindowEx (0, (LPCTSTR) (INT) atom, TEXT(""),
                           style | WS_CHILD | WS_CLIPCHILDREN,
                           0,0,0,0, hParent, (HMENU) id, hInstance, NULL);
};

namespace {

#ifndef HDF_SPLITBUTTON
#define HDF_SPLITBUTTON 0x1000000
#endif
#ifndef HDF_SORTUP
#define HDF_SORTUP      0x0400
#define HDF_SORTDOWN    0x0200
#endif
/*#ifndef HDS_OVERFLOW
#define HDS_OVERFLOW            0x1000
#endif*/

LRESULT OnCreate (HWND, UINT);
LRESULT OnSize (HWND, UINT, USHORT, USHORT);
LRESULT OnPaint (HWND, HDC, RECT);
LRESULT OnClick (HWND, UINT, WPARAM, SHORT, SHORT);
LRESULT OnClickUp (HWND, UINT, WPARAM, SHORT, SHORT);
LRESULT OnHitTest (HWND, SHORT, SHORT);
LRESULT OnMouseMove (HWND, SHORT, SHORT);
LRESULT OnKeyUp (HWND, WPARAM, LPARAM);
LRESULT OnKeyDown (HWND, WPARAM, LPARAM);
LRESULT OnHorizontalWheel (HWND, int, USHORT);
LRESULT OnHorizontalScroll (HWND, UINT, int = 1u);
LRESULT OnVerticalWheel (HWND, int, USHORT);
LRESULT OnVerticalScroll (HWND, UINT, int = 1u);
LRESULT OnNotify (HWND, UINT, const NMHDR *);
LRESULT OnGetDlgCode (HWND, UINT);

bool SetActiveCell (HWND, UINT, UINT item, UINT column);
UINT UpdateHorizontalScrollBar (HWND, UINT max = 0u, UINT page = 0u);
bool SendEditBoxChangeNotify (HWND);
bool ScrollRectToView (HWND, const RECT &);
void UpdateGridControlPos (HWND hWnd);

LRESULT SetSortOrder (HWND, WPARAM, LPARAM);
LRESULT GetSortOrder (HWND, WPARAM, LPARAM);

LRESULT SendBasicNotify (HWND, UINT);
LRESULT CustomDrawNotify (HWND, DWORD, HDC, LPCRECT,
                          UINT = 0, UINT = CDIS_DEFAULT, LPARAM = 0);
LRESULT SendRequestNotify (HWND, UINT request, UINT row, UINT column, UINT index = 0u);
LRESULT SendPrefetchNotify (HWND, UINT, UINT, UINT, UINT);
LRESULT SendChangeNotify (HWND, UINT, UINT, const wchar_t *);
LRESULT SendDragNotify (HWND, UINT, UINT, UINT, UINT);
LRESULT SendMouseNotify (HWND, UINT, DWORD_PTR, DWORD_PTR, const POINT &);
LRESULT SendTrackNotify (HWND, UINT, UINT);
LRESULT SendEnterNotify (HWND, UINT, UINT);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return OnCreate (hWnd, ((CREATESTRUCT *) lParam)->style);

        case WM_SETFONT:
            SendDlgItemMessage (hWnd, 1, message, wParam, lParam);
            SendDlgItemMessage (hWnd, 3, message, wParam, lParam);
            SendDlgItemMessage (hWnd, 4, message, wParam, lParam);

            if (LOWORD (lParam))
                InvalidateRect (hWnd, NULL, TRUE);
            break;
        case WM_GETFONT:
            return SendDlgItemMessage (hWnd, 1, WM_GETFONT, 0, 0);

        case WM_GETDLGCODE:
            return OnGetDlgCode (hWnd, wParam);

        case WM_SIZE:
            return OnSize (hWnd, wParam, LOWORD (lParam), HIWORD (lParam));

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            if (HDC hDC = BeginPaint (hWnd, &ps)) {
                OnPaint (hWnd, hDC, ps.rcPaint);
            };
            EndPaint (hWnd, &ps);
            UpdateHorizontalScrollBar (hWnd);
        } break;

        case WM_PRINTCLIENT:
        {
            RECT rc;
            if (GetClientRect (hWnd, &rc)) {
                OnPaint (hWnd, (HDC) wParam, rc);
            };
        } break;

        case WM_SETFOCUS:
            InvalidateRect (hWnd, NULL, TRUE);
            if (auto hEditBox = GetDlgItem (hWnd, 3)) {
                if ((HWND) wParam != hEditBox) {
                    SetActiveCell (hWnd, WM_SETFOCUS, GetWindowExtra (hWnd, &Data::item), GetWindowExtra (hWnd, &Data::column));
                };
            };
            return SendBasicNotify (hWnd, NM_SETFOCUS);
        case WM_KILLFOCUS:
            InvalidateRect (hWnd, NULL, TRUE);
            return SendBasicNotify (hWnd, NM_KILLFOCUS);
        
        case WM_MOUSEMOVE:
            return OnMouseMove (hWnd, LOWORD (lParam), HIWORD (lParam));
        case WM_MOUSELEAVE:
            return OnMouseMove (hWnd, -1, -1);
        
        case WM_VSCROLL:
            return OnVerticalScroll (hWnd, LOWORD (wParam));
        case WM_HSCROLL:
            return OnHorizontalScroll (hWnd, LOWORD (wParam));
        case WM_MOUSEWHEEL:
            return OnVerticalWheel (hWnd, (SHORT) HIWORD (wParam), LOWORD (wParam));
        case WM_MOUSEHWHEEL:
            return OnHorizontalWheel (hWnd, (SHORT) HIWORD (wParam), LOWORD (wParam));
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SendEditBoxChangeNotify (hWnd);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            return OnClick (hWnd, message, wParam, LOWORD (lParam), HIWORD (lParam));
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            return OnClickUp (hWnd, message, wParam, LOWORD (lParam), HIWORD (lParam));
            
        case WM_KEYDOWN:
            return OnKeyDown (hWnd, wParam, lParam);
        case WM_KEYUP:
            return OnKeyUp (hWnd, wParam, lParam);
        
        case WM_UPDATEUISTATE:
        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        
        case WM_COMMAND:
            if (HIWORD (wParam) > 1) {
                NMHDR nmFake = { (HWND) lParam, LOWORD (wParam), HIWORD (wParam) };
                return OnNotify (hWnd, LOWORD (wParam), &nmFake);
            };
            break;
            
        case WM_NOTIFY:
            return OnNotify (hWnd, wParam, reinterpret_cast <NMHDR *> (lParam));
        
        case Message::Insert: {
            auto count = SendDlgItemMessage (hWnd, 1, HDM_GETITEMCOUNT, 0, 0);
            HDITEM hdi = {
                HDI_FORMAT | HDI_TEXT | HDI_WIDTH | HDI_LPARAM,
                (int) wParam, LPSTR_TEXTCALLBACK, NULL, 0,
                HDF_LEFT | HDF_STRING,// | HDF_SPLITBUTTON,
                lParam, I_IMAGENONE, 0, 0u, NULL
            };
            auto rv = SendDlgItemMessage (hWnd, 1, HDM_INSERTITEM, count, (LPARAM) &hdi);
            UpdateHorizontalScrollBar (hWnd);
            return rv;
        } break;
        
        case Message::SetCurrent: {
            auto columns = SendDlgItemMessage (hWnd, 1, HDM_GETITEMCOUNT, 0, 0);
            auto rows = (unsigned int) SendBasicNotify (hWnd, Request::Rows);
            
            if (wParam >= rows)
                wParam = -1;
            if (lParam >= columns)
                lParam = -1;
            
            SetActiveCell (hWnd, WM_KEYDOWN, wParam, lParam);
        } break;
        
        case Message::SetColumnWidth: {
            HDITEM hdi = {
                HDI_WIDTH, (int) wParam, LPSTR_TEXTCALLBACK,
                NULL, 0, 0, 0, 0, 0, 0u, NULL
            };
            auto rv = SendDlgItemMessage (hWnd, 1, HDM_SETITEM, lParam, (LPARAM) &hdi);
            UpdateHorizontalScrollBar (hWnd);
            return rv;
        } break;
        
/*        case Message::GetColumnIndex: {
            HDITEM item = {
                HDI_LPARAM, (int) wParam, LPSTR_TEXTCALLBACK,
                NULL, 0, 0, 0, 0, 0, 0u, NULL
            };
//            if (SendDlgItemMessage (hWnd, 1, HDM_GETITEM, wParam, (LPARAM) &item)) {
                    
        } break;*/
        
        case Message::SetSortOrder:
            return SetSortOrder (hWnd, wParam, lParam);
        case Message::GetSortOrder:
            return GetSortOrder (hWnd, wParam, lParam);

        default:
            if (message >= HDM_FIRST && message < HDM_FIRST + 0x100)
                return SendDlgItemMessage (hWnd, 1, message, wParam, lParam);

            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT OnCreate (HWND hWnd, UINT style) {
    auto hInstance = (HINSTANCE) GetWindowLongPtr (hWnd, GWLP_HINSTANCE);
    
    SetWindowExtra (hWnd, &Data::item, -1u);
    SetWindowExtra (hWnd, &Data::column, -1u);
    SetWindowExtra (hWnd, &Data::hot_item, -1u);
    SetWindowExtra (hWnd, &Data::hot_column, -1u);
    SetWindowExtra (hWnd, &Data::drag_item, -1u);
    SetWindowExtra (hWnd, &Data::drag_column, -1u);

    SetWindowExtra (hWnd, &Data::close_t, 0u);
    SetWindowExtra (hWnd, &Data::close_item, -1u);
    SetWindowExtra (hWnd, &Data::close_column, -1u);
    
    HWND h =
    Windows::CreateControl (WC_HEADER, WS_VISIBLE |
                            HDS_DRAGDROP | HDS_BUTTONS | HDS_FULLDRAG | HDS_HORZ | HDS_HOTTRACK,
                            hWnd, 1u, hInstance);
    SetProp (h, TEXT ("Shadow::Grid::Subclass"),
             (HANDLE) SetWindowLongPtr (h, GWL_WNDPROC, (DWORD) HeaderSubclass));
    
    h =
    Windows::CreateControl (TEXT ("EDIT"), WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | WS_CLIPSIBLINGS,
                            hWnd, 3u, hInstance);
    SetProp (h, TEXT ("Shadow::Grid::Subclass"),
             (HANDLE) SetWindowLongPtr (h, GWL_WNDPROC, (DWORD) EditorSubclass));

    if (!(style & Style::NoDropDowns)) {
        h =
        Shadow::Grid::Create (hInstance, hWnd, WS_BORDER | WS_CLIPSIBLINGS |
                              WS_VSCROLL | Style::NoDropDowns | Style::NoHeader, 4u);
        SendMessage (h, Shadow::Grid::Message::Insert, 0, 0);
    };

    UpdateHorizontalScrollBar (hWnd);
    SendMessage (hWnd, WM_CHANGEUISTATE, UIS_INITIALIZE, 0u);
    return 0;
};

LRESULT OnSize (HWND hWnd, UINT type, USHORT width, USHORT height) {
    if (type == SIZE_MINIMIZED)
        return 0;
    
    HDWP hdwp = BeginDeferWindowPos (3);

    if (HWND hHeader = GetDlgItem (hWnd, 1)) {
    
        WINDOWPOS wp;
        RECT r = { 0, 0, width, height };
        HDLAYOUT hdly = { &r, &wp };
        
        if (Header_Layout (hHeader, &hdly)) {
            const auto wx = (long) UpdateHorizontalScrollBar (hWnd, 0u, width);
            const auto ox = GetScrollPos (hWnd, SB_HORZ);

            if (GetWindowLongPtr (hWnd, GWL_STYLE) & Style::NoHeader) {
                wp.y = -1;
                wp.cy = 1;
            };
            
            Windows::DeferWindowPos (hdwp, hHeader,
                                     { wp.x - ox, wp.y, wx, wp.cy },
                                     SWP_NOACTIVATE);
        };
    };

    if (hdwp)
        EndDeferWindowPos (hdwp);

    UpdateGridControlPos (hWnd);
    return 0;
};

LRESULT OnGetDlgCode (HWND hWnd, UINT code) {
    const auto style = GetWindowLongPtr (hWnd, GWL_STYLE);

    LRESULT result = DLGC_WANTARROWS;
    if (code == VK_RETURN || code == VK_ESCAPE) {
        result |= DLGC_WANTALLKEYS;
    };
    return result;
};

LRESULT OnNotify (HWND hWnd, UINT id, const NMHDR * nmhdr) {
    switch (id) {
        case 1: // Header
            if (nmhdr->code == NM_CUSTOMDRAW) {

                // forward custom drawing of header?

                const_cast <NMHDR *> (nmhdr)->hwndFrom = hWnd;
                const_cast <NMHDR *> (nmhdr)->idFrom = GetDlgCtrlID (hWnd);
                const_cast <NMHDR *> (nmhdr)->code = Shadow::Grid::Request::HeaderCustomDraw;

                return SendMessage (GetParent (hWnd), WM_NOTIFY,
                                    nmhdr->idFrom, (LPARAM) nmhdr);
            };
            if (nmhdr->code >= HDN_LAST && nmhdr->code <= HDN_FIRST) {

                // forward message to parent
                //  - only from our header, not already forwarded from child
                
                switch (nmhdr->code) {
                    case HDN_ITEMCHANGING:
                        UpdateHorizontalScrollBar (hWnd);
                        // ...
                    case HDN_ENDDRAG:
                        InvalidateRect (hWnd, NULL, FALSE);
                        break;
                };
                    
                if (!(GetWindowLongPtr (hWnd, GWL_STYLE) & Style::NoHeader)) {
                    const_cast <NMHDR *> (nmhdr) ->hwndFrom = hWnd;
                    const_cast <NMHDR *> (nmhdr) ->idFrom = GetDlgCtrlID (hWnd);
                        
                    auto result = SendMessage (GetParent (hWnd), WM_NOTIFY,
                                                nmhdr->idFrom, (LPARAM) nmhdr);
                    switch (nmhdr->code) {
                        case HDN_ENDDRAG:
                        case HDN_ITEMCHANGING:
                            SetFocus (hWnd); // simply dismiss editor
                            break;
                    };
                    return result;
                };
            };
            break;
        
        case 3: // Edit box
            switch (nmhdr->code) {
                case EN_CHANGE:
                    SetWindowExtra (hWnd, &Data::change,
                                    GetWindowExtra (hWnd, &Data::change) + 1u);
                    if (GetWindowLong (hWnd, GWL_STYLE) & Shadow::Grid::Style::LiveUpdate) {
                        InvalidateRect (hWnd, NULL, TRUE); // TODO: only current cell?
                        SendEditBoxChangeNotify (hWnd);
                    };
                    break;
                case EN_KILLFOCUS:
                    InvalidateRect (hWnd, NULL, TRUE); // TODO: only current cell?
                    SendEditBoxChangeNotify (hWnd);
                    ShowWindow (nmhdr->hwndFrom, SW_HIDE);
                    break;
            };
            break;
        
        case 4: // Another Grid posing as combobox
            switch (nmhdr->code) {
                case NM_KILLFOCUS:
                    SetWindowExtra (hWnd, &Data::close_t, GetTickCount ());
                    SetWindowExtra (hWnd, &Data::close_item, GetWindowExtra (hWnd, &Data::item));
                    SetWindowExtra (hWnd, &Data::close_column, GetWindowExtra (hWnd, &Data::column));
                    ShowWindow (nmhdr->hwndFrom, SW_HIDE);
                    break;

                case NM_KEYDOWN:
                    if (reinterpret_cast <const NMKEY *> (nmhdr)->nVKey == VK_ESCAPE) {
                        ShowWindow (nmhdr->hwndFrom, SW_HIDE);
                        SetFocus (hWnd);
                    };
                    break;
                    
                case Request::Rows:
                    return SendRequestNotify (hWnd, Request::Depth,
                                              GetWindowExtra (hWnd, &Data::item),
                                              GetWindowExtra (hWnd, &Data::column));
                case Request::Type:
                    switch ((CellType) SendRequestNotify (hWnd, Request::Type,
                                                          GetWindowExtra (hWnd, &Data::item),
                                                          GetWindowExtra (hWnd, &Data::column))) {
                        case CellType::Selection:
                            return (LRESULT) CellType::MenuItem;
                        case CellType::MultipleSelection:
                            return (LRESULT) CellType::Checkbox;
                        default:
                            return (LRESULT) CellType::Grayed;
                    };
                
                case Request::Value:
                    return SendRequestNotify (hWnd, Request::Option,
                                              GetWindowExtra (hWnd, &Data::item),
                                              GetWindowExtra (hWnd, &Data::column),
                                              reinterpret_cast <const NMRequest *> (nmhdr) ->row);
                
                case Request::Change:
                    auto item = GetWindowExtra (hWnd, &Data::item);
                    auto column = GetWindowExtra (hWnd, &Data::column);
                    
                    switch ((CellType) SendRequestNotify (hWnd, Request::Type,
                                                          item, column)) {
                        case CellType::Selection:
                            ShowWindow (nmhdr->hwndFrom, SW_HIDE);
                            return SendChangeNotify (hWnd, item, column,
                                                     reinterpret_cast <const NMChange *> (nmhdr) ->value);
                            
                        case CellType::MultipleSelection: {
                            auto response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * 24);
                            if (response) {
                                _snwprintf (response, 24, L"%c%u",
                                            reinterpret_cast <const NMChange *> (nmhdr) ->value[0] == L'0' ? L'-' : L'+',
                                            reinterpret_cast <const NMChange *> (nmhdr) ->row);
                                if (!SendChangeNotify (hWnd, item, column, response)) {
                                    HeapFree (hHeap, 0, response);
                                };
                            };
                        } break;
                            
                        default:
                            break;
                    };
                    break;
            };
            break;
    };
    
    return 0;
};

UINT GetFontHeight (HDC hDC) {
    TEXTMETRIC textMetrics;
    if (GetTextMetrics (hDC, &textMetrics)) {
        return textMetrics.tmHeight;
    } else
        return 0u;
};
SIZE GetControlSize (HWND hWnd) {
    RECT r;
    if (GetClientRect (hWnd, &r)) {
        return { r.right, r.bottom };
    } else
        return { 0, 0 };
};
UINT GetHeaderHeight (HWND hWnd) {
    if (GetWindowLongPtr (hWnd, GWL_STYLE) & Style::NoHeader) {
        return 0;
    } else {
        return GetControlSize (GetDlgItem (hWnd, 1)) .cy;
    };
};

int GetScrollTrack (HWND hWnd, int fnBar) {
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_TRACKPOS;

    if (GetScrollInfo (hWnd, fnBar, &si)) {
        return si.nTrackPos;
    } else
        return 0;
};

std::pair <unsigned int, unsigned int> FindHeaderEndItems (HWND hHeader,
                                                           UINT x, UINT page) {
    unsigned int first = -1u;
    unsigned int last = 0u;
    
    UINT count = Header_GetItemCount (hHeader);
    if (count != -1u) {
        for (UINT i = 0u; i != count; ++i) {
            RECT r;
            if (Header_GetItemRect (hHeader, i, &r)) {
                if (((unsigned int) r.right > x) && ((unsigned int) r.left < x + page)) {
                    if (i < first) first = i;
                    if (i > last) last = i;
                };
            };
        };
    };
    
    return std::make_pair (first, last);
};
std::pair <unsigned int, unsigned int> FindCell (HWND hWnd, UINT x, UINT y,
                                                 UINT * flags = NULL) {
    unsigned int item = -1u;
    unsigned int column = -1u;

    if (HWND hHeader = GetDlgItem (hWnd, 1)) {
        
        auto yscroll = GetScrollPos (hWnd, SB_VERT);
        auto xscroll = GetScrollPos (hWnd, SB_HORZ);

        typedef struct _HD_HITTESTINFO {
            POINT pt;
            UINT flags;
            int iItem;
        } HDHITTESTINFO;

        // column from header
        
        HDHITTESTINFO hittest = { { int (x) + xscroll, 0 }, 0, 0 };
        LRESULT result = SendMessage (hHeader, HDM_HITTEST, 0, (LPARAM) &hittest);
        if (result != -1) {
            if (flags) {
                *flags = hittest.flags;
                column = result;
            } else {
                if (!(hittest.flags & (HHT_NOWHERE | HHT_ONDIVIDER))) {
                    column = result;
                };
            };
        };
        
        // row
        
        if (auto height = GetWindowExtra (hWnd, &Data::row_height)) {
            item = (y + yscroll - GetHeaderHeight (hWnd)) / height;
            if (item >= (unsigned int) SendBasicNotify (hWnd, Request::Rows)) {
                item = -1u;
            };
        };
    };
    return std::make_pair (item, column);
};

UINT DrawCellText (HDC hDC, const wchar_t * string, RECT r, UINT flags = 0) {
    UINT result = 0;
    auto length = std::wcslen (string);
    auto buffer = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * (length + 4));
    if (buffer) {
        _snwprintf (buffer, length + 1, L"%s", string);
        result = DrawText (hDC, buffer, -1, &r, DT_SINGLELINE | DT_VCENTER |
                           DT_END_ELLIPSIS | DT_MODIFYSTRING | DT_NOPREFIX | flags);
        HeapFree (hHeap, 0, buffer);
    };
    return result;
};

void DrawCheckBox (HANDLE hTheme, HDC hDC,
                   LRESULT value, const RECT & r, UINT state) {
    
    // determine value
    //  - if not small short integer, consider it to be string
    //  - after the integer can follow string
    
    const wchar_t * string = NULL;
    
    if ((value < -1) || (value > 0xFFFF)) {
        wchar_t * tailptr = nullptr;
        value = std::wcstol ((const wchar_t *) value, &tailptr, 0);
        
        if (tailptr) {
            if (std::iswspace (*tailptr)) {
                ++tailptr;
            };
            if (*tailptr != L'\0') {
                string = tailptr;
            };
        };
    };
    
    // determine checkbox rect
    
    RECT rBox = r;
    if (string) {
        rBox.right = rBox.left + (rBox.bottom - rBox.top);
    };
    
    // draw
    
    if (hTheme) {
        UINT draw = (value & 0xFu) * 4u + 1u;
        
        if (value == -1) {
            draw = 4; // unchecked disabled
        } else
        if (value & 0x10u) {
            draw += 3;
        } else
        if (state & CDIS_SELECTED) {
            draw += 2;
        } else
        if (state & CDIS_HOT) {
            draw += 1;
        } else
        if (value & 0x20) {
            draw = 4;
        };
        
        UxTheme::DrawBackground (hTheme, hDC, 3, draw, &rBox);
    } else {
        rBox.top += 3; // TODO: padding as settings?
        rBox.bottom -= 3;
        
        UINT draw = DFCS_BUTTONCHECK;
        if ((value > 0) && (value % 2) == 1) {
            draw |= DFCS_CHECKED;
        };
        if ((value == -1) || (value & 0x10)) {
            draw |= DFCS_INACTIVE;
        } else
        if (state & CDIS_SELECTED) {
            draw |= DFCS_PUSHED;
        } else
        if (state & CDIS_HOT) {
            draw |= DFCS_HOT;
        };
        
        DrawFrameControl (hDC, &rBox, DFC_BUTTON, draw);
        
        if ((value & 0xF) == 2) {
            long dimm = rBox.bottom - rBox.top - 2 * GetSystemMetrics (SM_CXEDGE) - 4;
            long left = rBox.left + ((rBox.right - rBox.left - dimm) / 2);
            long top = rBox.top + ((rBox.bottom - rBox.top - dimm) / 2);
            RECT rQuad = { left, top, left + dimm, top + dimm };
            
            if (draw & DFCS_INACTIVE) {
                FillRect (hDC, &rQuad, GetSysColorBrush (COLOR_3DSHADOW)); // as w2k
            } else
            if (draw & DFCS_HOT) {
                FillRect (hDC, &rQuad, GetSysColorBrush (COLOR_HOTLIGHT));
            } else {
                FillRect (hDC, &rQuad, GetSysColorBrush (COLOR_WINDOWTEXT));
            };
        };
    };
    
    // draw text

    if (string) {
        DrawCellText (hDC, string,
                      { rBox.right, r.top, r.right - 6, r.bottom });
    };
    return;
};

void DrawComboBox (HANDLE hTheme, HANDLE hOldBitmap, HDC hDC,
                   const wchar_t * string, RECT r, UINT state) {
    
    RECT rBox = { r.right - (r.bottom - r.top),
                  r.top, r.right, r.bottom };
    
    if (hTheme) {
        UINT draw = 1;
        if (state & CDIS_SELECTED) {
            draw += 2;
        } else
        if (state & (CDIS_HOT | CDIS_FOCUS)) {
            draw += 1;
        };
        
        UxTheme::DrawBackground (hTheme, hDC, 5, draw, &r);
        
        if (Windows::Version >= Windows::Version::WindowsVista) {
            RECT rClip = rBox;
            rClip.left += 3;
            UxTheme::DrawBackground (hTheme, hDC, 6, draw, &rBox, &rClip);
        } else {
            rBox.top += 1;
            rBox.right -= 1;
            rBox.bottom -= 1;
            UxTheme::DrawBackground (hTheme, hDC, 1, draw, &rBox);
        };
    } else {
        int o = 0;
        UINT draw = 0;
        if (state & CDIS_SELECTED) {
            draw |= DFCS_PUSHED;
            o = 1;
        } else
        if (state & (CDIS_HOT | CDIS_FOCUS)) { // TODO: CDIS_FOCUS -> DrawFocusRect
            draw |= DFCS_HOT;
        };

        r.right += 1;
        DrawFrameControl (hDC, &r, DFC_BUTTON, DFCS_BUTTONPUSH | draw);
        
        BITMAP bm;
        if (GetObject (hOldBitmap, sizeof bm, &bm)) {
            if (HDC hDCmem = CreateCompatibleDC (hDC)) {
                auto hBmOld = SelectObject (hDCmem, hOldBitmap);
                
                BitBlt (hDC,
                        rBox.left + ((rBox.right - rBox.left) - bm.bmWidth) / 2 + o,
                        rBox.top + ((rBox.bottom - rBox.top) - bm.bmHeight) / 2 + o,
                        bm.bmWidth, bm.bmHeight, hDCmem, 0, 0, SRCCOPY);

                if (hBmOld) {
                    SelectObject (hDCmem, hBmOld);
                };
                DeleteDC (hDCmem);        
            };
        };
    };
    
    if (string) {
        DrawCellText (hDC, string, { r.left + 6, r.top, rBox.left + 2, r.bottom });
    };
    return;
};

LRESULT OnPaint (HWND hWnd, HDC _hDC, RECT rcInvalid) {
    HWND hHeader = GetDlgItem (hWnd, 1);

    RECT rc;
    if (GetClientRect (hWnd, &rc)) {
        rc.top = GetHeaderHeight (hWnd);
    } else
        return 0;

    HDC hDC = NULL;
    HANDLE hBuffered = /*NULL; // */ UxTheme::BeginBufferedPaint (_hDC, &rc, &hDC);
    
    if (!hBuffered)
        hDC = _hDC;

    HGDIOBJ hPrevious [] = {
        SelectObject (hDC, (HGDIOBJ) SendDlgItemMessage (hWnd, 1, WM_GETFONT, 0, 0)),
        SelectObject (hDC, GetStockObject (NULL_BRUSH)),
        SelectObject (hDC, GetStockObject (DC_PEN))
    };
    
    SetBkMode (hDC, TRANSPARENT);
    SetDCPenColor (hDC, GetSysColor (COLOR_3DFACE));
    
    DWORD rvPrePaint = CustomDrawNotify (hWnd, CDDS_PREPAINT, hDC, &rc);
    if (!(rvPrePaint & CDRF_SKIPDEFAULT)) {
        
        long maxx = 0u;
        long maxy = 0u;
        
        // set default text color
        //  - and allow parent to change it
        
        SetTextColor (hDC, GetSysColor (COLOR_WINDOWTEXT));
        HBRUSH hListBoxBrush = (HBRUSH) SendMessage (GetParent (hWnd), WM_CTLCOLORLISTBOX,
                                                     (WPARAM) hDC, (LPARAM) hWnd);
        COLORREF clrDefault = GetTextColor (hDC);

        if (rvPrePaint & CDRF_NOTIFYPOSTERASE) {
            maxx = -1u;
            maxy = -1u;
            FillRect (hDC, &rc, hListBoxBrush);
            CustomDrawNotify (hWnd, CDDS_POSTERASE, hDC, &rc);
        };

        // update vertical scrollbar

        const unsigned int rows = SendBasicNotify (hWnd, Request::Rows);
        const unsigned int height = GetFontHeight (hDC) + 2 * 3 + 1;

        {   SCROLLINFO si = {
                sizeof si, SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL,
                0, (int) (height * rows),
                (rc.bottom < rc.top) ? 0u : (rc.bottom - rc.top), 0, 0
            };
            SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
        };
        
        const int xoffset = GetScrollPos (hWnd, SB_HORZ);
        const int yoffset = GetScrollPos (hWnd, SB_VERT);
        
        SetWindowExtra (hWnd, &Data::row_height, (unsigned short) height);
        
        const unsigned int rowA = yoffset / height;
        const unsigned int rowZ = (rc.bottom < rc.top) ? rowA
                                : std::min (rowA + (rc.bottom - rc.top) / height + 1u,
                                            rows - 1uL);
        const auto colbounds = FindHeaderEndItems (hHeader, xoffset, rc.right);
        const unsigned int colA = colbounds.first;
        const unsigned int colZ = colbounds.second;
        
        SendPrefetchNotify (hWnd, rowA, rowZ, colA, colZ);
        
        const unsigned int hot_item = GetWindowExtra (hWnd, &Data::hot_item);
        const unsigned int hot_column = GetWindowExtra (hWnd, &Data::hot_column);
        const unsigned int current_item = GetWindowExtra (hWnd, &Data::item);
        const unsigned int current_column = GetWindowExtra (hWnd, &Data::column);
        const unsigned int drag_item = GetWindowExtra (hWnd, &Data::drag_item);
        const unsigned int drag_column = GetWindowExtra (hWnd, &Data::drag_column);
        
        // themes
        
        HANDLE hButtonTheme = UxTheme::Open (hWnd, L"BUTTON");
        HANDLE hComboboxTheme = UxTheme::Open (hWnd, L"COMBOBOX");
        HANDLE hOldBitmap = NULL;
        
        if (!hComboboxTheme) {
//            height
            hOldBitmap = LoadImage (NULL, MAKEINTRESOURCE (/*OBM_COMBO*/32738), IMAGE_BITMAP,
                                    0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED);
        };
        
        // display data
        //  - start by iterating columns, only processing those visible
        
        unsigned int n = 0u;
        maxy = rc.top + height * rows - yoffset;

        for (auto column = colA; column != colZ + 1u; ++column) {
            RECT rColumn;
            if (Header_GetItemRect (hHeader, column, &rColumn)) {
                if (maxx < rColumn.right) {
                    maxx = rColumn.right;
                };
                
                if ((rColumn.right > xoffset) && (rColumn.left < xoffset + rc.right)) {
                    
                    // for all rows in view
                    
                    for (auto row = rowA; row != rowZ + 1u; ++row) {

                        RECT rTemp;
                        RECT rCell = {
                            rColumn.left - xoffset,
                            rc.top + int (height * row) - yoffset,
                            rColumn.right - xoffset,
                            rc.top + int (height * row) - yoffset + int (height),
                        };

                        // for all rows in invalidated area

                        if (IntersectRect (&rTemp, &rCell, &rcInvalid)) {
                            --rCell.right;
                            --rCell.bottom;
                            
                            auto type = (CellType) SendRequestNotify (hWnd, Request::Type, row, column, 0);
                            UINT state = 0;
                            
                            // prepare state
    
                            if (row == current_item && column == current_column) {
                                state |= CDIS_FOCUS;
                                if (GetKeyState (VK_SPACE) & 0x8000) {
                                    state |= CDIS_SELECTED;
                                };
                                if (GetKeyState (VK_RETURN) & 0x8000) {
                                    state |= CDIS_SELECTED;
                                };
                                if (type == CellType::Selection || type == CellType::MultipleSelection) {
                                    if (IsWindowVisible (GetDlgItem (hWnd, 4))) {
                                        state |= CDIS_SELECTED;
                                    };
                                };
                            };
                            if (row == hot_item && column == hot_column) {
                                state |= CDIS_HOT;
                                if (GetKeyState (VK_LBUTTON) & 0x8000) {
                                    state |= CDIS_SELECTED;
                                };
                            };
                            if (row == drag_item && column == drag_column) {
                                state |= CDIS_MARKED;
                            };
                            if (hot_column == drag_column && column == hot_column) {
                                if (GetKeyState (VK_SHIFT) & 0x8000) {
                                    if (row > drag_item && row < hot_item) state |= CDIS_SELECTED;
                                    if (row > hot_item && row < drag_item) state |= CDIS_SELECTED;
                                };
                            };
                            
                            if (type == CellType::Inactive) {
                                state = CDIS_DISABLED;
                            };
                            if (type == CellType::Grayed) {
                                state = CDIS_GRAYED | CDIS_DISABLED;
                            };

                            // border

                            POINT border [] = {
                                { rCell.left, rCell.bottom },
                                { rCell.right, rCell.bottom },
                                { rCell.right, rCell.top - 1 }
                            };
                            Polyline (hDC, border, sizeof border / sizeof border [0]);
                            
                            // initialize drawing colors
                            
                            switch (type) {
                                case CellType::Grayed:
                                    SelectObject (hDC, GetSysColorBrush (COLOR_3DFACE));
                                    SetTextColor (hDC, GetSysColor (COLOR_GRAYTEXT));
                                    break;
                                    
                                case CellType::Inactive:
                                    SelectObject (hDC, hListBoxBrush),
                                    SetTextColor (hDC, GetSysColor (COLOR_GRAYTEXT));
                                    break;
                                
                                case CellType::MenuItem:
                                    if (state & (CDIS_HOT/* | CDIS_FOCUS*/)) {
                                        SelectObject (hDC, GetSysColorBrush (COLOR_HIGHLIGHT));
                                        SetTextColor (hDC, GetSysColor (COLOR_HIGHLIGHTTEXT));
                                    } else {
                                        SelectObject (hDC, GetSysColorBrush (COLOR_WINDOW/*COLOR_MENU*/));
                                        SetTextColor (hDC, GetSysColor (COLOR_MENUTEXT));
                                    };
                                    break;
                                
                                default:
                                    SelectObject (hDC, hListBoxBrush),
                                    SetTextColor (hDC, clrDefault);
                                    break;
                            };
                            
                            // request confirmation to draw from parent window
                            
                            DWORD rvPreItemPaint = 0;
                            if (rvPrePaint & CDRF_NOTIFYITEMDRAW) {
                                rvPreItemPaint = CustomDrawNotify (hWnd, CDDS_ITEMPREPAINT, hDC, &rCell,
                                                                   MAKELPARAM (column, row), state, (LPARAM) type);
                                if (rvPreItemPaint & 0x00400000) {
                                    state |= CDIS_HOT;
                                };
                            };
                            if (!(rvPreItemPaint & CDRF_SKIPDEFAULT)) {
                                RECT r = rCell;
                                auto text = SendRequestNotify (hWnd, Request::Value, row, column, 0);
                                
                                FillRect (hDC, &rCell, (HBRUSH) GetCurrentObject (hDC, OBJ_BRUSH));
                                
                                switch (type) {
                                    case CellType::Selection:
                                    case CellType::MultipleSelection:
                                        DrawComboBox (hComboboxTheme, hOldBitmap, hDC,
                                                      reinterpret_cast <const wchar_t *> (text),
                                                      r, state);
                                        break;

                                    case CellType::Inactive:
                                    case CellType::Grayed:
                                    case CellType::Text:
                                    case CellType::MenuItem:
                                        
//                                    case CellType::RealNumber: // DT_RIGHT
//                                    case CellType::DecimalNumber: // DT_RIGHT
//                                    case CellType::MonetaryValue: // DT_RIGHT
                                        
                                        if (text) {
                                            r.left += 6; // TODO: padding as settings?
                                            r.right -= 6;
                                            
                                            DrawCellText (hDC, reinterpret_cast <const wchar_t *> (text), r);
                                        };
                                        break;
                                    
                                    case CellType::Checkbox:
                                        DrawCheckBox (hButtonTheme, hDC, text, r, state);
                                        // GetThemeTransitionDuration // for XP return 0
                                        break;
                                };
                            };

                            // post paint notifications
                            
                            DWORD rvPostItemPaint = 0;
                            if (rvPreItemPaint & CDRF_NOTIFYPOSTPAINT) {
                                rvPostItemPaint = CustomDrawNotify (hWnd, CDDS_ITEMPOSTPAINT, hDC, &rCell,
                                                                    MAKELPARAM (column, row), state, (LPARAM) type);
                            };
                            
                            if (!(rvPreItemPaint & CDRF_SKIPPOSTPAINT) && !(rvPostItemPaint & CDRF_SKIPPOSTPAINT)) {
                                switch (type) {
                                    case CellType::Checkbox:
                                        if (!(rvPrePaint & CDRF_SKIPPOSTPAINT)) { 
                                            if (GetFocus () == hWnd && (state & CDIS_FOCUS)) {
                                                RECT rFocus = rCell;
                                                rFocus.left   += 1;
                                                rFocus.top    += 1;
                                                rFocus.right  -= 2;
                                                rFocus.bottom -= 1;
                                                
                                                if (!(LOWORD (SendMessage (hWnd, WM_QUERYUISTATE, 0,0))
                                                                                    & UISF_HIDEFOCUS)) {
                                                    DrawFocusRect (hDC, &rFocus);
                                                };
                                            };
                                        };
                                        break;
                                        
                                    case CellType::MenuItem:
                                    case CellType::Selection:
                                    case CellType::MultipleSelection:
                                        
                                        break;
                                    default:
                                        if (GetFocus () == hWnd
                                                || GetFocus () == GetDlgItem (hWnd, 3)
                                                || GetFocus () == GetDlgItem (hWnd, 4)) {
                                            UINT mask = CDIS_SELECTED | CDIS_FOCUS | CDIS_MARKED;
                                            if (GetWindowLongPtr (hWnd, GWL_STYLE) & Style::HotTrack) {
                                                mask |= CDIS_HOT;
                                            };
                                        
                                            if (state & mask) {
                                                POINT frame [] = {
                                                    { rCell.left,  rCell.bottom },
                                                    { rCell.right, rCell.bottom },
                                                    { rCell.right, rCell.top  },
                                                    { rCell.left,  rCell.top  },
                                                    { rCell.left,  rCell.bottom }
                                                };
                                            
                                                COLORREF color = 0;
                                                if (state & CDIS_FOCUS) {
                                                    // do not paint for checkbox
                                                    color = GetSysColor (COLOR_WINDOWFRAME);
                                                } else
                                                if (state & CDIS_SELECTED) {
                                                    color = GetSysColor (COLOR_HOTLIGHT);
                                                } else
                                                if (state & CDIS_HOT) {
                                                    // TODO: mix hotlight with 3dface?
                                                    color = GetSysColor (COLOR_HIGHLIGHT);
                                                } else
                                                if (state & CDIS_MARKED) {
                                                    color = 0x00007F00;
                                                };
                                            
                                                COLORREF prev = SetDCPenColor (hDC, color);
                                                Polyline (hDC, frame, sizeof frame / sizeof frame [0]);
                                                SetDCPenColor (hDC, prev);
                                            };
                                        };
                                        break;
                                };
                            };
    
                            ++n;
                        };
                    };
                };
            };
        };

        if (hOldBitmap) {
            DeleteObject (hOldBitmap);
        };
        
        if (hButtonTheme) {
            UxTheme::Close (hButtonTheme);
        };
        if (hComboboxTheme) {
            UxTheme::Close (hComboboxTheme);
        };
        
        // Fill right and bottom border (if any)
        
        if (maxx < rc.right) {
            RECT rRightWhite = { maxx, rc.top, rc.right, rc.bottom };
            FillRect (hDC, &rRightWhite, hListBoxBrush);
        };
        if (maxy < rc.bottom) {
            RECT rBottomWhite = { rc.left, maxy, maxx, rc.bottom };
            FillRect (hDC, &rBottomWhite, hListBoxBrush);
        };
    };
    
    if (rvPrePaint & CDRF_NOTIFYPOSTPAINT) {
        CustomDrawNotify (hWnd, CDDS_POSTPAINT, hDC, &rc);
    };
    
    for (auto i = 0u; i != sizeof hPrevious / sizeof hPrevious [0]; ++i) {
        if (hPrevious [i])
            SelectObject (hDC, hPrevious [i]);
    };
    
    if (hBuffered)
        UxTheme::EndBufferedPaint (hBuffered);
    
    return 0;
};

RECT FindCellRect (HWND hWnd, UINT item, UINT column) {
    RECT r = { -1, -1, -1, -1 };
    if (item != -1u && column != -1u) {
        if (HWND hHeader = GetDlgItem (hWnd, 1)) {
            if (Header_GetItemRect (hHeader, column, &r)) {
            
                auto xoff = GetScrollPos (hWnd, SB_HORZ);
                auto height = GetWindowExtra (hWnd, &Data::row_height);
                
                r.top = GetHeaderHeight (hWnd) + (item * height) - GetScrollPos (hWnd, SB_VERT);
                r.left -= xoff;
                r.right -= xoff;
                r.bottom = r.top + height;
            };
        };
    };
    return r;
};

LRESULT ActionOnCell (HWND hWnd, UINT message, WPARAM wParam, SHORT x, SHORT y,
                      UINT row, UINT column, const RECT & r) {
    auto type = (CellType) SendRequestNotify (hWnd, Request::Type, row, column);
    
    switch (type) {
        case CellType::Inactive:
        case CellType::Grayed:
            return false;
        
        case CellType::Selection:
        case CellType::MultipleSelection:
            switch (message) {
                case WM_KEYUP:
                case WM_KEYDOWN:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK: {
                    if (HWND h = GetDlgItem (hWnd, 4u)) {
                        
                        switch (message) {
                            case WM_LBUTTONDOWN:
                            case WM_LBUTTONDBLCLK: {
                                
                                if (row == GetWindowExtra (hWnd, &Data::close_item))
                                    if (column == GetWindowExtra (hWnd, &Data::close_column))
                                        if (GetTickCount () - GetWindowExtra (hWnd, &Data::close_t) < 30u)
                                            return false;
                            } break;
                        };
                        
                        UINT overflow = 4u;
                        UINT wndheight = GetControlSize (hWnd) .cy;
                        UINT hdrheight = GetHeaderHeight (hWnd);
                        
                        {   HDITEM item;
                            item.mask = HDI_WIDTH;
                            item.cxy = 0;
                            SendMessage (h, HDM_SETITEM, 0, (LPARAM) &item);
                            item.cxy = r.right - r.left - GetSystemMetrics (SM_CXVSCROLL) + 2 * overflow - 2;
                            SendMessage (h, HDM_SETITEM, 0, (LPARAM) &item);
                        };
                        
                        UINT height = (wndheight - 2 * hdrheight) / 2u;
                        UINT full = GetWindowExtra (hWnd, &Data::row_height)
                                  * SendRequestNotify (hWnd, Request::Depth, row, column);
                        if (height > full)
                            height = full;
                        
                        height += 3; // borders
                        SendMessage (h, Message::SetCurrent, -1, -1); // TODO: find current option, if any
                        
                        int x;
                        int y;

                        if (r.bottom - overflow + height > wndheight) {
                            x = r.left - overflow;
                            y = r.top + overflow - height;
                        } else {
                            x = r.left - overflow;
                            y = r.bottom - overflow;
                        };

                        SetWindowPos (h, HWND_TOP, x, y,
                                      r.right - r.left + 2 * overflow, height,
                                      SWP_SHOWWINDOW);
                        
                        x += GetScrollPos (hWnd, SB_HORZ);
                        y += GetScrollPos (hWnd, SB_VERT);
                        
                        SetWindowExtra (hWnd, &Data::anchor_x, x);
                        SetWindowExtra (hWnd, &Data::anchor_y, y);
                        SetFocus (h);
                    };
                } break;
            };
            break;

        case CellType::MenuItem:
            switch (message) {
                case WM_KEYUP:
                case WM_KEYDOWN:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK: {
                    auto response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * 16);
                    if (response) {
                        _snwprintf (response, 16, L"%u", row);
                        if (!SendChangeNotify (hWnd, row, column, response)) {
                            HeapFree (hHeap, 0, response);
                        };
                    };
                } break;
            };
            break;

        case CellType::Text:
            switch (message) {
                case WM_KEYDOWN:
                    SendEditBoxChangeNotify (hWnd);

                case WM_SETFOCUS:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                    if (HWND h = GetDlgItem (hWnd, 3u)) {

                        if (GetWindowLong (hWnd, GWL_STYLE) & Shadow::Grid::Style::LiveUpdate) {
                            SetWindowExtra (hWnd, &Data::change, -1u);
                        } else {
                            SetWindowExtra (hWnd, &Data::change, 0u);
                        };

                        if (auto value = (const wchar_t *) SendRequestNotify (hWnd, Request::Value, row, column)) {
                            SetWindowText (h, value);
                        } else {
                            SetWindowText (h, L"");
                        };
                        
                        SendMessage (h, EM_SETLIMITTEXT, SendRequestNotify (hWnd, Request::Limit, row, column), 0);
                        SetWindowPos (h, HWND_TOP, r.left + 3, r.top + 3, r.right - r.left - 6, r.bottom - r.top - 6, SWP_SHOWWINDOW);

                        SetWindowExtra (hWnd, &Data::anchor_x, (int) (r.left + 3 + GetScrollPos (hWnd, SB_HORZ)));
                        SetWindowExtra (hWnd, &Data::anchor_y, (int) (r.top + 3 + GetScrollPos (hWnd, SB_VERT)));
                        
                        switch (message) {
                            case WM_KEYDOWN:
                            case WM_SETFOCUS:
                                SendMessage (h, EM_SETSEL, 0, -1);
                                break;
                            case WM_LBUTTONDOWN:
                            case WM_RBUTTONDOWN:
                                SendMessage (h, message, wParam,
                                             MAKELPARAM ((x - r.left), (y - r.top)));
                                break;
                            case WM_LBUTTONDBLCLK:
                                SendMessage (h, WM_LBUTTONDOWN, wParam,
                                             MAKELPARAM ((x - r.left), (y - r.top)));
                                break;
                            case WM_RBUTTONDBLCLK:
                                SendMessage (h, WM_RBUTTONDOWN, wParam,
                                             MAKELPARAM ((x - r.left), (y - r.top)));
                                break;
                        };
                    };
                    break;
            };
            break;
        
        case CellType::Checkbox:
            switch (message) {
                case WM_KEYUP:
                case WM_KEYDOWN:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                    auto value = (unsigned int) SendRequestNotify (hWnd, Request::Value, row, column);
                    if (value != -1u && value > 0xFFFF) {
                        value = std::wcstol ((const wchar_t *) value, nullptr, 0);
                    };
                    if (value & 0x20) {
                        value &= ~0x20;
                    };
                    
                    switch (value) {
                        case 0u: value = 1u; break; // unchecked -> checked
                        case 1u: value = 0u; break; // checked -> unchecked
                        case 2u: value = 1u; break; // mixed -> checked
                        case 3u: value = 0u; break; // implicit -> unchecked
                        case 4u: value = 1u; break; // excluded -> checked
                        default:
                            return 0; // disabled checkbox -> do nothing
                    };
                    
                    auto response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * 4);
                    if (response) {
                        _snwprintf (response, 4, L"%u", value);
                        if (!SendChangeNotify (hWnd, row, column, response)) {
                            
                            // freeing only when returning zero
                            //  - non-zero return means "moving from"
                            
                            HeapFree (hHeap, 0, response);
                        };
                    };
                    return true;
            };
            break;
    };
    return true;
};
bool SendEditBoxChangeNotify (HWND hWnd) {
    if (GetWindowExtra (hWnd, &Data::change) == 0u)
        return false;
    
    SetWindowExtra (hWnd, &Data::change, 0u);
    
    if (HWND hEdit = GetDlgItem (hWnd, 3)) {
        
        const unsigned int item = GetWindowExtra (hWnd, &Data::item);
        const unsigned int column = GetWindowExtra (hWnd, &Data::column);
        
        if (item != -1u && column != -1u) {
            auto length = GetWindowTextLength (hEdit);
            auto buffer = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * (length + 1));
            if (buffer) {
                if (!GetWindowText (hEdit, buffer, length + 1)) {
                    buffer [0] = 0;
                };
    
                if (!SendChangeNotify (hWnd,
                                       GetWindowExtra (hWnd, &Data::item),
                                       GetWindowExtra (hWnd, &Data::column),
                                       buffer)) {
                    
                    // freeing only when returning zero
                    //  - non-zero return means "moving from"
                    
                    HeapFree (hHeap, 0, buffer);
                } else
                    return true;
            };
        };
    };
    
    return false;
};


LRESULT OnClick (HWND hWnd, UINT message, WPARAM wParam, SHORT x, SHORT y) {
    
    auto cell = FindCell (hWnd, x, y);
    auto rect = FindCellRect (hWnd, cell.first, cell.second);
    if (!IsRectEmpty (&rect)) {

        int code;
        switch (message) {
            default:
            case WM_LBUTTONDOWN:    code = NM_CLICK; break;
            case WM_LBUTTONDBLCLK:  code = NM_DBLCLK; break;
            case WM_RBUTTONDOWN:    code = NM_RCLICK; break;
            case WM_RBUTTONDBLCLK:  code = NM_RDBLCLK; break;
        };

        if (!SendMouseNotify (hWnd, code, cell.first, cell.second, { x, y })) {
        
            if ((message == WM_LBUTTONDOWN) && (wParam & MK_CONTROL)) {
                // TODO: validate type?
                SetWindowExtra (hWnd, &Data::drag_item, cell.first);
                SetWindowExtra (hWnd, &Data::drag_column, cell.second);
                InvalidateRect (hWnd, &rect, FALSE);
                
            } else {
                
                // TODO: Set focus for not-inactive and not-text cells
                //SetFocus (hWnd);

                if (ActionOnCell (hWnd, message, wParam, x, y, cell.first, cell.second, rect)) {
                    
                    auto previtem = SetWindowExtra (hWnd, &Data::item, cell.first);
                    auto prevcolumn = SetWindowExtra (hWnd, &Data::column, cell.second);
                
                    auto prev = FindCellRect (hWnd, previtem, prevcolumn);
                    InvalidateRect (hWnd, &prev, FALSE);
                    
                    SendEnterNotify (hWnd, cell.first, cell.second);
                };
                
                if (!ScrollRectToView (hWnd, rect)) {
                    InvalidateRect (hWnd, &rect, FALSE);
                };
            };
        };
    };
    return 0;
};

LRESULT OnClickUp (HWND hWnd, UINT message, WPARAM wParam, SHORT x, SHORT y) {

    // invalidate cell under mouse

    auto cell = FindCell (hWnd, x, y);
    auto rect = FindCellRect (hWnd, cell.first, cell.second);

    if (!IsRectEmpty (&rect)) {
        
        if (message == WM_LBUTTONUP) {
            auto item = GetWindowExtra (hWnd, &Data::drag_item);
            auto column = GetWindowExtra (hWnd, &Data::drag_column);
            auto drag = FindCellRect (hWnd, item, column);

            SetWindowExtra (hWnd, &Data::drag_item, -1);
            SetWindowExtra (hWnd, &Data::drag_column, -1);
            
            if (!IsRectEmpty (&drag)) {
                InvalidateRect (hWnd, &drag, FALSE);
            };
            
            if (item != -1u && column != -1u && (wParam & MK_CONTROL)) {
                if (item != cell.first || column != cell.second) {
                    
                    if ((wParam & MK_SHIFT) && (column == cell.second)) {
                        
                        // drag many
                        
                        auto first = std::min (item, cell.first);
                        auto last = std::max (item, cell.first);
                        
                        for (auto i = first; i != last + 1; ++i) {
                            if (i != item) {
                                if (!SendDragNotify (hWnd, item, column, i, cell.second)) {
                                    
                                    wchar_t * response = nullptr;
                                    auto value = (unsigned int) SendRequestNotify (hWnd, Request::Value, item, column);
                                    
                                    if (value == -1u || value <= 0xFFFF) {
                                        response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * 8);
                                        _snwprintf (response, 8, L"%d", value);
                                    } else {
                                        auto str = (const wchar_t *) value;
                                        auto len = std::wcslen (str) + 1;
                                        response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * len);
                                        _snwprintf (response, len, L"%s", str);
                                    };
                                    
                                    if (response) {
                                        if (!SendChangeNotify (hWnd, i, cell.second, response)) {
                                            // freeing only when returning zero
                                            //  - non-zero return means "moving from"
                                            HeapFree (hHeap, 0, response);
                                        };
                                    };
                                };
                            };
                        };
                        InvalidateRect (hWnd, NULL, FALSE);
                        
                    } else {
                        
                        // drag single item
                    
                        if (!SendDragNotify (hWnd, item, column, cell.first, cell.second)) {
                            
                            wchar_t * response = nullptr;
                            auto value = (unsigned int) SendRequestNotify (hWnd, Request::Value, item, column);
                            
                            if (value == -1u || value <= 0xFFFF) {
                                response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * 8);
                                _snwprintf (response, 8, L"%d", value);
                            } else {
                                auto str = (const wchar_t *) value;
                                auto len = std::wcslen (str) + 1;
                                response = (wchar_t *) HeapAlloc (hHeap, 0, sizeof (wchar_t) * len);
                                _snwprintf (response, len, L"%s", str);
                            };
                            
                            if (response) {
                                if (!SendChangeNotify (hWnd, cell.first, cell.second, response)) {
                                    // freeing only when returning zero
                                    //  - non-zero return means "moving from"
                                    HeapFree (hHeap, 0, response);
                                };
                            };
                        };
                        InvalidateRect (hWnd, &rect, FALSE);
                    };
                };
                
                return 0;
            };
        };
        
        // else handle action
        
        ActionOnCell (hWnd, message, wParam, x, y, cell.first, cell.second, rect);
        InvalidateRect (hWnd, &rect, FALSE);
    };
    return 0;
};

LRESULT OnMouseMove (HWND hWnd, SHORT x, SHORT y) {
    auto hotitem = GetWindowExtra (hWnd, &Data::hot_item);
    auto hotcolumn = GetWindowExtra (hWnd, &Data::hot_column);

    if ((x == -1) && (y == -1)) {
        auto hotitem = GetWindowExtra (hWnd, &Data::hot_item);
        auto hotcolumn = GetWindowExtra (hWnd, &Data::hot_column);

        SetWindowExtra (hWnd, &Data::hot_item, -1u);
        SetWindowExtra (hWnd, &Data::hot_column, -1u);
        
        SendTrackNotify (hWnd, -1u, -1u);

        RECT r = FindCellRect (hWnd, hotitem, hotcolumn);
        InvalidateRect (hWnd, &r, FALSE);

    } else {
        auto hotnew = FindCell (hWnd, x, y);
        if (hotitem != hotnew.first || hotcolumn != hotnew.second) {
            RECT rPre = FindCellRect (hWnd, hotitem, hotcolumn);
            RECT rNew = FindCellRect (hWnd, hotnew.first, hotnew.second);
            
            SetWindowExtra (hWnd, &Data::hot_item, hotnew.first);
            SetWindowExtra (hWnd, &Data::hot_column, hotnew.second);

            SendTrackNotify (hWnd, hotnew.first, hotnew.second);
            
            InvalidateRect (hWnd, &rPre, FALSE);
            InvalidateRect (hWnd, &rNew, FALSE);
        };

        if (hotnew.first != -1u && hotnew.second != -1u) {
            UINT type = SendRequestNotify (hWnd, Request::Type,
                                           hotnew.first, hotnew.second, 0);
            switch ((CellType) type) {
                case CellType::Inactive:
                case CellType::Grayed:
                case CellType::Checkbox:
                case CellType::MenuItem:
                    SetCursor (hArrowCursor);
                    break;
                case CellType::Selection:
                case CellType::MultipleSelection:
                    if (GetKeyState (VK_CONTROL) & 0x8000) {
                        SetCursor (hDragCtrCursor);
                    } else {
                        SetCursor (hArrowCursor);
                    };
                    break;
                case CellType::Text:
                    if (GetKeyState (VK_CONTROL) & 0x8000) {
                        SetCursor (hDragCtrCursor);
                    } else {
                        SetCursor (hTextCursor);
                    };
                    break;
            };
        } else {
            SetCursor (hArrowCursor);
        };
        
        TrackMouseEvent (hWnd, TME_LEAVE);
    };
    return 0;
};

void SimulateMouseMove (HWND hWnd) {
    POINT pt;
    if (GetCursorPos (&pt)) {
        if (ScreenToClient (hWnd, &pt)) {
            OnMouseMove (hWnd, pt.x, pt.y);
        };
    };
    return;
};

LRESULT OnKeyActionOnCell (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    const unsigned int item = GetWindowExtra (hWnd, &Data::item);
    const unsigned int column = GetWindowExtra (hWnd, &Data::column);

    if ((item != -1u) && (column != -1u)) {
        RECT r = FindCellRect (hWnd, item, column);
        if (!IsRectEmpty (&r)) {
            InvalidateRect (hWnd, &r, FALSE);

            if (!(lParam & 0x40000000)) {
                ActionOnCell (hWnd, message, wParam, 0, 0, item, column, r);
            };
        };
    };
    
    return 0;
};

LRESULT OnKeyDown (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    NMKEY nmKey = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_KEYDOWN },
        wParam, (UINT) lParam
    };
    if (!SendMessage (GetParent (hWnd), WM_NOTIFY,
                      nmKey.hdr.idFrom, (LPARAM) &nmKey)) {

        const unsigned int item = GetWindowExtra (hWnd, &Data::item);
        const unsigned int column = GetWindowExtra (hWnd, &Data::column);

        switch (wParam) {
            case VK_SHIFT:
                InvalidateRect (hWnd, NULL, FALSE);
                break;
            case VK_CONTROL:
                if (!(lParam & 0x40000000)) {
                    SimulateMouseMove (hWnd);
                };
                break;
            case VK_UP:
            case VK_DOWN:
            case VK_LEFT:
            case VK_RIGHT:
                if (item == -1u || column == -1u) {
                    // TODO: set first not-grayed cell active
                    SetActiveCell (hWnd, WM_KEYDOWN, 0u, 0u);
                } else
                    switch (wParam) {
                        case VK_UP:
                            if (item > 0u) {
                                // TODO: iterate, find first activable
                                SetActiveCell (hWnd, WM_KEYDOWN, item - 1u, column);
                            };
                            break;
            
                        case VK_DOWN:
                            if (item < SendBasicNotify (hWnd, Request::Rows) - 1u) {
                                SetActiveCell (hWnd, WM_KEYDOWN, item + 1u, column);
                            };
                            break;
                            
                        case VK_LEFT:
                        case VK_RIGHT:
                            int n = SendDlgItemMessage (hWnd, 1u, HDM_GETITEMCOUNT, 0, 0);
                            if (n > 0) {
                                if (int * index = (int *) HeapAlloc (hHeap, 0, n * sizeof (int))) {
                                    if (SendDlgItemMessage (hWnd, 1u, HDM_GETORDERARRAY, n, (LPARAM) index)) {
                                        for (int i = 0; i != n; ++i) {
                                            if (index [i] == (int) column) {
                                                switch (wParam) {
                                                    case VK_LEFT:
                                                        if (i > 0) {
                                                            SetActiveCell (hWnd, WM_KEYDOWN, item, index [i - 1]);
                                                        };
                                                        break;
                                                    case VK_RIGHT:
                                                        if (i < n - 1) {
                                                            SetActiveCell (hWnd, WM_KEYDOWN, item, index [i + 1]);
                                                        };
                                                        break;
                                                };
                                                break;
                                            };
                                        };
                                    };
                                    HeapFree (hHeap, 0, index);
                                };
                            };
                            break;
                    };  
                break;
            
            case VK_TAB:   
                if (GetKeyState (VK_SHIFT) & 0x8000) {
                    if (item > 0) {
                        SetActiveCell (hWnd, WM_KEYDOWN, item - 1u, column);
                    };
                } else {
                    if (item < SendBasicNotify (hWnd, Request::Rows) - 1u) {
                        SetActiveCell (hWnd, WM_KEYDOWN, item + 1u, column);
                    };
                };
                break;

            case VK_SPACE:
            case VK_RETURN:
                UINT type = SendRequestNotify (hWnd, Request::Type, item, column);
                switch ((CellType) type) {
                    case CellType::Inactive:
                    case CellType::Grayed:
                        break;
                        
                    case CellType::Checkbox:
                    case CellType::MenuItem:
                    case CellType::Selection:
                    case CellType::MultipleSelection:
                        if (!(lParam & 0x40000000)) {
                            const unsigned int item = GetWindowExtra (hWnd, &Data::item);
                            const unsigned int column = GetWindowExtra (hWnd, &Data::column);
            
                            if ((item != -1u) && (column != -1u)) {
                                RECT r = FindCellRect (hWnd, item, column);
                                InvalidateRect (hWnd, &r, FALSE);
                                
                                return OnKeyActionOnCell (hWnd, WM_KEYDOWN, wParam, lParam);
                            };
                        };
                        break;
                        
                    case CellType::Text:
                        SetActiveCell (hWnd, WM_KEYDOWN, item, column);
                        break;
                };
                break;
        };
    };
    return 0;
};

LRESULT OnKeyUp (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    switch (wParam) {
        case VK_SHIFT:
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        case VK_CONTROL:
            SimulateMouseMove (hWnd);
            break;
        case VK_SPACE:
        case VK_RETURN:
            return OnKeyActionOnCell (hWnd, WM_KEYUP, wParam, lParam);
    };
    return 0;
};

LRESULT OnChar (HWND, WPARAM, LPARAM) {
    return 0;
};

bool ScrollRectToView (HWND hWnd, const RECT & r) {
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_PAGE | SIF_POS;
    
    bool moved = false;

    if (GetScrollInfo (hWnd, SB_HORZ, &si)) {
        if (r.left < 0) {
            OnHorizontalScroll (hWnd, 0xFFFF, r.left);
            moved = true;
        } else
        if (r.right >= (int) si.nPage) {
            OnHorizontalScroll (hWnd, 0xFFFF, r.right - si.nPage);
            moved = true;
        };
    };
    if (GetScrollInfo (hWnd, SB_VERT, &si)) {
        int header = GetHeaderHeight (hWnd);
        if (r.top < header) {
            OnVerticalScroll (hWnd, 0xFFFF, r.top - header);
            moved = true;
        } else
        if (r.bottom >= (int) si.nPage + header) {
            OnVerticalScroll (hWnd, 0xFFFF, r.bottom - si.nPage - header);
            moved = true;
        };
    };
    
    if (moved) {
        InvalidateRect (hWnd, NULL, FALSE);
    };
    return moved;
};

bool SetActiveCell (HWND hWnd, UINT message, UINT item, UINT column) {
    
    // remove previous editor

    const unsigned int previous_item = GetWindowExtra (hWnd, &Data::item);
    const unsigned int previous_column = GetWindowExtra (hWnd, &Data::column);
    
    if ((previous_item != -1u) && (previous_column != -1u)) {
        RECT r = FindCellRect (hWnd, previous_item, previous_column);
        InvalidateRect (hWnd, &r, FALSE);
    };
    
    // set new, if any
    
    if ((item != -1u) && (column != -1u)) {
        UINT type = SendRequestNotify (hWnd, Request::Type, item, column, 0);
        RECT r = FindCellRect (hWnd, item, column);
        
        ScrollRectToView (hWnd, r);
        
        // activate
        
        switch ((CellType) type) {
            case CellType::Inactive:
            case CellType::Grayed:
                ShowWindow (GetDlgItem (hWnd, 3), SW_HIDE);
                return false;
                
            case CellType::Checkbox:
            case CellType::Selection:
            case CellType::MultipleSelection:
                ShowWindow (GetDlgItem (hWnd, 3), SW_HIDE);
                break;
            case CellType::Text:
                ActionOnCell (hWnd, message, 0, 0, 0, item, column,
                              FindCellRect (hWnd, item, column));
                SetFocus (GetDlgItem (hWnd, 3));
                break;
                
            case CellType::MenuItem:
//            case CellType::DecimalNumber:
//            case CellType::RealNumber:
//            case CellType::MonetaryValue:
                ShowWindow (GetDlgItem (hWnd, 3), SW_HIDE);
                break;
        };
        
        SetWindowExtra (hWnd, &Data::item, item);
        SetWindowExtra (hWnd, &Data::column, column);

        InvalidateRect (hWnd, &r, FALSE);
        SendEnterNotify (hWnd, item, column);
    };
    return true;
};

void UpdateGridControlPos (HWND hWnd) {
    auto hHeader = GetDlgItem (hWnd, 1);
    auto x = GetWindowExtra (hWnd, &Data::anchor_x) - GetScrollPos (hWnd, SB_HORZ);
    auto y = GetWindowExtra (hWnd, &Data::anchor_y) - GetScrollPos (hWnd, SB_VERT);
    
    for (const auto & idCtrl : { 3, 4 }) {
        if (HWND hCtrl = GetDlgItem (hWnd, idCtrl)) {
            if (IsWindowVisible (hCtrl)) {
                SetWindowPos (hCtrl, hHeader, x, y, 0, 0,
                              SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_NOSIZE);
            };
        };
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
                si.nPos -= si.nPage;
                break;
            case SB_PAGERIGHT:
                si.nPos += si.nPage;
                break;
                
            case SB_LINELEFT:
                si.nPos -= n * GetWindowExtra (hWnd, &Data::row_height);
                break;
            case SB_LINERIGHT:
                si.nPos += n * GetWindowExtra (hWnd, &Data::row_height);
                break;
            
            case SB_THUMBTRACK:
                if (si.nPos == si.nTrackPos) {
                    int target = GetScrollPos (hWnd, SB_HORZ);
                    if (std::abs (si.nPos - target) > 1) {
                        si.nPos = target;
                    };
                } else
                    si.nPos = si.nTrackPos;
                break;
                
            case SB_THUMBPOSITION:
                break;

            case 0xFFFF:
                si.nPos += (int) n;
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
        
        RECT r;
        if (GetClientRect (hWnd, &r)) {
            ScrollWindowEx (hWnd, previous - si.nPos, 0, &r, &r,
                            NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
        };
        UpdateGridControlPos (hWnd);
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

        const auto hh = GetHeaderHeight (hWnd);
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
                si.nPos -= r.bottom - hh;
                break;
            case SB_PAGEDOWN:
                si.nPos += r.bottom - hh;
                break;

            case SB_LINEUP:
                si.nPos -= n * GetWindowExtra (hWnd, &Data::row_height);
                break;
            case SB_LINEDOWN:
                si.nPos += n * GetWindowExtra (hWnd, &Data::row_height);
                break;

            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                si.nPos = si.nTrackPos;
                break;

            case 0xFFFF:
                si.nPos += (int) n;
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
        
        r.top = hh;
        ScrollWindowEx (hWnd, 0, previous - si.nPos, &r, &r,
                        NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
        UpdateGridControlPos (hWnd);
    };
    return result;
};

LRESULT OnHorizontalWheel (HWND hWnd, int distance, USHORT flags) {
    distance += GetWindowExtra (hWnd, &Data::wheelH);

    const unsigned int chars = GetSystemParameter <SPI_GETWHEELSCROLLCHARS> ();
    while (distance >= +WHEEL_DELTA) {
        distance -= WHEEL_DELTA;
        OnHorizontalScroll (hWnd,
                            (flags & MK_CONTROL) ? SB_PAGERIGHT : SB_LINERIGHT,
                            chars);
    };
    while (distance <= -WHEEL_DELTA) {
        distance += WHEEL_DELTA;
        OnHorizontalScroll (hWnd,
                            (flags & MK_CONTROL) ? SB_PAGELEFT : SB_LINELEFT,
                            chars);
    };

    SetWindowExtra (hWnd, &Data::wheelH, (signed char) distance);
    return 0;
};
LRESULT OnVerticalWheel (HWND hWnd, int distance, USHORT flags) {
    distance += GetWindowExtra (hWnd, &Data::wheelV);
    
    // SPI_GETWHEELSCROLLLINES
    //  - user setting on how many lines should scroll wheel actually scroll
    //  - -1u (WHEEL_PAGESCROLL == UINT_MAX) is specified to scroll the whole
    //    page and the same usually does holding CTRL, so do the same
    
    const unsigned int lines = GetSystemParameter <SPI_GETWHEELSCROLLLINES> ();
    while (distance >= +WHEEL_DELTA) {
        distance -= WHEEL_DELTA;
        
        if (lines == -1u || flags & MK_CONTROL) {
            OnVerticalScroll (hWnd, SB_PAGEUP);
        } else {
            OnVerticalScroll (hWnd, SB_LINEUP, lines);
        };
    };
    while (distance <= -WHEEL_DELTA) {
        distance += WHEEL_DELTA;

        if (lines == -1u || flags & MK_CONTROL) {
            OnVerticalScroll (hWnd, SB_PAGEDOWN);
        } else {
            OnVerticalScroll (hWnd, SB_LINEDOWN, lines);
        };
    };

    SetWindowExtra (hWnd, &Data::wheelV, (signed char) distance);
    return 0;
};


UINT UpdateHorizontalScrollBar (HWND hWnd, UINT max, UINT page) {
    if (!max) {
        if (HWND hHeader = GetDlgItem (hWnd, 1)) {
            UINT count = Header_GetItemCount (hHeader);
            
            if (count != -1u) {
                UINT index = Header_OrderToIndex (hHeader, count - 1u);
                RECT r;
                if (Header_GetItemRect (hHeader, index, &r)) {
                    max = r.right - 1;
                };
            };
        };
    };
    
    if (!page) {
        RECT r;
        if (GetClientRect (hWnd, &r)) {
            page = r.right;
        };
    };
    
    SCROLLINFO si = {
        sizeof si, SIF_PAGE | SIF_RANGE,
        0, (int) max, page, 0, 0
    };
    if (!(GetWindowLongPtr (hWnd, GWL_STYLE) & WS_HSCROLL)) {
        si.fMask |= SIF_DISABLENOSCROLL;
    };

    SetScrollInfo (hWnd, SB_HORZ, &si, TRUE);
    return std::max (max, page);
};

#ifndef HDM_GETFOCUSEDITEM
#define HDM_GETFOCUSEDITEM (HDM_FIRST+27)
#endif

LRESULT CALLBACK HeaderSubclass (HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    SetFocus (GetParent (hWnd));
                    break;
                case VK_DOWN:
                    SetActiveCell (GetParent (hWnd), message, 0u,
                                   SendMessage (hWnd, HDM_GETFOCUSEDITEM, 0, 0));
                    break;
            };
        break;
    };
    return CallWindowProc ((WNDPROC) GetProp (hWnd, TEXT ("Shadow::Grid::Subclass")),
                           hWnd, message, wParam, lParam);
};

LRESULT CALLBACK EditorSubclass (HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_RETURN:
                    return OnKeyDown (GetParent (hWnd), VK_DOWN, lParam);

                case VK_ESCAPE:
                    SetFocus (GetParent (hWnd));
                    return 0;
                
                case VK_UP:
                case VK_DOWN:
                case VK_TAB:
                    return OnKeyDown (GetParent (hWnd), wParam, lParam);

                case VK_LEFT:
                case VK_RIGHT:
                    if (GetKeyState (VK_MENU) & 0x8000)
                        return OnKeyDown (GetParent (hWnd), wParam, lParam);
                    
                    LRESULT length = SendMessage (hWnd, WM_GETTEXTLENGTH, 0, 0);
                    LRESULT selection = SendMessage (hWnd, EM_GETSEL, 0, 0);
                    
                    if (LOWORD (selection) == HIWORD (selection)) {
                        if (wParam == VK_LEFT && LOWORD (selection) == 0) {
                            return OnKeyDown (GetParent (hWnd), wParam, lParam);
                        };
                        if (wParam == VK_RIGHT && LOWORD (selection) >= length) {
                            return OnKeyDown (GetParent (hWnd), wParam, lParam);
                        };
                    };
                    break;
            };
            break;
        
        case WM_CHAR:
        case WM_KEYUP:
            switch (wParam) {
                case VK_ESCAPE:
                    return 0;
            };
            break;
        
        case WM_GETDLGCODE:
            switch (wParam) {
                case VK_RETURN:
                case VK_ESCAPE:
                    return DLGC_WANTALLKEYS;
                case VK_TAB:
                    auto parent = GetParent (hWnd);
                    if (GetWindowLongPtr (parent, GWL_STYLE) & Shadow::Grid::Style::WantTab) {
                        const unsigned int item = GetWindowExtra (parent, &Data::item);
                        if (GetKeyState (VK_SHIFT) & 0x8000) {
                            if (item > 0) {
                                return DLGC_WANTALLKEYS | DLGC_WANTTAB;
                            };
                        } else {
                            if (item < SendBasicNotify (parent, Request::Rows) - 1u) {
                                return DLGC_WANTALLKEYS | DLGC_WANTTAB;
                            };
                        };
                    };
                    break;
            };
            break;
                           
    };
    return CallWindowProc ((WNDPROC) GetProp (hWnd, TEXT ("Shadow::Grid::Subclass")),
                           hWnd, message, wParam, lParam);
};

LRESULT SetSortOrder (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    HDITEM item;
    memset (&item, 0, sizeof item);
    item.mask = HDI_FORMAT;
    
    if (SendDlgItemMessage (hWnd, 1, HDM_GETITEM, wParam, (LPARAM) &item)) {
        
        item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        
        if (lParam > 0) item.fmt |= HDF_SORTUP;
        if (lParam < 0) item.fmt |= HDF_SORTDOWN;
        
        return SendDlgItemMessage (hWnd, 1, HDM_SETITEM, wParam, (LPARAM) &item);
    };
    
    return 0;
};
LRESULT GetSortOrder (HWND hWnd, WPARAM wParam, LPARAM) {
    HDITEM item;
    memset (&item, 0, sizeof item);
    item.mask = HDI_FORMAT;
    
    if (SendDlgItemMessage (hWnd, 1, HDM_GETITEM, wParam, (LPARAM) &item)) {
        
        if (item.fmt & HDF_SORTUP) return +1;
        if (item.fmt & HDF_SORTDOWN) return -1;
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
LRESULT SendRequestNotify (HWND hWnd, UINT rq, UINT row, UINT column, UINT index) {
    if (column == -1u || row == -1u)
        return 0;
    
    NMRequest request = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), rq },
        row, column, index
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        request.hdr.idFrom, (LPARAM) &request);
};
LRESULT SendPrefetchNotify (HWND hWnd, UINT rowA, UINT rowZ, UINT colA, UINT colZ) {
    NMPrefetch prefetch = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Prefetch },
        { rowA, rowZ }, { colA, colZ }
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        prefetch.hdr.idFrom, (LPARAM) &prefetch);
};
LRESULT SendChangeNotify (HWND hWnd, UINT row, UINT column, const wchar_t * value) {
    NMChange change = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Change },
        row, column, value
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        change.hdr.idFrom, (LPARAM) &change);
};
LRESULT SendDragNotify (HWND hWnd, UINT row0, UINT col0, UINT row1, UINT col1) {
    NMDrag drag = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Drag },
        { row0, col0 }, { row1, col1 }
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        drag.hdr.idFrom, (LPARAM) &drag);
};
LRESULT SendMouseNotify (HWND hWnd, UINT type,
                         DWORD_PTR  i, DWORD_PTR c, const POINT &ptMouse) {
    NMMOUSE nmMouse = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), type },
        i, c, ptMouse, HTCLIENT
    };
    MapWindowPoints (hWnd, NULL, &nmMouse.pt, 1u);
    
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmMouse.hdr.idFrom, (LPARAM) &nmMouse);
};
LRESULT SendTrackNotify (HWND hWnd, UINT row, UINT column) {
    NMTrack track = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Track },
        row, column
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        track.hdr.idFrom, (LPARAM) &track);
};
LRESULT SendEnterNotify (HWND hWnd, UINT row, UINT column) {
    NMTrack track = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::Enter },
        row, column
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        track.hdr.idFrom, (LPARAM) &track);
};

};
