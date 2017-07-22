#include "Shadow_Editor.hpp"

/* Emphasize Shadow Controls Library - Editor
// Shadow_Editor.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      06.07.2011 - initial version
*/

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cwchar>

#include "../Windows/Windows_GetSystemParameter.hpp"
#include "../Windows/Windows_W2kCompatibility.hpp"
#include "../Windows/Windows_TrackMouseEvent.hpp"
#include "../Windows/Windows_UxTheme.hpp"

#include "Shadow_Editor_Data.hpp"
#include "Shadow_Editor_Index.hpp"
#include "Shadow_Editor_Renderer.hpp"

using namespace Windows;
using namespace Shadow::Editor;

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    
    HCURSOR hHitCursor [OutsideHit] = { NULL };
};

ATOM Shadow::Editor::Initialize (HINSTANCE hInstance) {
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
        wc.lpszClassName = TEXT ("Shadow::Editor");

        HCURSOR hArrowCursor = LoadCursor (NULL, IDC_ARROW);    // borders and dragging
        HCURSOR hAlterCursor = LoadCursor (NULL, IDC_UPARROW);  // row selection
        HCURSOR hTextCursor = LoadCursor (NULL, IDC_IBEAM);     // common edit
        
        hHitCursor [CaptionHit] = hArrowCursor;
        hHitCursor [PrefixHit]  = hAlterCursor;
        hHitCursor [NumberHit]  = hAlterCursor;
        hHitCursor [PostfixHit] = hTextCursor;
        hHitCursor [TextHit]    = hTextCursor;
        
        atom = RegisterClass (&wc);
    };

    if (!Shadow::Editor::Index::Initialize (hInstance))
        return 0;
    else
        return atom;
};

HWND Shadow::Editor::Create (HINSTANCE hInstance,
                             HWND hParent, UINT style, UINT id) {
    
    return CreateWindowEx (0, (LPCTSTR) (INT) atom, TEXT(""),
                           style | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           0,0,0,0, hParent, (HMENU) id, hInstance, NULL);
};

namespace {
    
LRESULT OnCreate (HWND, const CREATESTRUCT *);
LRESULT OnDestroy (HWND);
LRESULT OnSize (HWND, WPARAM, USHORT, USHORT);
LRESULT OnPaint (HWND, HDC, RECT);
LRESULT OnChar (HWND, UINT);
LRESULT OnKeyDown (HWND, UINT);
LRESULT OnKeyUp (HWND, UINT);
LRESULT OnClick (HWND, UINT, USHORT, SHORT, SHORT);
LRESULT OnTimer (HWND, DWORD);
LRESULT OnMouseMove (HWND, SHORT, SHORT);
LRESULT OnMouseHover (HWND, SHORT, SHORT);
LRESULT OnHorizontalWheel (HWND, SHORT, USHORT);
LRESULT OnHorizontalScroll (HWND, UINT, UINT = 1u);
LRESULT OnVerticalWheel (HWND, SHORT, USHORT);
LRESULT OnVerticalScroll (HWND, UINT, UINT = 1u);
LRESULT OnCopyClearOrPaste (HWND, bool copy, bool clear, bool paste);

LRESULT SendNotify (HWND, UINT);
LRESULT SendRequest (HWND, UINT);
LRESULT SendBumpNotify (HWND, UINT);
LRESULT SendMouseNotify (HWND, UINT, NMMOUSE *);
LRESULT SendCharNotify (HWND, WPARAM, LPARAM);
LRESULT SendKeyDownNotify (HWND, WPARAM, LPARAM);

LRESULT SendFirstRowRequest (HWND, const Data *, unsigned int & fraction);
LRESULT SendRowDetailRequest (HWND, UINT, UINT, UINT = 0);
LRESULT SendNextTokenRequest (HWND, UINT, UINT, LPCTSTR &, unsigned int &);
LRESULT SendAnchorRequest (HWND, UINT, Data *);
LRESULT SendInsertRequest (HWND, UINT, UINT, LPCTSTR, UINT);
LRESULT SendInsertRequest (HWND, UINT, UINT, TCHAR);
LRESULT SendBreakRowRequest (HWND, UINT, UINT);
LRESULT SendUnBreakRequest (HWND, UINT);
LRESULT SendDeleteRequest (HWND, UINT, UINT, UINT);
LRESULT SendCopyAppendRequest (HWND, UINT, UINT, UINT, bool = false);

NMMOUSE LocateCursor (HWND, POINT, Hit, POINT * = NULL);
DWORD   LocateToken (HWND, UINT, int, RECT &, POINT *);
POINT   LocateCharacterOffset (HWND, UINT, UINT, SIZE * = NULL);
DWORD   LocateCharacter (HDC, RECT &, unsigned int, const wchar_t *, unsigned int);
LRESULT MapIndexToToken (HWND, UINT, Shadow::Editor::Mapping &);
LRESULT SetSelection (HWND, UINT, const Shadow::Editor::Selection *);

unsigned int GetRowLength (HWND, unsigned int);
unsigned int GetRowHeight (HWND, const Data *, unsigned int);
unsigned int GetScrollMultiplier (bool, unsigned int);

bool FindPrevRow (HWND, Data *, unsigned int &);
bool FindNextRow (HWND, Data *, unsigned int &);

void Recompute (HWND, HDC, Data *);
void UpdateScrollBars (HWND);
void UpdateCaret (HWND, Data *, bool hinting = true);
bool UpdateViewToCaret (HWND, Data *);
bool UpdateViewToCaretVertical (HWND, Data *);
bool UpdateViewToCaretHorizontal (HWND, Data *);
void UpdateSelectionCaret (HWND, Data *);
Hit  HitTest (HWND, POINT);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return OnCreate (hWnd, reinterpret_cast <const CREATESTRUCT *> (lParam));
        case WM_DESTROY:
            return OnDestroy (hWnd);
        case WM_SIZE:
            return OnSize (hWnd, wParam, LOWORD (lParam), HIWORD (lParam));
        case WM_TIMER:
            return OnTimer (hWnd, wParam);

        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            break;

        case WM_GETDLGCODE:
            return DLGC_HASSETSEL
                 | DLGC_WANTALLKEYS
                 | DLGC_WANTARROWS
                 | DLGC_WANTCHARS
                 | DLGC_WANTTAB;

        case WM_SETFONT:
            Data::Ref (hWnd)->recompute = true;
            Data::Ref (hWnd)->hFont = (HFONT) wParam;
            
            if (LOWORD (lParam))
                InvalidateRect (hWnd, NULL, TRUE);
            break;
        case Message::SetNumbersFont:
            SendDlgItemMessage (hWnd, 1, WM_SETFONT, wParam, lParam);
            break;

        case WM_GETFONT:
            return (LRESULT) Data::Ref (hWnd)->hFont;
        case Message::GetNumbersFont:
            return SendDlgItemMessage (hWnd, 1, WM_GETFONT, wParam, lParam);

        case Message::SetPadding:
            if (Data * data = Data::Ref (hWnd)) {
                data->margin.top = LOWORD (lParam);
                data->index.padding.left = LOWORD (wParam);
                data->index.padding.right = HIWORD (wParam);
            };
            break;
        case Message::SetThumbMinimum:
            Data::Ref (hWnd) ->thumb.minimum = (wParam < 50u) ? wParam : 50u;
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            if (HDC hDC = BeginPaint (hWnd, &ps)) {
                OnPaint (hWnd, hDC, ps.rcPaint);
            };
            EndPaint (hWnd, &ps);
            UpdateScrollBars (hWnd);
        } break;
        case WM_PRINTCLIENT: {
            RECT rc;
            if (GetClientRect (hWnd, &rc)) {
                OnPaint (hWnd, (HDC) wParam, rc);
            };
        } break;
        
        case WM_SETFOCUS:
            if (Data * data = Data::Ref (hWnd)) {
                CreateCaret (hWnd, NULL, data->caret.width, data->caret.height);
                UpdateCaret (hWnd, data);
            };
            ShowCaret (hWnd);
            return SendNotify (hWnd, NM_SETFOCUS);
        case WM_KILLFOCUS:
            DestroyCaret ();
            return SendNotify (hWnd, NM_KILLFOCUS);
        
        case WM_NOTIFY:
            if (NMHDR * nm = reinterpret_cast <NMHDR *> (lParam)) {
                
                if (nm->idFrom == 1u && nm->code == (unsigned) NM_CUSTOMDRAW)
                    reinterpret_cast <NMCUSTOMDRAW *>
                                     (nm) ->dwDrawStage |= Drawing::Index;
                
                nm->hwndFrom = hWnd;
                nm->idFrom = GetDlgCtrlID (hWnd);
                
                return SendMessage (GetParent (hWnd),
                                    WM_NOTIFY, nm->idFrom, lParam);
            };
            break;
        
        case WM_MOUSEMOVE:
            return OnMouseMove (hWnd, LOWORD (lParam), HIWORD (lParam));
        case WM_MOUSELEAVE:
            return OnMouseMove (hWnd, -1, -1);
        case WM_MOUSEHOVER:
            return OnMouseHover (hWnd, LOWORD (lParam), HIWORD (lParam));
        
        case WM_VSCROLL:
            SetFocus (hWnd);
            return OnVerticalScroll (hWnd, LOWORD (wParam));
        case WM_HSCROLL:
            SetFocus (hWnd);
            return OnHorizontalScroll (hWnd, LOWORD (wParam));
        case WM_MOUSEWHEEL:
            return OnVerticalWheel (hWnd, HIWORD (wParam), LOWORD (wParam));
        case WM_MOUSEHWHEEL:
            return OnHorizontalWheel (hWnd, HIWORD (wParam), LOWORD (wParam));
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
            return OnClick (hWnd, message, HIWORD (wParam),
                            LOWORD (lParam), HIWORD (lParam));

        case WM_SYSKEYDOWN:
            Procedure (hWnd, WM_KEYDOWN, wParam, lParam);
            return DefWindowProc (hWnd, message, wParam, lParam);
        case WM_KEYDOWN:
            if (!SendKeyDownNotify (hWnd, wParam, lParam))
                return OnKeyDown (hWnd, wParam);
            else
                return 0;
                
        case WM_SYSKEYUP:
            Procedure (hWnd, WM_KEYUP, wParam, lParam);
            return DefWindowProc (hWnd, message, wParam, lParam);
        case WM_KEYUP:
            return OnKeyUp (hWnd, wParam);
        case WM_CHAR:
            if (!SendCharNotify (hWnd, wParam, lParam))
                return OnChar (hWnd, wParam);
            else
                return 0;
        
        // Typical editor control operations
        
        case Message::Jump:
            if (Data * data = Data::Ref (hWnd)) {
                data->caret.row = wParam;
                data->caret.column = lParam;
                
                UpdateCaret (hWnd, data);
                UpdateViewToCaret (hWnd, data);
            };
            break;
        case Message::View:
            UpdateViewToCaret (hWnd, Data::Ref (hWnd));
            break;
        
        case Message::Map:
            return MapIndexToToken (hWnd, wParam,
                                    *reinterpret_cast <Mapping *> (lParam));
        case Message::Select:
            return SetSelection (hWnd, wParam,
                                 reinterpret_cast <const Selection *> (lParam));
        
        case EM_UNDO:
        case WM_UNDO:
            SendRequest (hWnd, Request::Undo);
            break;
        case EM_REDO:
            SendRequest (hWnd, Request::Redo);
            break;
        case EM_SETSEL:
            if (wParam == 0u && lParam == -1)
                return SetSelection (hWnd, 1, NULL);
            else
            if (wParam == -1u)
                return SetSelection (hWnd, 0, NULL);
            else
                // TODO: translate linear offsets to column/row here
                return FALSE;
        case EM_CANPASTE:
            switch (wParam) {
                case CF_TEXT:
                case CF_OEMTEXT:
                case CF_UNICODETEXT:
                    return TRUE;
                default:
                    return FALSE;
            };
        case EM_GETLINECOUNT:
            return SendRequest (hWnd, Request::Rows);
        
        case WM_PASTE:
            return OnCopyClearOrPaste (hWnd, false, true, true);
        case WM_CLEAR:
            return OnCopyClearOrPaste (hWnd, false, true, false);
        case WM_COPY:
            return OnCopyClearOrPaste (hWnd, true, false, false);
        case WM_CUT:
            return OnCopyClearOrPaste (hWnd, true, true, false);
        
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT OnCreate (HWND hWnd, const CREATESTRUCT * create) {
    LONG_PTR ptr = Shadow::Editor::Data::New ();
    if (ptr)
        SetWindowLongPtr (hWnd, 0, Shadow::Editor::Data::New ());
    else
        return -1;
    
    // Further settings

    Data * data = Data::Ref (hWnd);
    if ((create->style & WS_HSCROLL)
            && !(create->style & Style::AlwaysHorzScrollBar)) {
        data->autoscrollbar = true;
    };
    
    // Index window
    //  - renders appropriately row numbers and hides some artifacts
    
    UINT style = WS_VISIBLE;
    if (create->style & Shadow::Editor::Style::ZerosPrefixIndex)
        style |= Shadow::Editor::Index::Style::ZerosPrefix;
    
    if (Shadow::Editor::Index::Create (create->hInstance, hWnd, style, 1u)) {
        SendDlgItemMessage (hWnd, 1, Index::Message::SetNumbersCount,
                                     data->index.minimum, 0);
        return 0;
    } else
        return -1;
};

LRESULT OnDestroy (HWND hWnd) {
    Shadow::Editor::Data::Delete (hWnd);
    return 0;
};

LRESULT OnSize (HWND hWnd, WPARAM wParam, USHORT, USHORT height) {
    if (wParam != SIZE_MINIMIZED) {
        const Data * data = Data::Ref (hWnd);
        MoveWindow (GetDlgItem (hWnd, 1), 0, data->margin.top,
                    data->margin.left, height - data->margin.top, FALSE);
    };
    return 0;
};

LRESULT OnPaint (HWND hWnd, HDC _hDC, RECT rcClip) {
    RECT rc;
    GetClientRect (hWnd, &rc);
    
    Data * const data = Data::Ref (hWnd);

    HDC hDC = NULL;
    HANDLE hBuffered = UxTheme::BeginBufferedPaint (_hDC, &rc, &hDC);

    if (!hBuffered)
        hDC = _hDC;
    
    if (data->recompute) {
        data->recompute = false;
        Recompute (hWnd, hDC, data);
        MoveWindow (GetDlgItem (hWnd, 1), 0, data->margin.top,
                    data->margin.left, rc.bottom - data->margin.top, FALSE);
    };
    
    // Prepare details for further rendering

    unsigned int fraction = 0u;
    unsigned int row = SendFirstRowRequest (hWnd, data, fraction);

    SendDlgItemMessage (hWnd, 1, Index::Message::SetOffset, row, fraction);

    POINT sp [2];
    
    if (data->selection.type) {
        sp [0] = LocateCharacterOffset (hWnd, data->selection.row,
                                              data->selection.column);
        sp [1] = LocateCharacterOffset (hWnd, data->caret.row,
                                              data->caret.column);
    };
    
    Shadow::Editor::Renderer (hWnd, hDC, rc, rcClip,
                              row, fraction, sp);

    if (hBuffered)
        UxTheme::EndBufferedPaint (hBuffered);

    if (data->reposition) {
        data->reposition = false;
        UpdateCaret (hWnd, data, false);
    };
    return 0;
};

LRESULT OnChar (HWND hWnd, UINT c) {
    if (c < 32)
        return 0;

    Data * const data = Data::Ref (hWnd);
    
    // make sure the caret is visible
    
    UpdateViewToCaret (hWnd, data);
    
    // first request to insert the character
    //  - truncating 'col' on the end of row if neccessary
    
    unsigned int row = data->caret.row;
    unsigned int col = data->caret.column;
    unsigned int max = GetRowLength (hWnd, row);
    
    if (col > max)
        col = max;
    
    // delete any selection
    //  - typical overwritting selected text

    if (data->selection.type != Data::Selection::None) {
        OnKeyDown (hWnd, VK_DELETE);
    };
    
    // callback
    //  - TODO: callback returns how far to move the caret!!!

    if (SendInsertRequest (hWnd, row, col, c)) {
        
        // invalidate the affected row
        
        RECT rc;
        POINT caret;
        if (GetClientRect (hWnd, &rc) && GetCaretPos (&caret)) {
            RECT rRow = {
                0, caret.y,
                rc.right, caret.y + (long) GetRowHeight (hWnd, data, row)
            };
            InvalidateRect (hWnd, &rRow, FALSE);
        };
        
        // if overwritting mode, erase next character
        //  - simply simulating DELETE key
        //  - we also don't want the cursor to skip to next row
        
        if (data->overwritting && col < max) {
            OnKeyDown (hWnd, VK_DELETE);
        };
        
        // move the caret
        //  - if the caret is ouside the view, move the view, otherwise
        //    invalidate the current row
        //  - set selection column; faster than calling UpdateSelectionCaret
        
        data->caret.column = col + 1;
        data->selection.column = data->caret.column;
        UpdateCaret (hWnd, data);
    };
    
    // If mouse vanish feature is enabled, hide mouse
    //  - vanish mouse only if in text area (client without margins)
    
    if (!data->vanished
            && Windows::GetSystemParameter <SPI_GETMOUSEVANISH> ())  {

        POINT pt;
        RECT rc;
        if (GetCursorPos (&pt) && GetWindowRect (hWnd, &rc)) {

            rc.left += data->margin.left;
            rc.top += data->margin.top;
            
            if (PtInRect (&rc, pt)) {
                data->vanished = true;
                ShowCursor (false);
            };
        };
    };
    
    return 0;
};

LRESULT OnKeyUp (HWND hWnd, UINT vk) {
    Data * const data = Data::Ref (hWnd);
    const bool shift = GetKeyState (VK_SHIFT) & 0x8000;

    switch (vk) {
        
        // ALT
        //  - releasing ALT key while selecting means selection type
        //    change from Column to Normal
        
        case VK_MENU:
            if (shift || (GetKeyState (VK_LBUTTON) & 0x8000))
                if (data->selection.type == Data::Selection::Column) {
                    data->selection.type = Data::Selection::Normal;
                    InvalidateRect (hWnd, NULL, FALSE);
                };
            break;
    };
    
    return 0;
};

bool OnKeyDownLeft (HWND, Data *);
bool OnKeyDownRight (HWND, Data *);
bool OnKeyDownHome (HWND, Data *, bool ctrl);
bool OnKeyDownEnd (HWND, Data *, bool ctrl);
bool OnKeyDownPageUp (HWND, Data *, bool ctrl);
bool OnKeyDownPageDn (HWND, Data *, bool ctrl);

LRESULT OnKeyDown (HWND hWnd, UINT vk) {
    Data * const data = Data::Ref (hWnd);
    
    const bool shift = GetKeyState (VK_SHIFT) & 0x8000;
    const bool ctrl = GetKeyState (VK_CONTROL) & 0x8000;
    const bool alt = GetKeyState (VK_MENU) & 0x8000;
    
    switch (vk) {
        
        // ALT
        //  - pressing ALT key while selecting means selection type
        //    change from Normal to Column

        case VK_MENU:
            if (shift || (GetKeyState (VK_LBUTTON) & 0x8000))
                if (data->selection.type == Data::Selection::Normal) {
                    data->selection.type = Data::Selection::Column;
                    InvalidateRect (hWnd, NULL, FALSE);
                };
            break;
        
        // Backspace
        //  - selections are just deleted
        
        case VK_BACK:
            if (alt) {
                if (ctrl) {
                    if (shift) {
                        
                    } else {
                        
                    };
                } else {
                    // Undo
                    if (shift) {
                        Procedure (hWnd, EM_REDO, 0, 0);
                    } else {
                        Procedure (hWnd, EM_UNDO, 0, 0);
                    };
                };
            } else {
                if (data->selection.type != Data::Selection::None) {
                    return OnKeyDown (hWnd, VK_DELETE);
                    
                } else {
                    if (ctrl) {
                        if (shift) {
                            
                        } else {
                            // delete token to the left
                        };
                    } else {
                        if (shift) {
                            
                        } else {
                            if (data->caret.column) {
                                data->caret.column -= 1u;
                                SendDeleteRequest (hWnd, data->caret.row,
                                                   data->caret.column, 1u);
                            } else
                            if (data->caret.row) {
                                data->caret.row -= 1u;
                                data->caret.column = GetRowLength (hWnd,
                                                                   data->caret.row);
                                SendUnBreakRequest (hWnd, data->caret.row);
                            };
                            
                            UpdateCaret (hWnd, data);
                            UpdateViewToCaret (hWnd, data);
                            InvalidateRect (hWnd, NULL, FALSE);
                        };
                    };
                };
            };
            break;
        
        case VK_DELETE:
            if (data->selection.type != Data::Selection::None) {
                if (ctrl) {
                    
                } else {
                    if (shift) {
                        Procedure (hWnd, WM_CUT, 0, 0);
                    } else {
                        Procedure (hWnd, WM_CLEAR, 0, 0);
                    };
                };
            } else {
                if (shift) {

                } else {
                    if (ctrl) {
                        
                        // TODO: delete rest of the token to the right
                        
                    } else {
                        
                        // DELete key is emulated
                        //  - move one to right and delete with backspace
                        
                        if (OnKeyDownRight (hWnd, data)) {
                            OnKeyDown (hWnd, VK_BACK);
                        };
                    };
                };
            };
            break;
            
        case VK_INSERT:
            if (!alt) {
                if (data->selection.type != Data::Selection::None) {
                    Procedure (hWnd, WM_CLEAR, 0, 0);
                };
                if (ctrl) {
                    if (shift) {
                        
                    } else {
                        Procedure (hWnd, WM_COPY, 0, 0);
                    };
                } else {
                    if (shift) {
                        Procedure (hWnd, WM_PASTE, 0, 0);
                    } else {
                        data->overwritting = !data->overwritting;
                        UpdateCaret (hWnd, data);
                    };
                };
            };
            break;

        case VK_RETURN:
            if (!alt && !ctrl) {
                if (data->selection.type != Data::Selection::None) {
                    OnKeyDown (hWnd, VK_DELETE);
                };
                
                const unsigned int length = GetRowLength (hWnd, data->caret.row);
                if (data->caret.column > length)
                    data->caret.column = length;
                
                if (SendBreakRowRequest (hWnd, data->caret.row, data->caret.column)) {
                    ++data->caret.row;
                    data->caret.column = 0u;
                
                    UpdateCaret (hWnd, data);
                    UpdateViewToCaret (hWnd, data);
                    InvalidateRect (hWnd, NULL, FALSE);
                };
            };
            break;
        
        // Clear/5
        //  - numeric 5 when NumLock is OFF
        //  - simulating left mouse button click
        //  - TODO: right/middle/double-click (CTRL+ALT+SHIFT???)
        
        case VK_CLEAR: {
            POINT caret;
            if (GetCaretPos (&caret)) {
                OnClick (hWnd, WM_LBUTTONUP, 0u, caret.x, caret.y);
            };
        } break;

        // Keys that update caret position
        //  - note that we also ensure that caret is visible
        
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            if (alt && !shift) {
                if (ctrl) {
                    // CTRL+ALT+keys
                } else {
                    // ALT+keys
                };
            } else {
                if (ctrl) {
                    if (SendAnchorRequest (hWnd, vk, data)) {
                        UpdateViewToCaret (hWnd, data);
                        UpdateSelectionCaret (hWnd, data);
                    };
                } else { 
                    bool moved = false;
                    unsigned int row = data->caret.row;
                    
                    switch (vk) {
                        case VK_UP:
                            if ((moved = FindPrevRow (hWnd, data, row))) {
                                RECT r = { 0, 0, 0x7FFFFFFF, 0 };
                                POINT pt;
                                LocateToken (hWnd, row, data->caret.offset, r, &pt);
                                
                                data->caret.column = pt.x;
                                data->caret.row = row;
                            };
                            break;
                        case VK_DOWN:
                            if ((moved = FindNextRow (hWnd, data, row))) {

                                RECT r = { 0, 0, 0x7FFFFFFF, 0 };
                                POINT pt;
                                LocateToken (hWnd, row, data->caret.offset, r, &pt);

                                data->caret.column = pt.x;
                                data->caret.row = row;
                            };
                            break;
                        case VK_LEFT:
                            moved = OnKeyDownLeft (hWnd, data);
                            break;
                        case VK_RIGHT:
                            moved = OnKeyDownRight (hWnd, data);
                            break;
                    };
                    
                    if (moved) {
                        UpdateCaret (hWnd, data, (vk != VK_UP) && (vk != VK_DOWN));
                    } else {
                        SendBumpNotify (hWnd, vk);
                    };
                    
                    UpdateViewToCaret (hWnd, data);
                    UpdateSelectionCaret (hWnd, data);
                };
            };
            break;

        // Broad navigation

        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
            if (alt && !shift) {
                if (ctrl) {
                    // CTRL+ALT+keys
                } else {
                    // ALT+keys
                };
            } else {
                switch (vk) {
                    case VK_HOME:
                        OnKeyDownHome (hWnd, data, ctrl);
                        break;
                    case VK_END:
                        OnKeyDownEnd (hWnd, data, ctrl);
                        break;
                    case VK_PRIOR:
                        OnKeyDownPageUp (hWnd, data, ctrl);
                        break;
                    case VK_NEXT:
                        OnKeyDownPageDn (hWnd, data, ctrl);
                        break;
                };
            };
            break;
    };
    
    // CTRL+key
    
    if (ctrl && !shift && !alt)
        switch (vk) {
            case 'A': Procedure (hWnd, EM_SETSEL, 0, -1); break;
            case 'Z': Procedure (hWnd, EM_UNDO, 0, 0); break;
            case 'Y': Procedure (hWnd, EM_UNDO, 0, 0); break;
            case 'X': Procedure (hWnd, WM_CUT, 0, 0); break;
            case 'C': Procedure (hWnd, WM_COPY, 0, 0); break;
            case 'V': Procedure (hWnd, WM_PASTE, 0, 0); break;
        };

    // CTRL+SHIFT+key

    if (ctrl && shift && !alt)
        switch (vk) {
            case 'Z': Procedure (hWnd, EM_REDO, 0, 0); break;
            case 'Y': Procedure (hWnd, EM_REDO, 0, 0); break;
        };
    
    
    return 0;
};

bool OnKeyDownLeft (HWND hWnd, Data * data) {
    if (data->caret.column) {
        unsigned int length = GetRowLength (hWnd, data->caret.row);
        if (data->caret.column > length) {
            data->caret.column = length;
        };

        --data->caret.column;
        return true;
        
    } else {
        unsigned int row = data->caret.row;
        if (FindPrevRow (hWnd, data, row)) {
            data->caret.row = row;
            data->caret.column = GetRowLength (hWnd, row);
            return true;
            
        } else
            return false;
    };
};

bool OnKeyDownRight (HWND hWnd, Data * data) {
    unsigned int length = GetRowLength (hWnd, data->caret.row);
    if (data->caret.column < length) {
        
        ++data->caret.column;
        return true;
        
    } else {
        unsigned int row = data->caret.row;
        if (FindNextRow (hWnd, data, row)) {
            data->caret.row = row;
            data->caret.column = 0u;
            return true;
    
        } else
            return false;
    };
};

bool OnKeyDownHome (HWND hWnd, Data * data, bool ctrl) {
    OnHorizontalScroll (hWnd, SB_LEFT);
    data->caret.column = 0u;

    if (ctrl) {
        unsigned int row = -1u;
        if (FindNextRow (hWnd, data, row)) {
            data->caret.row = row;
            OnVerticalScroll (hWnd, SB_TOP);
        };
    } else {
        UpdateViewToCaretVertical (hWnd, data);
    };

    UpdateCaret (hWnd, data);
    UpdateSelectionCaret (hWnd, data);
    return true;
};
bool OnKeyDownEnd (HWND hWnd, Data * data, bool ctrl) {
    if (ctrl) {
        unsigned int row = SendRequest (hWnd, Request::Rows);
        if (FindPrevRow (hWnd, data, row)) {
            data->caret.row = row;
            data->caret.column = GetRowLength (hWnd, row);

            OnVerticalScroll (hWnd, SB_BOTTOM);
            UpdateCaret (hWnd, data);
        };
    } else {
        data->caret.column = GetRowLength (hWnd, data->caret.row);
        UpdateCaret (hWnd, data);
    };

    UpdateViewToCaret (hWnd, data);
    UpdateSelectionCaret (hWnd, data);
    return true;
};

bool OnKeyDownPageUp (HWND hWnd, Data * data, bool ctrl) {
    if (ctrl) {
        unsigned int fraction = 0u;
        unsigned int row = SendFirstRowRequest (hWnd, data, fraction);

        if (fraction) {
            unsigned int next = row;
            if (FindNextRow (hWnd, data, next)) {
                row = next;
            };
        };

        data->caret.row = row;

        UpdateCaret (hWnd, data);
        UpdateViewToCaret (hWnd, data);

    } else {
        RECT rc;
        if (GetClientRect (hWnd, &rc)) {

            rc.bottom -= data->margin.top;
            rc.bottom -= 2 * data->character.cy;

            unsigned int row = data->caret.row;
            if (row > 0) {

                int sum = 0;
                do {
                    sum += GetRowHeight (hWnd, data, row);
                    --row;
                } while (row > 0u && sum < rc.bottom);

                data->caret.row = row;

                UpdateCaret (hWnd, data);
                UpdateViewToCaret (hWnd, data);
            };
        };
    };
    UpdateSelectionCaret (hWnd, data);
    return true;
};

bool OnKeyDownPageDn (HWND hWnd, Data * data, bool ctrl) {
    RECT rc;
    POINT caret;

    if (GetClientRect (hWnd, &rc) && GetCaretPos (&caret)) {

        rc.bottom -= data->margin.top;
        rc.bottom -= data->character.cy;

        const unsigned int rows = SendRequest (hWnd, Request::Rows);

        if (ctrl) {
            unsigned int fra = 0u;
            unsigned int row = SendFirstRowRequest (hWnd, data, fra);
            unsigned int height = 0u;

            rc.bottom += fra;

            while (row < rows) {
                height += GetRowHeight (hWnd, data, row++);
                if (height > (unsigned) rc.bottom) {

                    if (FindPrevRow (hWnd, data, row)) {
                        data->caret.row = row;
                        UpdateCaret (hWnd, data);
                    };

                    UpdateViewToCaretHorizontal (hWnd, data);
                    break;
                };
            };
        } else {
            unsigned int row = data->caret.row;
            if (row + 1u < rows) {

                int sum = 0;
                do {
                    sum += GetRowHeight (hWnd, data, row);
                    ++row;
                } while (row < rows && sum < rc.bottom);

                if (FindPrevRow (hWnd, data, row)) {
                    data->caret.row = row;
                    UpdateCaret (hWnd, data);
                };
                UpdateViewToCaret (hWnd, data);
            };
        };

        UpdateSelectionCaret (hWnd, data);
        return true;
    } else
        return false;
};

const wchar_t * wcs_endline (const wchar_t * string) {
    while (*string
               && *string != L'\n'
               && *string != L'\r') {
        ++string;
    };
    return string;
};

// OnCopyClearOrPaste
//  - implemented together because all operations traverse the data in same way
//  - paste is here for column selection type insertion

LRESULT OnCopyClearOrPaste (HWND hWnd, bool copy, bool clear, bool paste) {
    if (copy && paste)
        return 0;
    
    HANDLE hPaste = NULL;
    const wchar_t * insertion = NULL;
    
    if (paste) {
        if (OpenClipboard (hWnd)) {
            if ((hPaste = GetClipboardData (CF_UNICODETEXT))) {
                insertion = static_cast <const wchar_t *> (GlobalLock (hPaste));
            } else {
                CloseClipboard ();
                return 0;
            };
        } else
            return 0;
    };
    
    Data * const data = Data::Ref (hWnd);
    if ((data->selection.type && (copy || clear)) || paste) {
        
        // notify user if copying
        //  - if user doesn't confirm, cancel the whole operation
        
        if (copy && !SendNotify (hWnd, Request::CopyBegin))
            return 0;
        
        // selection type
        //  - fixed for simple single row operation
        //  - copied because we don't want to modify it
        
        Data::Selection::Type type = data->selection.type;

        if (data->selection.row == data->caret.row) {
            if (type == Data::Selection::Column) {
                type = Data::Selection::Normal;
            };
        };
        
        switch (type) {
            case Data::Selection::Normal:
                if (data->selection.row == data->caret.row) {
                    
                    unsigned int column;
                    unsigned int length;
                    
                    if (data->caret.column < data->selection.column) {
                        column = data->caret.column;
                        length = data->selection.column - data->caret.column;
                    } else {
                        column = data->selection.column;
                        length = data->caret.column - data->selection.column;
                    };
                    
                    if (copy)
                        SendCopyAppendRequest (hWnd, data->caret.row, column, length);
                    if (clear)
                        if (SendDeleteRequest (hWnd, data->caret.row, column, length)) {
                            data->caret.column = column;
                        };
                } else {
                    
                    unsigned int last;
                    unsigned int last_column;
                    unsigned int first;
                    unsigned int first_column;
                    
                    if (data->selection.row < data->caret.row) {
                        last = data->caret.row;
                        last_column = data->caret.column;
                        first = data->selection.row;
                        first_column = data->selection.column;
                    } else {
                        last = data->selection.row;
                        last_column = data->selection.column;
                        first = data->caret.row;
                        first_column = data->caret.column;
                    };
                    
                    if (copy) {
                        SendCopyAppendRequest (hWnd, first, first_column, -1u, true);
                        
                        for (unsigned int row = first + 1u; row < last; ++row)
                            SendCopyAppendRequest (hWnd, row, 0, -1u, true);
                        
                        if (last_column)
                            SendCopyAppendRequest (hWnd, last, 0, last_column);
                    };
                    
                    if (clear) {
                        SendDeleteRequest (hWnd, last, 0, last_column);
                        
                        for (unsigned int row = last - 1u; row > first; --row)
                            SendDeleteRequest (hWnd, row, 0u, -1u);
                    
                        // finally remove the trail of the first row
                        //  - and unbreak the remaining new-line
                    
                        SendDeleteRequest (hWnd, first, first_column,
                                           GetRowLength (hWnd, first) - first_column);
                        
                        if (SendUnBreakRequest (hWnd, first)) {
                            data->caret.row = first;
                            data->caret.column = first_column;
                        };
                    };
                };
    
                if (clear) {
                    data->selection.type = Data::Selection::None;

                    UpdateCaret (hWnd, data);
                    UpdateViewToCaret (hWnd, data);
                    InvalidateRect (hWnd, NULL, FALSE);
                };
                if (!paste)
                    break;
                
            case Data::Selection::None:
                if (insertion) {
                    unsigned int r = data->caret.row;
                    unsigned int c = data->caret.column;
                    const wchar_t * end = NULL;
                    
                    do {
                        end = wcs_endline (insertion);
                        unsigned int length = end - insertion;
                        
                        if (SendInsertRequest (hWnd, r, c, insertion, length)) {
                            if (*end) {
                                if (SendBreakRowRequest (hWnd, r, c + length)) {
                                    ++r;
                                    c = 0u;
                                };
                            } else {
                                c += length;
                            };
                        } else
                            break;
                        
                        if (end[0] == L'\r' && end[1] == L'\n')
                            insertion = end + 2u;
                        else
                            insertion = end + 1u;
                        
                    } while (*end);
                    
                    Data::Ref (hWnd) ->caret.column = c;
                    Data::Ref (hWnd) ->caret.row = r;
                    
                    UpdateCaret (hWnd, data);
                    UpdateViewToCaret (hWnd, data);
                    InvalidateRect (hWnd, NULL, FALSE);
                };
                break;
            
            case Data::Selection::Column: {
                const POINT sp [2] = {
                    LocateCharacterOffset (hWnd, data->selection.row,
                                                 data->selection.column),
                    LocateCharacterOffset (hWnd, data->caret.row,
                                                 data->caret.column)
                };
                const unsigned int left  = std::min (sp[0].x, sp[1].x);
                const unsigned int right = std::max (sp[0].x, sp[1].x);
                const unsigned int first = std::min (data->selection.row, data->caret.row);
                const unsigned int last  = std::max (data->selection.row, data->caret.row);
                
                RECT r;
                POINT pt [2];
                
                for (unsigned int row = first; row <= last; ++row) {
                    
                    r.left = 0;
                    r.right = 0x7FFFFFFF;
                    LocateToken (hWnd, row, left, r, &pt[0]);
                    
                    r.left = 0;
                    r.right = 0x7FFFFFFF;
                    LocateToken (hWnd, row, right, r, &pt[1]);
                    
                    if (copy) {
                        SendCopyAppendRequest (hWnd, row, pt[0].x, pt[1].x - pt[0].x, true);
                    };
                    if (clear) {
                        if (pt[1].x != pt[0].x) {
                            SendDeleteRequest (hWnd, row, pt[0].x, pt[1].x - pt[0].x);
                        };
                    };
                    if (insertion) {
                        const wchar_t * end = wcs_endline (insertion);
                        const unsigned int length = end - insertion;
    
                        if (!SendInsertRequest (hWnd, row, pt[0].x, insertion, length))
                            break;
                        
                        if (*end) {
                            if (end[0] == L'\r' && end[1] == L'\n')
                                insertion = end + 2u;
                            else
                                insertion = end + 1u;
                        } else
                            insertion = NULL;
                    };
                };
                
                if (clear) {
                    data->selection.type = Data::Selection::None;
                    
                    if (data->caret.column < data->selection.column)
                        data->selection.column = data->caret.column;
                    else
                        data->caret.column = data->selection.column;
                    
                    UpdateCaret (hWnd, data);
                    UpdateViewToCaret (hWnd, data);
                    UpdateSelectionCaret (hWnd, data);
                    InvalidateRect (hWnd, NULL, FALSE);
                };
            } break;
        };

        if (copy) {
            SendNotify (hWnd, Request::CopyCommit);
        };
    };

    if (hPaste) {
        GlobalUnlock (hPaste);
        CloseClipboard ();
    };
    return 0;
};

UINT OnClickNotifyCode (UINT message, WPARAM wParam) {

    // Convert WM_ mouse message to NM_ code
    //  - fastest using table (single movsbl instruction)
    //  - all NM_ codes fit into single signed byte thus signed char

    static const signed char table [] = {
        NM_LDOWN,           // 0x0201: WM_LBUTTONDOWN
        NM_CLICK,           // 0x0202: WM_LBUTTONUP
        NM_DBLCLK,          // 0x0203: WM_LBUTTONDBLCLK
        NM_RDOWN,           // 0x0204: WM_RBUTTONDOWN
        NM_RCLICK,          // 0x0205: WM_RBUTTONUP
        NM_RDBLCLK,         // 0x0206: WM_RBUTTONDBLCLK
        Notify::MDown,      // 0x0207: WM_MBUTTONDOWN
        Notify::MClick,     // 0x0208: WM_MBUTTONUP
        Notify::MDblClk,    // 0x0209: WM_MBUTTONDBLCLK
        0u,                 // 0x020A: irrelevant WM_MOUSEWHEEL
        Notify::X1Down,     // 0x020B: WM_XBUTTONDOWN
        Notify::X1Click,    // 0x020C: WM_XBUTTONUP
        Notify::X1DblClk    // 0x020D: WM_XBUTTONDBLCLK
    };

    UINT code = table [message - WM_LBUTTONDOWN];

    switch (message) {

        // special path for X buttons
        //  - wParam (HIWORD actually) is bitmaps, where only single bit is set,
        //    which determines which button generated this messages
        //  - if no bit is set, then this is fubar, just return X1

        case WM_XBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
            if (wParam) {
                while (!(wParam & 1u)) {
                    wParam >>= 1u;
                    ++code;
                };
                break;
            };
            break;
    };
    
    return code;
};

LRESULT OnClick (HWND hWnd, UINT message, USHORT wParam, SHORT x, SHORT y) {
    UINT code = OnClickNotifyCode (message, wParam);
    bool move = false;
    
    switch (message) {
        
        // Left and Right buttons
        //  - clicks generate request to move cursor
        //  - capture starts timer that scrolls if mouse leaves the window
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            move = true;
            break;
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            KillTimer (hWnd, 1u);
            ReleaseCapture ();
            break;
    };
    
    // Callback
    //  - returning non-zero from callback cancels default behavior
    
    POINT caret;
    POINT pt = { x, y };
    NMMOUSE nm = LocateCursor (hWnd, pt, HitTest (hWnd, pt),
                               move ? &caret : NULL);
    
    if (!SendMouseNotify (hWnd, code, &nm)) {
        switch (nm.dwHitInfo) {
            
            case PrefixHit:
            case NumberHit:
                break;
            
            case PostfixHit:
            case TextHit: {
                Data * const data = Data::Ref (hWnd);
                switch (message) {
                    case WM_LBUTTONDOWN:
                        if (data->selection.type) {
                            data->selection.type = Data::Selection::None;
                            InvalidateRect (hWnd, NULL, FALSE);
                        };
        
                    case WM_RBUTTONDOWN:
                        
                        // update caret position
                        //  - selection offset too (selection remains on drag)
                        
                        data->selection.column = caret.x;
                        data->caret.column = caret.x;
                        
                        data->selection.row = caret.y;
                        data->caret.row = caret.y;
                        
                        UpdateCaret (hWnd, data);
                        
                    case WM_MBUTTONDOWN:
                        SetFocus (hWnd);
                };
            };
            
            default:
                if (move) {
                    SetCapture (hWnd);
                    SetTimer (hWnd, 1u,
                              std::max (10u, GetDoubleClickTime () / 8u), NULL);
                };
        };
    };

    return 0;
};

LRESULT OnMouseMove (HWND hWnd, SHORT x, SHORT y) {
    POINT caret;
    POINT pt = { x, y };
    Hit hit = HitTest (hWnd, pt);
    
    // show mouse cursor if hidden due to vanish feature

    Data * const data = Data::Ref (hWnd);
    if (data->vanished)  {
        data->vanished = false;
        ShowCursor (true);
    };
    
    // tracking selection
    //  - the only use of SetCapture is to drag selection
    
    bool tracking = (GetCapture () == hWnd);
    if (tracking) {
        hit = TextHit;
    };
    
    NMMOUSE nmMouse = LocateCursor (hWnd, pt, hit,
                                    tracking ? &caret : NULL);
    switch (hit) {
        case TextHit:
            if (tracking) {
                if (data->caret.column != (unsigned int) caret.x
                    || data->caret.row != (unsigned int) caret.y) {
                    
                    data->caret.row = caret.y;
                    data->caret.column = caret.x;

                    UpdateSelectionCaret (hWnd, data);
                    UpdateCaret (hWnd, data);
                };
            };
        
        case CaptionHit:
        case PrefixHit:
        case NumberHit:
        case PostfixHit:
            if (!SendMouseNotify (hWnd, NM_SETCURSOR, &nmMouse)) {
                SetCursor (hHitCursor [hit]);
            };
            Windows::TrackMouseEvent (hWnd, TME_LEAVE | TME_HOVER);
            break;
            
        case OutsideHit:
            Windows::TrackMouseEvent (hWnd, TME_HOVER | TME_CANCEL);
            break;
    };
    
    return 0;
};

LRESULT OnMouseHover (HWND hWnd, SHORT x, SHORT y) {
    POINT pt = { x, y };
    NMMOUSE nmMouse = LocateCursor (hWnd, pt, HitTest (hWnd, pt));
    
    SendMouseNotify (hWnd, NM_HOVER, &nmMouse);
    return 0;
};

LRESULT OnHorizontalScroll (HWND hWnd, UINT event, UINT n) {
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_TRACKPOS | SIF_RANGE | SIF_PAGE | SIF_POS;

    if (GetScrollInfo (hWnd, SB_HORZ, &si)) {
        
        const Data * const data = Data::Ref (hWnd);
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
                si.nPos -= n * data->character.cx;
                break;
            case SB_LINERIGHT:
                si.nPos += n * data->character.cx;
                break;
            
            case SB_THUMBTRACK:
                if (si.nPos == si.nTrackPos) {
                    // si.nPos = GetScrollPos (hWnd, SB_HORZ);
                    
                    // TODO: cele tohle zpicene soupani koleckem mysi udelat
                    //       tak, ze na MOUSEHSCROLL nastavim timer
                    //       (cas nevim) a prijde-li WM_HSCROLL s SB_THUMBTRACK
                    //       tak budu MOUSEHSCROLL ignorovat, jinak posunu
                    
                    // Protoze logitech setpoint posila i WM_HSCROLL
                    
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
                si.nPos += (SHORT) HIWORD (event);
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
        
        // Fractional scrolling
        //  - if not enabled, rounding position (and thumb width) to integer
        //    multiplier of default font character width
        //  - TODO: remove because of mouses, and make wrapper for
        //          GetScrollPos (hWnd, SB_HORZ) that will round the result
        //          where neccessary
        
        if (!(GetWindowLongPtr (hWnd, GWL_STYLE) & Style::FractionalScrolling)) {
            si.nPos /= data->character.cx;
            si.nPos *= data->character.cx;
        };
        
        // Update scrollbar position

        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_HORZ, &si, TRUE);
        
        // Scrolling window content
        
        RECT r;
        if (GetClientRect (hWnd, &r)) {
            
            r.top = data->margin.top;
            r.left = data->margin.left;
            
            ScrollWindowEx (hWnd, previous - si.nPos, 0, &r, &r,
                            NULL, NULL, SW_INVALIDATE);

            POINT caret;
            if (GetCaretPos (&caret)) {
                caret.x += previous - si.nPos;
                SetCaretPos (caret.x, caret.y);
            };
        };
    };
    return 0;
};
LRESULT OnVerticalScroll (HWND hWnd, UINT event, UINT n) {
    LRESULT result = 0;
    
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_TRACKPOS | SIF_RANGE | SIF_PAGE | SIF_POS;

    if (GetScrollInfo (hWnd, SB_VERT, &si)) {

        RECT r;
        GetClientRect (hWnd, &r);
            
        const Data * const data = Data::Ref (hWnd);
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
                si.nPos -= r.bottom - data->margin.top;
                break;
            case SB_PAGEDOWN:
                si.nPos += r.bottom - data->margin.top;
                break;

            case SB_LINEUP:
                si.nPos -= n * data->character.cy;
                break;
            case SB_LINEDOWN:
                si.nPos += n * data->character.cy;
                break;

            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                si.nPos = si.nTrackPos;
                break;
            
            case 0xFFFF:
                si.nPos += (SHORT) HIWORD (event);
                
                if (si.nPos >= maximum)
                    result = 1;
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

        // TODO: mam-li FractionalScrolling generovat scrollbar po radcich
        //        - najit jeho pouziti i na dalsich mistech
        // Fractional scrolling
        //  - if not enabled, rounding position (and thumb height) to integer
        //    multiplier of default font character height

        if (!(GetWindowLongPtr (hWnd, GWL_STYLE) & Style::FractionalScrolling)) {
            si.nPos /= data->character.cy;
            si.nPos *= data->character.cy;
        };

        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_VERT, &si, TRUE);

        // Scrolling window content
        //  - TODO: see and implement notes for OnHorizontalScroll

        r.top = data->margin.top;
        ScrollWindowEx (hWnd, 0, previous - si.nPos, &r, &r,
                        NULL, NULL, SW_INVALIDATE);
        
        POINT caret;
        if (GetCaretPos (&caret)) {
            caret.y += previous - si.nPos;
            SetCaretPos (caret.x, caret.y);
        };
    };
    return result;
};

LRESULT OnHorizontalWheel (HWND hWnd, SHORT distance, USHORT flags) {
    Data * const data = Data::Ref (hWnd);
    distance += data->wheelH;

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

    data->wheelH = distance;
    return 0;
};
LRESULT OnVerticalWheel (HWND hWnd, SHORT distance, USHORT flags) {
    Data * const data = Data::Ref (hWnd);
    distance += data->wheelV;
    
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

    data->wheelV = distance;
    return 0;
};

LRESULT OnTimer (HWND hWnd, DWORD timer) {
    switch (timer) {
        
        // scroll timer
        //  - started with capture, scrolls if mouse leaves the window
        
        case 1u: {
            RECT r;
            POINT pt;
            const Data * const data = Data::Ref (hWnd);
            
            if (GetWindowRect (hWnd, &r) && GetCursorPos (&pt)) {
                
                r.left += data->margin.left;
                r.top += data->margin.top;
                
                if (!PtInRect (&r, pt)) {
                    if (pt.x < r.left) {
                        OnHorizontalScroll (hWnd, SB_LINELEFT,
                                            GetScrollMultiplier (true, r.left - pt.x));
                    } else
                    if (pt.x >= r.right) {
                        OnHorizontalScroll (hWnd, SB_LINERIGHT,
                                            GetScrollMultiplier (true, pt.x - r.right));
                    };
                    
                    if (pt.y < r.top) {
                        OnVerticalScroll (hWnd, SB_LINEUP,
                                          GetScrollMultiplier (false, r.top - pt.y));
                    } else
                    if (pt.y >= r.bottom) {
                        OnVerticalScroll (hWnd, SB_LINEDOWN,
                                          GetScrollMultiplier (false, pt.y - r.bottom));
                    };
                    
                    // recompute selection rectangle
                    //  - without this, it would have scrolled away
                    
                    MapWindowPoints (HWND_DESKTOP, hWnd, &pt, 1u);
                    OnMouseMove (hWnd, pt.x, pt.y);
                };
            };
        } break;
        
    };
    return 0;
};

DWORD LocateCharacter (HDC hDC, RECT & r, unsigned int x, 
                       const wchar_t * string, unsigned int length) {
    
    // Find two letters closest to X
    
    unsigned int n = 1u;
    unsigned int o = r.left;
    
    while (n <= length
        && DrawText (hDC, string, n, &r, DT_CALCRECT |
                     DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP)) {
        
        // test if X belong to gap between letters N and N-1
        //  - this effectively splits the width of the two letters
        
        if (x < o + ((r.right - o) / 2u)) {
            
            // update r.right
            //  - OnClick sets caret according to this
            
            r.right = o;
            return n;
        };
        
        o = r.right;
        ++n;
    };
    
    return length + 1u;
};

DWORD LocateToken (HWND hWnd, UINT row, int x,
                   RECT & r, POINT * caret) {
    
    DWORD result = -1u;
    const Data * const data = Data::Ref (hWnd);
    
    if (HDC hDC = GetDC (hWnd)) {
        HGDIOBJ hPrevious = SelectObject (hDC, data->hFont);
        LPCTSTR string = NULL;
        
        unsigned int i = 0;
        unsigned int sum = 0u;
        unsigned int length = 0;
        
        if (x > 0)
        while (SendNextTokenRequest (hWnd, row, i, string, length)) {
            if (DrawText (hDC, string, length, &r, DT_CALCRECT |
                          DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP)) {
                if (x < r.right) {
                    
                    // location
                    //  - determine coordinates as row and column of the cursor
                    
                    if (caret)
                        sum += LocateCharacter (hDC, r, x, string, length) - 1u;

                    result = i;
                    break;
                };
                r.left = r.right;
            };
            
            SelectObject (hDC, data->hFont);
            sum += length;
            ++i;
        };

        if (caret) {
            caret->y = row;
            caret->x = sum;
        };

        if (hPrevious)
            SelectObject (hDC, hPrevious);

        ReleaseDC (hWnd, hDC);
    };
    return result;
};

NMMOUSE LocateCursor (HWND hWnd, POINT pt, Hit hit, POINT * caret) {
    
    const Data * const data = Data::Ref (hWnd);
    const UINT rows = SendRequest (hWnd, Request::Rows);
    
    // nmMouse
    //  - typical notification
    //  - this is returned to caller

    NMMOUSE nmMouse = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), 0u },
        -1u, -1u, pt, hit
    };

    switch (hit) {
        case CaptionHit:
            if (pt.x < (int) data->margin.left) {

                // mouse position is before first (index 0) column
                //  - return -1u as invalid index
                //  - set spec so user can further distinguish boundaries

                nmMouse.dwItemData = MAKELONG (data->index.padding.left,
                                               data->index.width);

            } else {

                // column index (width of letter 'M' of default font)

                nmMouse.dwItemData = 0u;
                nmMouse.dwItemSpec = (pt.x - data->margin.left
                                           + GetScrollPos (hWnd, SB_HORZ))
                                   / data->character.cx;
            };
        break;

        case PrefixHit:
        case NumberHit:
        case PostfixHit:
        case TextHit: {

            // common per-row walkthrough
            //  - to find which row the mouse cursor points at
            //  - adding heights of rows in the view (not neccessarily visible)

            unsigned int top = data->margin.top;
            unsigned int fra = 0u;
            unsigned int row = SendFirstRowRequest (hWnd, data, fra);
            
            for (; row < rows; ++row) {
                unsigned int rh = GetRowHeight (hWnd, data, row);
                if (pt.y + int (fra) < int (top + rh)) {
                    nmMouse.dwItemSpec = row;
                    break;
                };

                top += rh;
            };

            // mouse in actual text
            //  - walk by all "words" in the row to find which one the mouse
            //    cursor points at
            //  - processing click tests below last row as if on the last row
            //    to update cursor position, but not providing callback since
            //    not really clicked there (nmMouse.dwItemSpec == -1u)

            if (hit == TextHit || hit == PostfixHit) {
                
                if (nmMouse.dwItemSpec == -1u)
                    row = rows - 1;

                RECT r;
                r.top = top;
                r.left = data->margin.left;
                
                DWORD token = LocateToken (hWnd, row,
                                           pt.x + GetScrollPos (hWnd, SB_HORZ),
                                           r, caret);

                // r.left here is beginning of the 'token'-th word
                
                if (nmMouse.dwItemSpec != -1u)
                    nmMouse.dwItemData = token;
            };
        } break;
        
        case OutsideHit:
        break;
    };

    // return final structure
    //  - pt needs to be adjusted to screen space as per specs

    MapWindowPoints (hWnd, NULL, &nmMouse.pt, 1u);
    return nmMouse;
};

LRESULT MapIndexToToken (HWND hWnd, UINT row,
                         Shadow::Editor::Mapping & mapping) {
    unsigned int i = 0;
    unsigned int column = 0u;
    unsigned int length = 0;

    LPCTSTR string = NULL;
    
    if (mapping.column) {
        
        // Mapping column to token:index
        //  - traverse the tokens until value in question belongs into the token
        //  - offset is distance between request and token beginning
        
        while (SendNextTokenRequest (hWnd, row, i, string, length)) {
            if (mapping.column < column + length) {
                mapping.offset = mapping.column - column;
                mapping.token = i;
                return TRUE;
            };
            
            column += length;
            ++i;
        };
        
    } else {
        
        // Mapping token:index to column
        //  - accumulate column taken by all preceeding tokens
        //  - finally add provided offset (if valid) to the return value
        
        for (; i < mapping.token; ++i)
            if (SendNextTokenRequest (hWnd, row, i, string, length)) 
                column += length;
            else
                return FALSE;
            
        if (SendNextTokenRequest (hWnd, row, mapping.token, string, length))
            if (mapping.offset < length) {
                mapping.column = column + mapping.offset;
                return TRUE;
            };
    };
    return FALSE;
};

LRESULT SetSelection (HWND hWnd, UINT all,
                      const Shadow::Editor::Selection * selection) {

    Data * const data = Data::Ref (hWnd);
    if (selection) {
        const unsigned int rows = SendRequest (hWnd, Request::Rows);
        
        if (   selection->first.row < rows
            && selection->last.row < rows
            && selection->first.column <= GetRowLength (hWnd, selection->first.row)
            && selection->last.column <= GetRowLength (hWnd, selection->last.row)) {
            
            data->selection.row = selection->first.row;
            data->selection.column = selection->first.column;
            data->caret.row = selection->last.row;
            data->caret.column = selection->last.column;
            
            if (selection->column)
                data->selection.type = Data::Selection::Column;
            else
                data->selection.type = Data::Selection::Normal;
            
            UpdateViewToCaret (hWnd, data);
            UpdateCaret (hWnd, data);
            InvalidateRect (hWnd, NULL, FALSE);
            
            return TRUE;
        } else
            return FALSE;
    } else {
        if (all) {
            unsigned int row = SendRequest (hWnd, Request::Rows);
            if (FindPrevRow (hWnd, data, row)) {

                data->selection.type = Data::Selection::Normal;
                data->selection.row = 0;
                data->selection.column = 0;
                data->caret.row = row;
                data->caret.column = GetRowLength (hWnd, row);
    
                OnVerticalScroll (hWnd, SB_BOTTOM);
                UpdateViewToCaretHorizontal (hWnd, data);
            } else
                return FALSE;
                
        } else {
            data->selection.type = Data::Selection::None;
            UpdateSelectionCaret (hWnd, data);
        };

        UpdateCaret (hWnd, data);
        InvalidateRect (hWnd, NULL, FALSE);
        return TRUE;
    };
};

void Recompute (HWND hWnd, HDC hDC, Data * const data) {
    
    // Text font parameters

    SelectObject (hDC, data->hFont);

    SIZE size = { 8, 16 };
    GetTextExtentPoint32 (hDC, L"M", 1u, &size);
    data->character = size;

    GetTextExtentPoint32 (hDC, L"f", 1u, &size);
    data->bold_caret = size.cx;
    
    // Numbers column width
    //  - TODO: ensure that Recompute is called each time rows count changes

    {   unsigned int w = 0u;
        unsigned int rows = SendRequest (hWnd, Request::Rows);
        do {
            ++w;
        } while (rows /= 10u);
    
        if (w < data->index.minimum)
            w = data->index.minimum;

        static const wchar_t zeros [12] = L"0000000000";
        
        SelectObject (hDC, (HGDIOBJ) SendDlgItemMessage (hWnd, 1, WM_GETFONT, 0, 0));
        GetTextExtentPoint32 (hDC, zeros, 10u, &size);
    
        // final column width
        //  - complicated division of 'nw' here rounds the result upwards
    
        data->index.width = w * (size.cx + w - 1u)
                              / 10u
                          + 2u * Data::MagicIndexBorder;
    };
    
    data->margin.left = data->index.padding.left
                      + data->index.padding.right
                      + data->index.width;

    SendDlgItemMessage (hWnd, 1, Index::Message::SetPadding,
                                 data->index.padding.left,
                                 data->index.padding.right);
    SendDlgItemMessage (hWnd, 1, Index::Message::SetDefaultRowHeight,
                                 data->character.cy, 0);
    return;
};

void UpdateScrollBars (HWND hWnd) {
    RECT rc;
    const LONG_PTR style = GetWindowLongPtr (hWnd, GWL_STYLE);
    
    if ((style & (WS_VSCROLL | WS_HSCROLL))
                && GetClientRect (hWnd, &rc)) {
        
        SCROLLINFO si;
        si.cbSize = sizeof si;
        si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE;
        si.nMin = 0;
        
        // TODO: all following parameters are already retrieved in OnPaint
        //       so somehow just pass them here; maybe put it just in there
        
        const Data * const data = Data::Ref (hWnd);
        const bool fractional = GetWindowLongPtr (hWnd, GWL_STYLE)
                              & Style::FractionalScrolling;
        
        if (style & WS_VSCROLL) {
            const unsigned int rows = SendRequest (hWnd, Request::Rows);
            
            const unsigned int page = rc.bottom - data->margin.top;
            const unsigned int sum = SendRowDetailRequest (hWnd, Request::HeightSum, rows,
                                                           rows * data->character.cy);

            unsigned int add = data->thumb.minimum * sum / 50u;
            if (add >= page) {
                add -= page;
            } else
                add = 0u;

            si.nPage = page + add;
            si.nMax = sum + add;
                
            // Fractional scrolling
            //  - if not enabled, rounding thumb height (and position) to integer
            //    multiplier of default font character height

            if (!fractional) {
                si.nPage /= data->character.cy;
                si.nPage *= data->character.cy;
            };

            SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
        };
        
        if (data->autoscrollbar || (style & WS_HSCROLL)) {
            
            si.nPage = rc.right - data->margin.left;
            si.nMax = data->character.cx
                    * SendRequest (hWnd, Request::LengthMax);
            
            // Fractional scrolling
            //  - if not enabled, rounding thumb width (and position) to integer
            //    multiplier of default font character width
    
            if (!fractional) {
                si.nPage /= data->character.cx;
                si.nPage *= data->character.cx;
            };

            if (si.nPage < unsigned (si.nMax) || style & Style::AlwaysHorzScrollBar) {
                SetScrollInfo (hWnd, SB_HORZ, &si, TRUE);
            } else {
                ShowScrollBar (hWnd, SB_HORZ, FALSE);
            };
        };
    };
    return;
};

POINT LocateCharacterOffset (HWND hWnd, UINT row, UINT column, SIZE * caret) {
    const Data * const data = Data::Ref (hWnd);
    
    // Vertical offset
    //  - per-row walkthrough from current position since this way it is faster
    //  - height is saved as maximum caret height if necessary below

    unsigned int vertical = 0u;
    unsigned int fraction = 0u;
    unsigned int i = SendFirstRowRequest (hWnd, data, fraction);
    if (i != row) {
        if (i < row) {
            for (; i < row; ++i)
                vertical += GetRowHeight (hWnd, data, i);
        } else {
            for (; i != row; --i)
                vertical -= GetRowHeight (hWnd, data, i);
        };
    };
    
    // Horizontal offset is a little more complicated
    //  - nothing needs to be computed if required column is 0
    
    RECT r = { 0,0,0,0 };
    if (HDC hDC = GetDC (hWnd)) {
        HGDIOBJ hPrevious = SelectObject (hDC, data->hFont);

        // per-token walkthrough
        //  - find offset of provided column

        LPCTSTR string = NULL;

        unsigned int i = 0u;
        unsigned int sum = 0u;
        unsigned int length = 0u;
        
        if (column != 0u)
            while (SendNextTokenRequest (hWnd, row, i++, string, length)) {
                bool done = false;
//                printf ("%u > %u - %u (%u) = %u\n",
//                        length, column, sum, column - sum, length > column - sum);
                if (length > column - sum) { // ???????
                    length = column - sum;
                    done = true;
                };
    
                if (length) {
                    if (DrawText (hDC, string, length, &r,
                                  DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX)) {
                        r.left = r.right;
                    };
                };
    
                if (done)
                    break;
    
                SelectObject (hDC, data->hFont);
                sum += length;
            };
        
        // get typical font height
        //  - on first column of any token, or if there are no tokens at all
        
        if (!r.bottom) {
            DrawText (hDC, L" ", 1u, &r,
                      DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
        };
        
        // compute caret size if required
        //  - the caret size is based on the font size
        //  - width is relative to font size
        //     - TODO: algorithm should be improved as the control is used
        //  - height is product of DT_CALCRECT locating the token
        //     - height cannot be larger than row height
        
        if (caret) {
            TEXTMETRIC tm;
            GetTextMetrics (hDC, &tm);
            
            caret->cx = (tm.tmAveCharWidth < 7) ? 1u
                      : (tm.tmAveCharWidth < 16) ? 2u
                      : 3u;
            caret->cy = std::min (GetRowHeight (hWnd, data, row),
                                  (unsigned) r.bottom)
                      + 1u;
            
            // caching token index
            //  - caret size is requested when moved
            
            Data::Ref (hWnd) ->caret.token = i ? (i - 1u) : i;
        };

        if (hPrevious)
            SelectObject (hDC, hPrevious);

        ReleaseDC (hWnd, hDC);
    };
    
    // build result to return
    
    const POINT result = { r.left, (long) (vertical - fraction) };
    return result;
};

void UpdateCaret (HWND hWnd, Data * data, bool hinting) {
    SIZE size;
    const POINT offset = LocateCharacterOffset (hWnd, data->caret.row,
                                                      data->caret.column,
                                                      &size);
    if (data->overwritting) {
        size.cx = data->bold_caret;
    };
    
    if (data->caret.height != size.cy
        || data->caret.width != size.cx) {
        
        data->caret.height = size.cy;
        data->caret.width = size.cx;
        
        CreateCaret (hWnd, NULL, data->caret.width, data->caret.height);
        ShowCaret (hWnd);
    };
    
    if (hinting) {
        data->caret.offset = offset.x;
    };
    
    SetCaretPos (offset.x + data->margin.left
                          - GetScrollPos (hWnd, SB_HORZ)
                          + data->overwritting,
                 offset.y + data->margin.top);
    return;
};

bool UpdateViewToCaret (HWND hWnd, Data * data) {
    bool v = UpdateViewToCaretVertical (hWnd, data);
    bool h = UpdateViewToCaretHorizontal (hWnd, data);
    return v || h;
};

bool UpdateViewToCaretVertical (HWND hWnd, Data * data) {
    
    // If caret is above the view, scroll down appropriately
    //  - if the current row is also the first, just scroll down by the fraction
    
    unsigned int fraction = 0u;
    unsigned int first = SendFirstRowRequest (hWnd, data, fraction);

    if (data->caret.row == first && fraction) {
        OnVerticalScroll (hWnd, MAKELONG (0xFFFF, (SHORT) -fraction));
        return true;
    };
    if (data->caret.row < first) {
        POINT offset = LocateCharacterOffset (hWnd, data->caret.row,
                                                    data->caret.column);
        SetScrollPos (hWnd, SB_VERT,
                      GetScrollPos (hWnd, SB_VERT) + offset.y,
                      TRUE);

        InvalidateRect (hWnd, NULL, FALSE);
        data->reposition = true;
        return true;
    };

    // If caret is below the view, scroll up appropriately

    RECT rc;
    POINT caret;
    
    if (GetCaretPos (&caret)
            && GetClientRect (hWnd, &rc)
            && caret.y > rc.bottom - data->caret.height) {
        
        unsigned int vpos = (GetScrollPos (hWnd, SB_VERT) + caret.y)
                          - (rc.bottom - data->caret.height);
        
        if (!(GetWindowLongPtr (hWnd, GWL_STYLE) & Style::FractionalScrolling)) {
            if (vpos % data->character.cy) {
                vpos /= data->character.cy;
                ++vpos;
                vpos *= data->character.cy;
            };
        };

        SetScrollPos (hWnd, SB_VERT, vpos, TRUE);
        
        InvalidateRect (hWnd, NULL, FALSE);
        data->reposition = true;
        return true;
    };
    
    // TODO: If view to caret has been updated, allow callback for some animation
    
    return false;
};

bool UpdateViewToCaretHorizontal (HWND hWnd, Data * data) {
    
    // Is caret
    
    RECT rc;
    POINT caret;

    if (GetCaretPos (&caret) && GetClientRect (hWnd, &rc)) {
        
        if (caret.x < (int) data->margin.left) {
            OnHorizontalScroll (hWnd, MAKELONG (0xFFFF,
                                      caret.x - data->margin.left));
            return true;
        };

        rc.right -= 2u * data->character.cx;

        if (caret.x >= rc.right) {
            OnHorizontalScroll (hWnd, MAKELONG (0xFFFF,
                                      caret.x - rc.right));
            return true;
        };
    };
    
    // TODO: If view to caret has been updated, allow callback for some animation

    return false;
};

void UpdateSelectionCaret (HWND hWnd, Data * data) {
    
    // Selection update
    //  - do select, if shift is pressed and the caret actually moved
    //     - checking selection for empty
    //     - checking ALT key for selection type
    //  - unselect, if shift is released and clear the attributes

    if ((GetKeyState (VK_SHIFT) & 0x8000) || (GetCapture () == hWnd)) {
        switch (data->selection.type) {
            case Data::Selection::None:
                if (data->selection.column != data->caret.column
                    || data->selection.row != data->caret.row) {

                    if (GetKeyState (VK_MENU) & 0x8000) {
                        data->selection.type = Data::Selection::Column;
                    } else {
                        data->selection.type = Data::Selection::Normal;
                    };
                };
                break;
            case Data::Selection::Normal:
                if (data->selection.column == data->caret.column
                    && data->selection.row == data->caret.row) {
                    
                    data->selection.type = Data::Selection::None;
                };
                break;
            case Data::Selection::Column:
                // doing nothing since even empty column is column of newlines
                break;
        };

        InvalidateRect (hWnd, NULL, FALSE);
    } else {
        if (data->selection.type != Data::Selection::None) {
            data->selection.type = Data::Selection::None;
            InvalidateRect (hWnd, NULL, FALSE);
        };

        data->selection.row = data->caret.row;
        data->selection.column = data->caret.column;
    };

    return;
};

bool FindPrevRow (HWND hWnd, Data * data, unsigned int & row) {
    if (row > 0) {
        do {
            --row;
        } while (row > 0 && !GetRowHeight (hWnd, data, row));

        if (GetRowHeight (hWnd, data, row))
            return true;
    };
    return false;
};

bool FindNextRow (HWND hWnd, Data * data, unsigned int & row) {
    const unsigned int rows = SendRequest (hWnd, Request::Rows);

    if (row + 1u < rows) {
        do {
            ++row;
        } while (row < rows && !GetRowHeight (hWnd, data, row));
        
        if (GetRowHeight (hWnd, data, row))
            return true;
    };
    return false;
};

Hit HitTest (HWND hWnd, POINT pt) {
    RECT rc;
    if (GetClientRect (hWnd, &rc)
          && PtInRect (&rc, pt)) {

        const Data * const data = Data::Ref (hWnd);

        // caption
        //  - everything above top padding

        if (pt.y < int (data->margin.top))
            return CaptionHit;
        else {
            
            // prefix
            //  - everything left of numbers
    
            if (pt.x < int (data->index.padding.left))
                return PrefixHit;
            else {
    
                // numbers
                //  - get numbers column width and check
                //  - postfix and text are simple
    
                if (pt.x < int (data->index.padding.left + data->index.width))
                    return NumberHit;
                else
                if (pt.x < int (data->margin.left))
                    return PostfixHit;
                else
                    return TextHit;
            };
        };
    } else
        return OutsideHit;
};

unsigned int GetRowLength (HWND hWnd, unsigned int row) {
    return SendRowDetailRequest (hWnd, Request::Length, row);
};
unsigned int GetRowHeight (HWND hWnd, const Data * data, unsigned int row) {
    return SendRowDetailRequest (hWnd, Request::Height,
                                 row, data->character.cy);
};

unsigned int GetScrollMultiplier (bool horz, unsigned int px) {
    const unsigned int size = horz ? GetSystemMetrics (SM_CXSCREEN)
                                   : GetSystemMetrics (SM_CYSCREEN);
    
    // arbitrary multiplier formula
    //  - main multiplier is 20th of appropriate primary screen dimension
    //  - 5/4 is for slight exponencial acceleration
    
    return 1u + (5u * px)
              / (4u * size / 20u);
};

LRESULT SendNotify (HWND hWnd, UINT code) {
    NMHDR nm = {
        hWnd, (UINT) GetDlgCtrlID (hWnd), code
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.idFrom, (LPARAM) &nm);
};
LRESULT SendRequest (HWND hWnd, UINT code) {
    return SendNotify (hWnd, code);
};
LRESULT SendMouseNotify (HWND hWnd, UINT code, NMMOUSE * nm) {
    nm->hdr.code = code;
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm->hdr.idFrom, reinterpret_cast <LPARAM> (nm));
};
LRESULT SendBumpNotify (HWND hWnd, UINT vk) {
    NMBump nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Notify::Bump },
        vk
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};

LRESULT SendFirstRowRequest (HWND hWnd, const Data * data,
                             unsigned int & fraction) {
    
    const UINT off = GetScrollPos (hWnd, SB_VERT);
    const UINT row = off / data->character.cy;
    const UINT fra = off % data->character.cy;
    
    NMFirst nmFirst = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Request::First },
        off, row, fra
    };

    if (SendMessage (GetParent (hWnd), WM_NOTIFY,
                     nmFirst.hdr.idFrom, (LPARAM) &nmFirst)) {

        fraction = nmFirst.fraction;
        return nmFirst.row;
    } else {
        fraction = fra;
        return row;
    };
};

LRESULT SendRowDetailRequest (HWND hWnd, UINT code, UINT row, UINT value) {
    NMValue nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), code },
        row, value
    };
    if (SendMessage (GetParent (hWnd), WM_NOTIFY,
                     nm.hdr.idFrom, (LPARAM) &nm)) {
        return nm.value;
    } else
        return value;
};
LRESULT SendNextTokenRequest (HWND hWnd, UINT row, UINT i,
                              LPCTSTR & string, unsigned int & length) {
    NMToken nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::Token },
        row, i, 0, NULL
    };
    
    LRESULT result = SendMessage (GetParent (hWnd), WM_NOTIFY,
                                  nm.hdr.idFrom, (LPARAM) &nm);
    if (result) {
        string = nm.string;
        
        if (nm.length < 0)
            length = std::wcslen (nm.string);
        else
            length = nm.length;
    };
    return result;
};

LRESULT SendAnchorRequest (HWND hWnd, UINT vk, Data * data) {
    NMAnchor nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::Anchor },
        data->caret.token,
        vk,
        data->caret.row,
        data->caret.column
    };

    LRESULT result = SendMessage (GetParent (hWnd), WM_NOTIFY,
                                  nm.hdr.idFrom, (LPARAM) &nm);
    if (result) {
        data->caret.row = nm.row;
        data->caret.column = nm.column;
        
        UpdateCaret (hWnd, data);
    };
    return result;
};

LRESULT SendCharNotify (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    NMCHAR nmChar = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_CHAR },
        wParam, (UINT) lParam, 0u
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmChar.hdr.idFrom, (LPARAM) &nmChar);
};

LRESULT SendKeyDownNotify (HWND hWnd, WPARAM wParam, LPARAM lParam) {
    NMKEY nmKey = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), (UINT) NM_KEYDOWN },
        wParam, (UINT) lParam
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nmKey.hdr.idFrom, (LPARAM) &nmKey);
};

LRESULT SendInsertRequest (HWND hWnd, UINT row, UINT index, TCHAR c) {
    return SendInsertRequest (hWnd, row, index, &c, 1u);
};
LRESULT SendInsertRequest (HWND hWnd, UINT row, UINT index,
                           LPCTSTR string, UINT length) {
    NMInsert nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::Insert },
        row, index, length, string
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};
LRESULT SendBreakRowRequest (HWND hWnd, UINT row, UINT column) {
    NMNewLine nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::NewLine },
        row, column
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};
LRESULT SendUnBreakRequest (HWND hWnd, UINT row) {
    NMUnBreak nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::UnBreak },
        row
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};

LRESULT SendDeleteRequest (HWND hWnd, UINT row, UINT index, UINT length) {
    NMDelete nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::Delete },
        row, index, length
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};

LRESULT SendCopyAppendRequest (HWND hWnd, UINT row, UINT index,
                                          UINT length, bool nl) {
    NMCopyAppend nm = {
        { hWnd, (UINT) GetDlgCtrlID (hWnd), Shadow::Editor::Request::CopyAppend },
        row, index, length, nl
    };
    return SendMessage (GetParent (hWnd), WM_NOTIFY,
                        nm.hdr.idFrom, (LPARAM) &nm);
};

};
