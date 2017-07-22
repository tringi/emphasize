#include "Shadow_TabControl.hpp"

/* Emphasize Shadow Controls Library - TabControl
// Shadow_TabControl.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: 
//
// Changelog:
//      04.04.2011 - initial version
*/

#include <cstddef>
#include <algorithm>
#include <limits>

#include <cstdio>

#include "../Windows/Windows_WindowExtraMemory.hpp"
#include "../Windows/Windows_W2kCompatibility.hpp"
#include "../Windows/Windows_DeferWindowPos.hpp"
#include "../Windows/Windows_UxTheme.hpp"
#include "../Windows/Windows_Version.hpp"

using namespace Windows;
using namespace Shadow::TabControl;

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    
    struct Data {
        HFONT   hFont;

        UCHAR   pxActiveDiffTop;
        UCHAR   pxActiveDiffBottom;
        UCHAR   pxPaddingLeft;
        UCHAR   pxSubItemDiffTop;
        
        USHORT  current;        // activated tab
        USHORT  offset;         // first tab on the left
        USHORT  last;           // last fully rendered tab
        USHORT  hot;            // mouse is over this one, or -1u if over none
        
        UCHAR   pxPaddingRight;
        CHAR    wheelDeltaAccum;
    };
};

ATOM Shadow::TabControl::Initialize (HINSTANCE hInstance) {
    WNDCLASS wc;
    
    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = Procedure;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = GetWindowExtraSize <Data> ();
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT ("Shadow::TabControl");
    
    atom = RegisterClass (&wc);
    return atom;
};

HWND Shadow::TabControl::Create (HINSTANCE hInstance, HWND hParent,
                                 UINT style, UINT id) {
    
    return CreateWindowEx (0u, (LPCTSTR) (INT) atom, TEXT(""),
                           style | WS_CHILD | WS_CLIPCHILDREN,
                           0,0,0,0, hParent, (HMENU) id, hInstance, NULL);
};

namespace {

LRESULT OnCreate (HWND, UINT);
LRESULT OnDestroy (HWND);
LRESULT OnSize (HWND, UINT, USHORT, USHORT);
LRESULT OnPaint (HWND, HDC, RECT);
LRESULT OnClick (HWND, UINT, SHORT, SHORT);
LRESULT OnHitTest (HWND, SHORT, SHORT);
LRESULT OnMouseMove (HWND, SHORT, SHORT);
LRESULT OnKeyDown (HWND, WPARAM, LPARAM);
LRESULT OnHorizontalWheel (HWND, SHORT, USHORT);
LRESULT OnNotify (HWND, UINT, const NMHDR *);

LRESULT SendBasicNotify (HWND, UINT);
LRESULT SendMouseNotify (HWND, UINT, UINT, RECT &, const POINT &);
LRESULT CustomDrawNotify (HWND, DWORD, HDC, LPCRECT,
                          UINT = 0, UINT = CDIS_DEFAULT, LPARAM = 0);
LRESULT SendRequestNotify (HWND, UINT, UINT = 0u);

UINT GetVisualStyle (HWND, HANDLE);
UINT GetFullWidth (HWND);
void RenderTabFrame (HDC, const RECT &, bool, bool);

void InvokeSizeHandler (HWND hWnd) {
    RECT r;
    if (GetClientRect (hWnd, &r)) {
        OnSize (hWnd, SIZE_RESTORED, r.right, r.bottom);
    };
};

void Left (HWND);
void Right (HWND);
void Next (HWND);
void Previous (HWND);

// The Procedure

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return OnCreate (hWnd, ((CREATESTRUCT *) lParam)->style);
        case WM_DESTROY:
            return OnDestroy (hWnd);
        
        case WM_SETFONT:
            SetWindowExtra (hWnd, &Data::hFont, (HFONT) wParam);
            SendDlgItemMessage (hWnd, 3, message, wParam, lParam);
            if (LOWORD (lParam))
                InvalidateRect (hWnd, NULL, TRUE);
            break;
        case WM_GETFONT:
            return (LRESULT) GetWindowExtra (hWnd, &Data::hFont);
        
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
            return SendMessage (GetParent (hWnd), message, wParam, lParam);
        
        case WM_SIZE:
            return OnSize (hWnd, wParam, LOWORD (lParam), HIWORD (lParam));
        
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
        
        case WM_NCHITTEST:
            return OnHitTest (hWnd, LOWORD (lParam), HIWORD (lParam));
        
        case WM_MOUSEMOVE:
            return OnMouseMove (hWnd, LOWORD (lParam), HIWORD (lParam));
        case WM_MOUSELEAVE:
            return OnMouseMove (hWnd, -1, -1);
        
        case WM_LBUTTONDOWN:
            SetFocus (hWnd);
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            return OnClick (hWnd, message, LOWORD (lParam), HIWORD (lParam));
        case WM_KEYDOWN:
            return OnKeyDown (hWnd, wParam, lParam);
        
        case WM_MOUSEHWHEEL:
            return OnHorizontalWheel (hWnd, HIWORD (wParam), LOWORD (wParam));
        
        case WM_CHAR: {
            NMCHAR nmChar = {
                { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_CHAR },
                wParam, 0u, 0u
            };
            SendMessage (GetParent (hWnd), WM_NOTIFY,
                         nmChar.hdr.idFrom, (LPARAM) &nmChar);
        } break;
        
        case WM_UPDATEUISTATE:
        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        
        case WM_GETDLGCODE:
            switch (wParam) {
                case VK_LEFT:
                case VK_RIGHT:
                    return DLGC_WANTARROWS;
            };
            break;

        case Message::Update:
            SetWindowExtra (hWnd, &Data::current, wParam);
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        case Message::Inserted:
            if (wParam <= GetWindowExtra (hWnd, &Data::current)) {
                SetWindowExtra (hWnd, &Data::current,
                                GetWindowExtra (hWnd, &Data::current) + 1u);
            };
            break;
        case Message::Removed:
            if (wParam == GetWindowExtra (hWnd, &Data::current)) {
                const USHORT size = SendRequestNotify (hWnd, Request::Size);
                if (size > 1u && wParam == size - 1u) {
                    SetWindowExtra (hWnd, &Data::current, size - 2u);
                    SendMessage (GetParent (hWnd), WM_COMMAND,
                                 MAKEWPARAM (GetDlgCtrlID (hWnd), size - 2u),
                                 (LPARAM) hWnd);
                } else {
                    SendMessage (GetParent (hWnd), WM_COMMAND,
                                 MAKEWPARAM (GetDlgCtrlID (hWnd), wParam),
                                 (LPARAM) hWnd);
                };
            } else {
                const USHORT current = GetWindowExtra (hWnd, &Data::current);
                if (wParam < current) {
                    SetWindowExtra (hWnd, &Data::current, current - 1u);
                };
            };
            break;

        case Message::Left:
            Left (hWnd);
            break;
        case Message::Right:
            Right (hWnd);
            break;
        case Message::Next:
            Next (hWnd);
            break;
        case Message::Previous:
            Previous (hWnd);
            break;
        
        case WM_COMMAND:
            switch (wParam) {
                case 3:
                    Left (hWnd);

                    if (lParam && HIWORD (wParam) == 0) {
                        LONG_PTR style = GetWindowLongPtr ((HWND) lParam, GWL_STYLE);
                        if (style & BS_DEFPUSHBUTTON) {
                            style &= ~BS_DEFPUSHBUTTON;
                            SetWindowLongPtr ((HWND) lParam, GWL_STYLE, style);
                            InvalidateRect ((HWND) lParam, NULL, FALSE);
                        };
                    };
                    break;
            };
            break;
        case WM_NOTIFY:
            return OnNotify (hWnd, wParam, reinterpret_cast <NMHDR *> (lParam));
        
        case Message::GetCurSel:
            return GetWindowExtra (hWnd, &Data::current);
        case Message::GetFullWidth:
            return GetFullWidth (hWnd);

        case Message::SetActiveDims:
            if (LOWORD (wParam))
                SetWindowExtra (hWnd, &Data::pxActiveDiffBottom, LOWORD (wParam));
            if (HIWORD (wParam))
                SetWindowExtra (hWnd, &Data::pxActiveDiffTop, HIWORD (wParam));

            SetWindowExtra (hWnd, &Data::pxPaddingLeft, LOWORD (lParam));
            SetWindowExtra (hWnd, &Data::pxPaddingRight, HIWORD (lParam));
            break;
        
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT OnCreate (HWND hWnd, UINT style) {
    // TODO: DPI!
    SetWindowExtra (hWnd, &Data::pxActiveDiffTop, 2u);
    SetWindowExtra (hWnd, &Data::pxActiveDiffBottom, 1u);
    SetWindowExtra (hWnd, &Data::pxSubItemDiffTop, 2u);
    SetWindowExtra (hWnd, &Data::hot, -1u);

/*    if (style & Shadow::TabControl::Style::Buttons) {
                        CreateWindow (UPDOWN_CLASS, TEXT(""),
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | UDS_HORZ,
                                      0,0,0,0, hWnd, (HMENU) 1, (HINSTANCE)
                                      GetWindowLongPtr (hWnd, GWLP_HINSTANCE),
                                      NULL));
    };
    */
    if (style & Shadow::TabControl::Style::RewindBtn) {
        CreateWindow (TEXT("BUTTON"), TEXT("<"),
                      WS_CHILD | WS_VISIBLE | WS_TABSTOP, // BUTTON... image?
                      0,0,0,0, hWnd, (HMENU) 3,
                      (HINSTANCE) GetWindowLongPtr (hWnd, GWLP_HINSTANCE),
                      NULL);
    };
    
    SendMessage (hWnd, WM_CHANGEUISTATE, UIS_INITIALIZE, 0u);
    return 0;
};

LRESULT OnDestroy (HWND) {
    return 0;
};

LRESULT OnSize (HWND hWnd, UINT type, USHORT width, USHORT height) {
    if (type == SIZE_MINIMIZED)
        return 0;
    
    
/*            RECT r = {
                LOWORD (lParam) - 2 * GetSystemMetrics (SM_CXSMSIZE),
                0,
                2 * GetSystemMetrics (SM_CXSMSIZE),
                GetSystemMetrics (SM_CYSMSIZE) - 1
            };
*/            
    HDWP hdwp = BeginDeferWindowPos (3);
//            Windows::DeferWindowPos (hdwp, Windows::GetFirstChild (hWnd), r);

    Windows::DeferWindowPos (hdwp, hWnd, 3,
                             { 0, 1,
                               GetWindowExtra (hWnd, &Data::offset) ? 20 : 0,
                               height - 4 });
    if (hdwp)
        EndDeferWindowPos (hdwp);
};

LRESULT OnNotify (HWND hWnd, UINT id, const NMHDR * nmhdr) {
    auto * nmDraw = reinterpret_cast <const NMCUSTOMDRAW *> (nmhdr);
    
    switch (id) {
        case 1:
        case 2:
        case 3:
            if ((int) nmhdr->code == NM_CUSTOMDRAW && nmDraw->dwDrawStage == CDDS_PREPAINT) {
                    
                TCHAR text [256];
                if (int length = GetWindowText (nmhdr->hwndFrom, text, 256)) {
                    
                    SetBkMode (nmDraw->hdc, TRANSPARENT);
                    
                    if (IsWindowEnabled (nmhdr->hwndFrom)) {
                        SetTextColor (nmDraw->hdc, GetSysColor (COLOR_BTNTEXT));
                    } else {
                        SetTextColor (nmDraw->hdc, GetSysColor (COLOR_GRAYTEXT));
                    };
                    
                    RECT rc = nmDraw->rc;
                    HANDLE hTheme = Windows::UxTheme::Open (hWnd, L"BUTTON");

                    if (Windows::UxTheme::Current () == Windows::UxTheme::Classic
                            || Windows::Version < Windows::Version::WindowsVista) {
                        if (nmDraw->uItemState & CDIS_SELECTED) {
                            OffsetRect (&rc, +1, +1);
                        };
                    };
                
                    Windows::UxTheme::DrawText (hTheme, nmDraw->hdc, text, length,
                                                DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                                                &rc, 0);
                    if (hTheme)
                        Windows::UxTheme::Close (hTheme);
                    
                };
                return CDRF_SKIPDEFAULT;// */
            };
        break;
    };
    
    return 0;
};

class Iterator {
    public:
        unsigned int  i;
        RECT          r;
        LPCTSTR       text;
        bool          left;
        bool          right;
        
        LRESULT       type;
        LRESULT       prev_type;
        LRESULT       next_type;
    
        const USHORT  first;
        const USHORT  current;
        const UINT    width;
        const UINT    style;

    private:
        const HWND    hWnd;
        const UINT    size;
        const UCHAR   bottom;
        const UCHAR   diff_top;
        const UCHAR   diff_btm;
        const UCHAR   diff_sub;
    
    private:
        UINT cutsize (UINT n) {
            static const UINT max = (1u << 8u * sizeof (USHORT)) - 2u;
            return (n < max) ? n : max;
        };
    
    public:
        explicit Iterator (HWND _hWnd, LONG min_width = 0u)
            :   i        (0u),
                text     (TEXT ("")),
                type     (0),
                first    (GetWindowExtra (_hWnd, &Data::offset)),
                current  (GetWindowExtra (_hWnd, &Data::current)),
                width    (std::max (min_width, (GetClientRect (_hWnd, &this->r), this->r.right))),
                style    (GetWindowLong  (_hWnd, GWL_STYLE)),
                hWnd     (_hWnd),
                size     (cutsize (SendRequestNotify (_hWnd, Request::Size))),
                bottom   ((GetClientRect (_hWnd, &this->r), this->r.bottom)),
                diff_top (GetWindowExtra (_hWnd, &Data::pxActiveDiffTop)),
                diff_btm (GetWindowExtra (_hWnd, &Data::pxActiveDiffBottom)),
                diff_sub (GetWindowExtra (_hWnd, &Data::pxSubItemDiffTop)) {
            
            if (this->first == this->current) {
                this->r.right = 0u;
            } else {
                this->r.right = 2u; // TODO: DPI
            };
            
            if (this->style & Style::RewindBtn) {
                if (this->first != 0u) {
                    this->r.right += 20u; // TODO: constant somewhere + DPI
                };
            };
            
            this->i = this->first - 1u;
            this->next_type = SendRequestNotify (this->hWnd, Request::Type, this->i + 1u);
            return;
        };
        
        bool next (HDC hDC) {
            this->r.left = this->r.right;
            this->prev_type = this->type;
            this->type = this->next_type;
            
            ++this->i;

            this->left = true;
            this->right = true;
            
            if (this->i < this->size
                    && (unsigned int) this->r.left < this->width) {
                
                if (this->i + 1u < this->size) {
                    this->next_type = SendRequestNotify (this->hWnd, Request::Type, this->i + 1u);
                } else
                    this->next_type = 0u;

                if (this->type & ItemType::Separator) {

                    this->text = TEXT("");
                    this->r.right += 6; // DPI
                    
                } else {
    
                    // compute right text border with DrawTextEx
                    //  - const_cast is safe since DT_MODIFYSTRING is not preset
    
                    this->text = (LPCTSTR) SendRequestNotify (this->hWnd, Request::Text, this->i);
                    DrawTextEx (hDC, const_cast <LPTSTR> (this->text),
                                -1, &this->r, DT_CALCRECT | DT_SINGLELINE, NULL);

                    // padding
                    //  - 4px for border, 2×4px for padding
                    //  - TODO: DPI
                    
                    this->r.right += 4 + 2 * 4;
                };
    
                // horizontal borders
                //  - non-subitem before current does not render right border
                //  - item after current or subitems does not render left border
                //  - in between subitem and following non-subitem is a space
                //  - TODO: DPI

                if (this->i + 1u == this->current && !(this->type & ItemType::SubItem)) {
                    this->right = false;
                    this->r.right -= 2u;
                };
                if (this->i == this->current + 1u || (this->type & ItemType::SubItem)) {
                    this->left = false;
                    if (this->i == this->current + 1u && (this->type & ItemType::SubItem))
                        this->r.right -= 4u;
                    else
                        this->r.right -= 2u;
                };

                if (this->i == this->current) {
                    this->r.right += 4u;
                };
                
                if ((this->prev_type & ItemType::SubItem) && !(this->type & ItemType::SubItem)) {
                    int offset = 0;
                    
                    if (this->style & Style::Separated)
                        offset = 4;
                    if (this->i == this->current)
                        offset -= 2;
                    
                    this->r.left += offset;
                    this->r.right += offset;
                };
                
                // special style fixes
                
                if (!(this->style & Style::Separated)) {
                    
                    // tight mode just removes gap after last subitem
                    if ((this->type & ItemType::SubItem) && !(this->next_type & ItemType::SubItem)) {
                        this->r.right += 2;
                    };
                };
                
                // vertical dimmensions

                if (this->i == this->current) {
                    this->r.top = 0;
                    this->r.bottom = this->bottom;
                } else
                if (this->type & ItemType::SubItem) {
                    this->r.top = this->diff_top + this->diff_sub;
                    this->r.bottom = this->bottom - this->diff_btm;
                } else {
                    this->r.top = this->diff_top;
                    this->r.bottom = this->bottom - this->diff_btm;
                };
                
                return true;
            } else
                return false;
        };
};

LRESULT OnPaint (HWND hWnd, HDC _hDC, RECT rcInvalid) {
    RECT rc;
    GetClientRect (hWnd, &rc);
    
    HDC hDC = NULL;
    HANDLE hBuffered = UxTheme::BeginBufferedPaint (_hDC, &rc, &hDC);
    
    if (!hBuffered)
        hDC = _hDC;

    SelectObject (hDC, GetWindowExtra (hWnd, &Data::hFont));
    
    FillRect (hDC, &rc, (HBRUSH)
              SendMessage (GetParent (hWnd), WM_CTLCOLORBTN,
                           (WPARAM) hDC, (LPARAM) hWnd));
    
    RECT rFocus;
    DWORD rvPrePaint = CustomDrawNotify (hWnd, CDDS_PREPAINT, hDC, &rc);

    if (!(rvPrePaint & CDRF_SKIPDEFAULT)) {
        
        USHORT hot = GetWindowExtra (hWnd, &Data::hot);
        HANDLE hTheme = UxTheme::Open (hWnd, L"TAB");
        UINT   vs = GetVisualStyle (hWnd, hTheme);
    
        Iterator iterator (hWnd);
        iterator.r.left += GetWindowExtra (hWnd, &Data::pxPaddingLeft);
        iterator.r.right += GetWindowExtra (hWnd, &Data::pxPaddingLeft);

        ExcludeClipRect (hDC,
                         rc.right - GetWindowExtra (hWnd, &Data::pxPaddingRight),
                         rc.top, rc.right,
                         rc.bottom - GetWindowExtra (hWnd, &Data::pxActiveDiffBottom));
        
        while (iterator.next (hDC)) {
            
            RECT rcItersectTemp;
            if (IntersectRect (&rcItersectTemp, &iterator.r, &rcInvalid)) {
                
                // prepare style
                // and select proper resources
                //  - so the callback can change it
                //  - using COLOR_WINDOW for current to allow it seemlessly
                //    connect with editor control
                
                UINT style = iterator.type;
                if (iterator.i == iterator.current) {
                    style |= CDIS_SELECTED;
                    rFocus = iterator.r;
                };
                if (iterator.i == hot)
                    style |= CDIS_HOT;
                
                // TODO: CDIS_SHOWKEYBOARDCUES

                int part = 1; // TABP_TABITEM
                int state = 1; // TIS_NORMAL
                
                if (iterator.type & ItemType::Separator) {
                    
                    // nothing to prepare for separator background (there is none)
                    
                } else {
                    if (iterator.i == iterator.first) part += 1; // ...LEFTEDGE
                    if (iterator.i == iterator.current) part += 4; // ...TOP...
    
                    if (iterator.i == iterator.current) state = 3; // ...SELECTED
                    else if (iterator.i == hot) state = 2; // ...HOT
    
                    switch (vs) {
                        case Style::Classic:
                            SelectObject (hDC, GetStockObject (NULL_PEN));
                            if (iterator.i == iterator.current)
                                SelectObject (hDC, GetSysColorBrush (COLOR_WINDOW));
                            else
                                SelectObject (hDC, GetSysColorBrush (COLOR_BTNFACE));
                            
                            break;
                        case Style::Custom:
                        case Style::Frame:
                            SelectObject  (hDC, GetStockObject (DC_PEN));
                            SetDCPenColor (hDC, GetSysColor (COLOR_3DDKSHADOW));
    
                            if (iterator.i == iterator.current) {
                                SelectObject (hDC, GetSysColorBrush (COLOR_WINDOW));
                            } else {
                                SelectObject (hDC, GetStockObject (DC_BRUSH));
                            };
                            break;
                    };
                };

                DWORD rvPreItemErase = 0;
                if (rvPrePaint & CDRF_NOTIFYITEMDRAW) {
                    rvPreItemErase = CustomDrawNotify (hWnd, CDDS_ITEMPREERASE,
                                                       hDC, &iterator.r,
                                                       iterator.i, style);
                };
    
                if (!(rvPreItemErase & CDRF_SKIPDEFAULT)) {

                    if (iterator.type & ItemType::Separator) {
                        
                        // Rendering separator background? There is none.
                        
                    } else {
            
                        // Render background and frame
                        //  - using selected pens and brushes which might have been
                        //    changed by callback
                        
                        switch (vs) {
                            case Style::Classic: {
                                RECT r = {
                                    iterator.r.left,
                                    iterator.r.top    + 2,
                                    iterator.r.right  + 1,
                                    iterator.r.bottom + 1
                                };
                                UINT edge = BF_TOP;
    
                                if (iterator.left) {
                                    edge |= BF_LEFT;
                                    r.left += 2;
                                };
                                if (iterator.right) {
                                    edge |= BF_RIGHT;
                                    r.right -= 2;
                                };
                                
                                Rectangle (hDC, r.left, r.top, r.right, r.bottom);
                                DrawEdge (hDC, &iterator.r, EDGE_RAISED, edge);
                            } break;
                        
                            case Style::Frame:
                                RenderTabFrame (hDC, iterator.r,
                                                iterator.left, iterator.right);
                                // fallthrough
                                
                            case Style::Theme: {
                                RECT r = iterator.r;
                                if (!iterator.left) {
                                    r.left -= 2;
                                };
                                if (!iterator.right) {
                                    r.right += 2;
                                };
                                
                                // this is basically fix for XP rounded skins
                                if (vs == Style::Frame) {
                                    RECT clip = {
                                        iterator.r.left,
                                        iterator.r.top + 1,
                                        iterator.r.right,
                                        iterator.r.bottom
                                    };
                                    if (iterator.left) { clip.left += 1; };
                                    if (iterator.right) { clip.right -= 1; };
    
                                    if (iterator.i != iterator.current && iterator.right) {
                                        if (LOBYTE (LOWORD (GetVersion ())) < 6) {
                                            r.right += 2;
                                        } else {
                                            clip.right -= 1;
                                        };
                                    };
                                    UxTheme::DrawBackground (hTheme, hDC,
                                                             part, state, &r, &clip);
                                } else {
                                    r.bottom += 1;
                                    UxTheme::DrawBackground (hTheme, hDC,
                                                             part, state, &r, &iterator.r);
                                };
                            } break;
                                
                            case Style::Custom:
                                if (iterator.i == iterator.current) {
                                    Rectangle (hDC, iterator.r.left, iterator.r.top,
                                               iterator.r.right, iterator.r.bottom + 1);
                                } else {
                                    RenderTabFrame (hDC, iterator.r,
                                                    iterator.left, iterator.right);
    
                                    SelectObject (hDC, GetStockObject (NULL_PEN));
                                    SetDCBrushColor (hDC, GetSysColor (COLOR_BTNFACE));
                                    
                                    Rectangle (hDC,
                                               iterator.r.left + (iterator.left ? 1 : 0),
                                               iterator.r.top + 1,
                                               iterator.r.right - (iterator.right ? 0 : -2),
                                               iterator.r.bottom + 1);
                                };
                                break;
                        };
                    };
                }; 
    
                if (rvPreItemErase & CDRF_NOTIFYITEMDRAW) {
                    CustomDrawNotify (hWnd, CDDS_ITEMPOSTERASE,
                                      hDC, &iterator.r, iterator.i, style);
                };

                RECT rText = iterator.r;
                DWORD rvPreItemPaint = 0;

                SetTextColor (hDC, GetSysColor (COLOR_BTNTEXT));
                SetBkMode (hDC, TRANSPARENT);

                switch (vs) {
                    case Style::Classic:
                    case Style::Custom:
                        if (iterator.i == hot
                                && iterator.i != iterator.current) {
                            
                            SetTextColor (hDC, GetSysColor (COLOR_HOTLIGHT));
                        };
                        break;
                };
                
                rText.top += 3;
                if (!iterator.left) rText.right -= 2u;
                if (!iterator.right) rText.left += 2u;
                
                if (iterator.type & ItemType::SubItem) {
                    if (iterator.i == iterator.current + 1u)
                        rText.left += 1u;
                    else
                        rText.left += 3u;
                };

                if (rvPreItemErase & CDRF_NOTIFYITEMDRAW) {
                    rvPreItemPaint = CustomDrawNotify (hWnd, CDDS_ITEMPREPAINT, 
                                                       hDC, &rText,
                                                       iterator.i, style);
                };
                
                if (!(rvPreItemPaint & CDRF_SKIPDEFAULT)) {
                    if (iterator.type & ItemType::Separator) {
                        
                        
                    } else {
                        DWORD flags = DT_NOCLIP | DT_TOP | DT_SINGLELINE;
                        
                        if (iterator.type & ItemType::SubItem) {
                            flags |= DT_LEFT;
                        } else {
                            flags |= DT_CENTER;
                        };
                        
                        // TODO: when partially obscured, append or replace with ...
                        
                        DrawTextEx (hDC, const_cast <LPTSTR> (iterator.text),
                                    -1, &rText, flags, NULL);
                    };
                };

                // set alpha of everything under this tab rectangle
                //  - if not requested to skip drawing it

                if (!(rvPreItemErase & CDRF_SKIPDEFAULT)) {
                    if (!(iterator.type & ItemType::Separator)) {
                        UxTheme::BufferedPaintSetAlpha (hBuffered, &iterator.r, 255);
                    };
                };
    
                if (rvPreItemPaint & CDRF_NOTIFYITEMDRAW) {
                    CustomDrawNotify (hWnd, CDDS_ITEMPOSTPAINT,
                                      hDC, &rText, iterator.i, style,
                                      (LPARAM) &iterator.r);
                };
            };
        };

        if (UxTheme::Current () == UxTheme::Aero) {
            RECT r = {
                rc.right - GetWindowExtra (hWnd, &Data::pxPaddingRight),
                rc.top,
                rc.right,
                rc.bottom - GetWindowExtra (hWnd, &Data::pxActiveDiffBottom)
            };
            UxTheme::BufferedPaintSetAlpha (hBuffered, &r, 0);
        };
        
        // update last fully rendered tab
        //  - do not count space occupied by buttons
        
        {   const int width = rc.right;
            SetWindowExtra (hWnd, &Data::last,
                            iterator.i - !(iterator.r.right < width));
        };
        
        if (!(rvPrePaint & CDRF_SKIPPOSTPAINT)) { 
            if (GetFocus () == hWnd) {
                rFocus.left   += 2;
                rFocus.top    += 2;
                rFocus.right  -= 2;
                rFocus.bottom -= 1;
                
                if (vs == Style::Classic)
                    rFocus.right -= 1;
                
                if (!(LOWORD (SendMessage (GetParent (hWnd), WM_QUERYUISTATE, 0,0))
                                                           & UISF_HIDEFOCUS)) {
                    DrawFocusRect (hDC, &rFocus);
                };
            };
        };
        
        if (hTheme)
            UxTheme::Close (hTheme);
    };
    
    if (rvPrePaint & CDRF_NOTIFYPOSTPAINT) {
        CustomDrawNotify (hWnd, CDDS_POSTPAINT, hDC, &rc);
    };
    
    if (hBuffered)
        UxTheme::EndBufferedPaint (hBuffered);
    
    return 0;
};

void RenderTabFrame (HDC hDC, const RECT & r, bool left, bool right) {
    if (left) {
        MoveToEx (hDC, r.left, r.top, NULL);
        LineTo   (hDC, r.left, r.bottom);
    };
    if (right) {
        MoveToEx (hDC, r.right - 1, r.top, NULL);
        LineTo   (hDC, r.right - 1, r.bottom);
    };
    
    MoveToEx (hDC, r.left, r.top, NULL);
    LineTo   (hDC, r.right, r.top);

    SetDCPenColor (hDC, GetSysColor (COLOR_3DHIGHLIGHT)); // white?

    if (left) {
        MoveToEx (hDC, r.left + 1, r.top + 1, NULL);
        LineTo   (hDC, r.left + 1, r.bottom);
    };
    if (right) {
        MoveToEx (hDC, r.right - 2, r.top + 1, NULL);
        LineTo   (hDC, r.right - 2, r.bottom);
    };
    
    MoveToEx (hDC, r.left + left, r.top + 1, NULL);
    LineTo   (hDC, r.right - right, r.top + 1);
    return;
};

UINT GetFullWidth (HWND hWnd) {
    HDC     hDC = GetDC (hWnd);
    HGDIOBJ hPrevious = SelectObject (hDC, GetWindowExtra (hWnd, &Data::hFont));

    Iterator iterator (hWnd, -3u / 2);
    while (iterator.next (hDC))
        ;
    
    if (hPrevious)
        SelectObject (hDC, hPrevious);
    
    ReleaseDC (hWnd, hDC);
    return iterator.r.right;
};

LRESULT OnHitTest (HWND hWnd, SHORT x, SHORT y) {
    HDC     hDC = GetDC (hWnd);
    HGDIOBJ hPrevious = SelectObject (hDC, GetWindowExtra (hWnd, &Data::hFont));
    POINT   pt = { x, y };
    LRESULT result = HTTRANSPARENT;
    
    if (ScreenToClient (hWnd, &pt)) {
        Iterator iterator (hWnd);
        
        if (pt.x < (int) iterator.width)
        while (iterator.next (hDC)) {
            if (!(iterator.type & ItemType::Separator)) {
                if (PtInRect (&iterator.r, pt)) {
                    result = HTCLIENT;
                    break;
                };
            };
        };
    };
    
    if (hPrevious)
        SelectObject (hDC, hPrevious);
    
    ReleaseDC (hWnd, hDC);
    return result;
};

LRESULT OnClick (HWND hWnd, UINT message, SHORT x, SHORT y) {
    HDC     hDC = GetDC (hWnd);
    HGDIOBJ hPrevious = SelectObject (hDC, GetWindowExtra (hWnd, &Data::hFont));
    POINT   ptMouse = { x, y };
    
    unsigned int fwidth [16u];
    unsigned int fcount = 0u;

    Iterator iterator (hWnd);
    
    if (x < (int) iterator.width)
    while (iterator.next (hDC)) {

        // TODO: NEFUNGUJE
/*        if (fcount != sizeof fwidth / sizeof fwidth [0]) {
            fwidth [fcount] = iterator.r.right;

            if (!(iterator.type & ItemType::SubItem)
             && !(iterator.type & ItemType::Separator)) {
                
                ++fcount;
            };
        };// */
        
        
        if (!(iterator.type & ItemType::Separator)) {
            if (PtInRect (&iterator.r, ptMouse)) {
                
                int code;
                bool mv = false;
                
                switch (message) {
                    default:
                    case WM_LBUTTONDOWN:    code = NM_CLICK; mv = true; break;
                    case WM_LBUTTONDBLCLK:  code = NM_DBLCLK; mv = true; break;
                    case WM_RBUTTONDOWN:    code = NM_RCLICK; break;
                    case WM_RBUTTONDBLCLK:  code = NM_RDBLCLK; break;
                };
                
                if (!SendMouseNotify (hWnd, code,
                                      iterator.i, iterator.r, ptMouse) && mv) {
                    
                    if (!(iterator.type & ItemType::SubItem)) {
                        SetWindowExtra (hWnd, &Data::current, iterator.i);
                        
                        if (iterator.r.right > (int) iterator.width) {
                            
                            // shift N times to get 'i' into full view
                            //  - N leftmost visible tabs whose width is at least width of i
                            
                            unsigned int n = 1u;
/*                            unsigned int thiswidth = iterator.r.right - iterator.r.left;

                            for (unsigned int i = 0u; i != sizeof fwidth / sizeof fwidth [0]; ++i) {
                                if (fwidth [i] < thiswidth) {
                                    ++n;
                                };
                            };*/
                            for (unsigned int i = 0u; i != n; ++i) {
                                Right (hWnd);
                            };
                        };
                        
                        SendMessage (GetParent (hWnd), WM_COMMAND,
                                     MAKEWPARAM (GetDlgCtrlID (hWnd), iterator.i),
                                     (LPARAM) hWnd);
                    };
                    
                    InvalidateRect (hWnd, NULL, TRUE);
                };
                break;
            };
        };
    };
    
    if (hPrevious)
        SelectObject (hDC, hPrevious);
    
    ReleaseDC (hWnd, hDC);
    return 0u;
};

LRESULT OnMouseMove (HWND hWnd, SHORT x, SHORT y) {
    HDC     hDC = GetDC (hWnd);
    HGDIOBJ hPrevious = SelectObject (hDC, GetWindowExtra (hWnd, &Data::hFont));
    POINT   ptMouse = { x, y };
    USHORT  hot = (USHORT) -1u;
    USHORT  pre = GetWindowExtra (hWnd, &Data::hot);
    
    Iterator iterator (hWnd);
    
    while (iterator.next (hDC)) {
        
        // check for highlighted tab
        if (PtInRect (&iterator.r, ptMouse) && x < (int) iterator.width) {
            hot = iterator.i;

            // redraw the hot tab
            //  - if it is not already hot
            //  - and if it is not current tab
            
            if (hot != pre && hot != iterator.current) {
                InvalidateRect (hWnd, &iterator.r, TRUE);
            };
        };
        
        // if hot changed, invalidate previous item
        if (iterator.i == pre && hot != pre) {
            InvalidateRect (hWnd, &iterator.r, FALSE);
        };
    };
    
    if (hPrevious)
        SelectObject (hDC, hPrevious);
    
    ReleaseDC (hWnd, hDC);
    SetWindowExtra (hWnd, &Data::hot, hot);
    
    if (!((x == -1) && (y == -1))) {
        TRACKMOUSEEVENT tme = {
            sizeof (TRACKMOUSEEVENT),
            TME_LEAVE, hWnd, HOVER_DEFAULT
        };
        TrackMouseEvent (&tme);
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
        switch (wParam) {
            case VK_LEFT:
                if (GetKeyState (VK_CONTROL) & 0x8000) {
                    Left (hWnd);
                } else {
                    Previous (hWnd);
                };
                break;
            case VK_RIGHT:
                if (GetKeyState (VK_CONTROL) & 0x8000) {
                    Right (hWnd);
                } else {
                    Next (hWnd);
                };
                break;
        };
    };
    return 0;
};

LRESULT OnHorizontalWheel (HWND hWnd, SHORT distance, USHORT) {
    distance += GetWindowExtra (hWnd, &Data::wheelDeltaAccum);
    
    while (distance >= +WHEEL_DELTA) {
        distance -= WHEEL_DELTA;
        Right (hWnd);
    };
    while (distance <= -WHEEL_DELTA) {
        distance += WHEEL_DELTA;
        Left (hWnd);
    };
    
    SetWindowExtra (hWnd, &Data::wheelDeltaAccum, distance);
    return 0;
};

void Left (HWND hWnd) {
    if (GetWindowExtra (hWnd, &Data::offset) > 0u) {
        UINT offset = GetWindowExtra (hWnd, &Data::offset);
        do {
            --offset;
        } while (offset &&
                 SendRequestNotify (hWnd, Request::Type, offset)
                                        & (ItemType::SubItem | ItemType::Separator));
        
        SetWindowExtra (hWnd, &Data::offset, offset);
        InvalidateRect (hWnd, NULL, TRUE);
        InvokeSizeHandler (hWnd);
    };
    return;
};

void Right (HWND hWnd) {
    UINT size = SendRequestNotify (hWnd, Request::Size);
    if (size && GetWindowExtra (hWnd, &Data::last) < size) {
        
        UINT offset = GetWindowExtra (hWnd, &Data::offset);
        UINT end = size - 1u;
        bool sub = false;
        
        do {
            ++offset;
            sub = SendRequestNotify (hWnd, Request::Type, offset)
                & (ItemType::SubItem | ItemType::Separator);
            
        } while (offset < end && sub);
        
        if (offset <= end && !sub) {
            SetWindowExtra (hWnd, &Data::offset, offset);
            InvokeSizeHandler (hWnd);
        };
        InvalidateRect (hWnd, NULL, TRUE);
    };
    return;
};

void Next (HWND hWnd) {
    
    USHORT current = GetWindowExtra (hWnd, &Data::current);
    USHORT offset = GetWindowExtra (hWnd, &Data::offset);
    UINT size = SendRequestNotify (hWnd, Request::Size);
    UINT end = size - 1u;
    
    if (size && current < end) {
        bool sub = false;
        
        do {
            ++current;
            sub = SendRequestNotify (hWnd, Request::Type, current)
                & (ItemType::SubItem | ItemType::Separator);
            
        } while (current < end && sub);
        
        if (current <= end && !sub) {
            SetWindowExtra (hWnd, &Data::current, current);
            SendMessage (GetParent (hWnd), WM_COMMAND,
                         MAKEWPARAM (GetDlgCtrlID (hWnd), current),
                         (LPARAM) hWnd);
        };
        
        
        while (current >= GetWindowExtra (hWnd, &Data::last)) {
            Right (hWnd);
            
            // did offset change
            //  - if yes, internally repaint to update Data::last, and repeat
            //  - if no, thus nowhere to scroll more, end the loop
            
            if (offset != GetWindowExtra (hWnd, &Data::offset)) {
                RedrawWindow (hWnd, NULL, NULL,
                              RDW_INTERNALPAINT | RDW_UPDATENOW);
            } else
                break;
        };
        
        InvalidateRect (hWnd, NULL, TRUE);
    };
    
    return;
};

void Previous (HWND hWnd) {
    USHORT current = GetWindowExtra (hWnd, &Data::current);
    USHORT offset  = GetWindowExtra (hWnd, &Data::offset);

    if (current > 0u) {
        do {
            --current;
        } while (current &&
                 SendRequestNotify (hWnd, Request::Type, current)
                                        & (ItemType::SubItem | ItemType::Separator));
        
        SetWindowExtra (hWnd, &Data::current, current);
        SendMessage (GetParent (hWnd), WM_COMMAND,
                     MAKEWPARAM (GetDlgCtrlID (hWnd), current),
                     (LPARAM) hWnd);
        
        InvalidateRect (hWnd, NULL, TRUE);
    };

    if (offset > current) {
        offset = current;
        SetWindowExtra (hWnd, &Data::offset, offset);
        InvokeSizeHandler (hWnd);
    };

    return;
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
LRESULT SendMouseNotify (HWND hWnd, UINT type,
                         UINT i, RECT & r, const POINT &ptMouse) {
    NMMOUSE nmMouse = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), type },
        i, (DWORD_PTR) &r, ptMouse, HTCAPTION
    };
    MapWindowPoints (hWnd, NULL, &nmMouse.pt, 1u);
            
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmMouse.hdr.idFrom, (LPARAM) &nmMouse);
};

LRESULT SendRequestNotify (HWND hWnd, UINT code, UINT item) {
    return SendBasicNotify (hWnd, MAKELONG (item, code));
};

UINT GetScrollButtonSize (SIZE * sz) {
    UINT cx = GetSystemMetrics (SM_CXHSCROLL /*SM_CXSIZE*/);// + 4;
    if (sz) {
        sz->cx = cx;
        sz->cy = GetSystemMetrics (SM_CYHSCROLL /*SM_CYSIZE*/) - 2;// + 4;
    };
    return cx;
};

UINT GetVisualStyle (HWND hWnd, HANDLE hTheme) {
    
    UINT style = 0x0007u & GetWindowLong (hWnd, GWL_STYLE);
    switch (style) {
        
        // custom and classic can be rendered always
        case Style::Custom:
        case Style::Classic:
            return style;

        default:
            if (hTheme) {
                // theme handle provided, check how deep
                switch (UxTheme::Current ()) {
                    
                    case UxTheme::Lite:
                        if (LOBYTE (LOWORD (GetVersion ())) < 6) {
                            // on WinXP we cannot use themed because of rounded
                            // borders of Tab Control tabs, grr
                            switch (style) {
                                case Style::Auto:
                                case Style::Theme:
                                    return Style::Frame;
                                default:
                                    return style;
                            };
                        };
                        // on Win6+ (Vista/7) fallthrough
                        
                    case UxTheme::Aero:
                        if (style == Style::Auto)
                            // automatically select best (theme) when in Aero
                            return Style::Theme;
                        else
                            // we can safely render all modes in Aero or Vista/7
                            return style;

                    default:
                    case UxTheme::Classic:
                        // theming error and/or playing safe for unknown flags
                        return Style::Classic;
                };
            } else
                // no themes supported or enabled falling to classic
                return Style::Classic;
            
    };
};

};
