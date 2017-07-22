#include "Windows_CaptionHarness.hpp"

/* Emphasize Windows Client area to Title bar extension
// Windows_CaptionHarness.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      23.06.2011 - initial version
//      09.09.2012 - fixed WM_ACTIVATE to WM_NCACTIVATE
//      09.10.2013 - extended of four new types of glass frame - 2.0
*/

#include "../Windows/Windows_UxTheme.hpp"
#include <cstdio>

namespace {
    bool BackgroundFill (HWND, HDC, const RECT *, const RECT *,
                         const Windows::CaptionHarness::Settings &, bool);
    
    BOOL CALLBACK RedrawChildProc (HWND hWnd, LPARAM) {
        RedrawWindow (hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);
        return TRUE;
    };
    
    const TCHAR * szPropActive = TEXT ("Windows::CaptionHarness");
};

bool Windows::CaptionHarness::IsProcessibleMesssage (UINT message) {
    switch (message) {
        case WM_CREATE:         // 1
        case WM_SIZE:           // 5
        case WM_ERASEBKGND:     // 0x14
        case WM_GETMINMAXINFO:  // 0x24
        case WM_NCCALCSIZE:     // 0x83
        case WM_NCHITTEST:      // 0x84
        case WM_NCPAINT:        // 0x85
        case WM_NCACTIVATE:     // 0x86
        case WM_NCRBUTTONDOWN:
        case WM_CTLCOLORBTN:    // 0x135
        case WM_CTLCOLORSTATIC: // 0x138
        case WM_EXITSIZEMOVE:   // 0x0232
        case WM_THEMECHANGED:   // 0x031A
            return true;
    };
    return false;
};

bool Windows::CaptionHarness::ProcessMessage (HWND hWnd, UINT message,
                                              WPARAM wParam, LPARAM lParam,
                                              const Settings & settings,
                                              LRESULT * result) {
    using namespace Windows;
    switch (message) {
        
        // WM_CREATE
        //  - force resize, activation and frame repaint
        
        case WM_CREATE:
            {
                RECT r;
                if (GetWindowRect (hWnd, &r)) {
                    SetWindowPos (hWnd, NULL, r.left, r.top, r.right - r.left,
                                  r.bottom - r.top, SWP_FRAMECHANGED | SWP_DRAWFRAME);
                };
            };
            
            // continue...
        
        case WM_THEMECHANGED:
            //  - 7u = STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT
            //  - 3u = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON
            switch (settings.type) {
                case Windows::CaptionHarness::Basic:
                case Windows::CaptionHarness::FullGlass:
                    Windows::UxTheme::SetNonClientOptions (hWnd, settings.menu ? 0u : 6u, 7u);
                    break;
                case Windows::CaptionHarness::JustGlass:
                case Windows::CaptionHarness::ClientTitle:
                    Windows::UxTheme::SetNonClientOptions (hWnd, settings.menu ? 3u : 7u, 7u);
                    break;
            };
            
            Windows::UxTheme::SetAppProperties (7u);
            PostMessage (hWnd, WM_EXITSIZEMOVE, 0u, 0u);
            break;
        
        // WM_EXITSIZEMOVE
        //  - a reused message that extends Aero border by settings
        
        case WM_EXITSIZEMOVE:
            switch (settings.type) {
                case Windows::CaptionHarness::JustGlass:
                case Windows::CaptionHarness::FullGlass:
                    UxTheme::ExtendFrame (hWnd, -1, -1, -1, -1);
                    break;
                    
                case Windows::CaptionHarness::Basic:
                    UxTheme::ExtendFrame (hWnd, settings.margins.left,
                                                settings.margins.top,
                                                settings.margins.right,
                                                settings.margins.bottom);
                    break;
                    
                case Windows::CaptionHarness::ClientTitle:
                    UxTheme::ExtendFrame (hWnd, settings.margins.left,
                                                settings.margins.top
                                                    + GetSystemMetrics (SM_CYFRAME),
                                                settings.margins.right,
                                                settings.margins.bottom);
                    break;
            };
            break;
        
        // WM_NCACTIVATE
        //  - sends ourselves WM_EXITSIZEMOVE
        //  - force redraw of the title-bar

        case WM_NCACTIVATE:
            SetProp (hWnd, szPropActive, (HANDLE) wParam);
            
            ProcessMessage (hWnd, WM_EXITSIZEMOVE, 0,0, settings, NULL);
            *result = UxTheme::DefWindowProc (hWnd, message, wParam, lParam);

            if (!(GetWindowLongPtr (hWnd, GWL_EXSTYLE) & WS_EX_COMPOSITED)) {
                
                UINT redraw = RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN;
                
                if (wParam) {
                    redraw |= RDW_FRAME;
                };
                
                if (UxTheme::Current () == UxTheme::Classic) {
                    redraw |= RDW_ERASENOW;
                } else {
                    redraw |= RDW_UPDATENOW;
                };
                
                RedrawWindow (hWnd, NULL, NULL, redraw);
            };
            return true;

        // WM_NCCALCSIZE
        //  - this extends client area into window caption
        //  - after excessive studying of how this peculiar message works I have
        //    abandoned all hope to preserve top sizing border, and went and
        //    used the following simple version and reinvented hit-testing in
        //    WM_NCHITTEST below

        case WM_NCCALCSIZE:
            switch (settings.type) {
                case Windows::CaptionHarness::Basic:
                case Windows::CaptionHarness::FullGlass:
                    return false;
                    
                case Windows::CaptionHarness::JustGlass:
                case Windows::CaptionHarness::ClientTitle:
                    if (wParam) {
                        NCCALCSIZE_PARAMS * p = (NCCALCSIZE_PARAMS *) lParam;
                        
                        RECT rNew = p->rgrc[0];
                        RECT rOld = p->rgrc[1];
                        RECT rOldClient = p->rgrc[2];
                        
                        p->rgrc[0] .left = rOldClient.left + rNew.left - rOld.left;
                        p->rgrc[0] .right = rOldClient.right + rNew.right - rOld.right;
                        p->rgrc[0] .bottom = rOldClient.bottom + rNew.bottom - rOld.bottom;
        
                        *result = 0;
                        return true;
                    };
            };
            break;

        // WM_NCPAINT
        //  - hack to remove sizing flicker
        
        case WM_NCPAINT:
            if (UxTheme::Current () != UxTheme::Aero) {
                Sleep (2);
            };
            break;
        
        // WM_NCRBUTTONDOWN
        //  - 
        
        case WM_NCRBUTTONDOWN:
            switch (wParam) {
                case HTCAPTION:
                case HTSYSMENU:
                    if (settings.menu) {
                        SendMessage (hWnd, /*WM_SYSMENU*/787, 0, lParam);
                    };
                    break;
            };// */
            break;

        // WM_SIZE
        //  - request child window (in caption) to redraw when sizing
        //    since Windows could redraw the caption
        
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                if (UxTheme::Current () != UxTheme::Aero) {
                    RedrawWindow (hWnd, NULL, NULL,
                                  RDW_INVALIDATE | RDW_FRAME |
                                  RDW_UPDATENOW | RDW_ALLCHILDREN);
                };
                ProcessMessage (hWnd, WM_EXITSIZEMOVE, 0,0, settings, NULL);
            };
            break;// */

        // WM_NCHITTEST
        //  - first have (Dwm)DefWindowProc to determine its hittest value
        //    and then correct it
        
        case WM_NCHITTEST: {
            BOOL dwm = FALSE;
            *result = UxTheme::DefWindowProc (hWnd, message, wParam, lParam, &dwm);

            switch (*result) {
                case HTCLIENT:
                case HTBORDER: {
                    RECT r;
                    POINT pt = { (signed short) LOWORD (lParam),
                                 (signed short) HIWORD (lParam) };

                    GetClientRect (hWnd, &r);
                    ScreenToClient (hWnd, &pt);
                    
                    bool shifted = false;
                    int topoffset = 0;
                    
                    switch (settings.type) {
                        case Windows::CaptionHarness::Basic:
                        case Windows::CaptionHarness::FullGlass:
                            break;
                        case Windows::CaptionHarness::JustGlass:
                        case Windows::CaptionHarness::ClientTitle:
                            shifted = true;
                            topoffset = GetSystemMetrics (SM_CYFRAME);
                            break;
                    };
                    
                    
                    if (shifted && pt.y < GetSystemMetrics (SM_CYFRAME))
                        *result = HTTOP;
                    else
                    if (pt.y < settings.margins.top + topoffset) {
                        if (settings.menu
                                && pt.y < GetSystemMetrics (SM_CYCAPTION)
                                        + GetSystemMetrics (SM_CYFRAME)
                                && pt.x < GetSystemMetrics (SM_CYFRAME)
                                        + GetSystemMetrics (SM_CXSMICON)) {

                            *result = HTSYSMENU;
                        } else

                        if (UxTheme::Current () != UxTheme::Aero
                                && shifted
                                && pt.y < GetSystemMetrics (SM_CYCAPTION)
                                        + topoffset) {

                            if (pt.x > r.right - GetSystemMetrics (SM_CXSIZE))
                                *result = HTCLOSE;
                            else
                            if (pt.x > r.right - 2*GetSystemMetrics (SM_CXSIZE)) {
                                *result = HTMAXBUTTON;
                            } else
                            if (pt.x > r.right - 3*GetSystemMetrics (SM_CXSIZE))
                                *result = HTMINBUTTON;
                            else
                                *result = HTCAPTION;
                        } else
                            *result = HTCAPTION;
                    } else
                    if (pt.x > r.right - 15 && pt.y > r.bottom - 15) {
                        
                        unsigned int xx = r.right - pt.x;
                        unsigned int yy = r.bottom - pt.y;
                        
                        if (xx + yy < 16)
                            *result = HTBOTTOMRIGHT;
                    };
                } break;
                
                case HTCLOSE:
                case HTMAXBUTTON:
                case HTMINBUTTON:
                    if (UxTheme::Current () == UxTheme::Aero && !dwm) {
                        *result = HTCLIENT;
                    };
                    break;
            };
        } return true;
        
        // WM_GETMINMAXINFO
        //  - if minimum tracking window dimmensions are smaller than extended
        //    caption and borders, update accordingly
        
        case WM_GETMINMAXINFO:
            switch (settings.type) {
                case Windows::CaptionHarness::Basic:
                case Windows::CaptionHarness::ClientTitle:
                    if (MINMAXINFO * mmi = reinterpret_cast <MINMAXINFO *> (lParam)) {
                        
                        LONG width = settings.margins.left
                                   + settings.margins.right
                                   + 2 * GetSystemMetrics (SM_CXFRAME);
                        LONG height = settings.margins.top
                                    + settings.margins.bottom
                                    + 2 * GetSystemMetrics (SM_CYFRAME);
                        
                        if (settings.type == Windows::CaptionHarness::Basic) {
                            height += GetSystemMetrics (SM_CYCAPTION);
                        };
                        
                        if (mmi->ptMinTrackSize.x < width)
                            mmi->ptMinTrackSize.x = width;
        
                        if (mmi->ptMinTrackSize.y < height)
                            mmi->ptMinTrackSize.y = height;
                        
                        *result = 0;
                        return true;
                    } else
                        return false;

                case Windows::CaptionHarness::JustGlass:
                case Windows::CaptionHarness::FullGlass:
                    break;
            };
            break;
        
        // WM_CTLCOLORSTATIC
        //  - for a convenience lets have static controls transparent
        //    (because rendering cool extended border is the whole point)
        //  - falls through to WM_CTLCOLORBTN

        case WM_CTLCOLORSTATIC:
            SetBkMode ((HDC) wParam, TRANSPARENT);
        
        // WM_CTLCOLORBTN
        //  - render background for any common controls that use these
        //    messages (all standard and most custom do)
        
        case WM_CTLCOLORBTN: {
            RECT rWindow;
            RECT rControl;
            GetWindowRect (hWnd, &rWindow);
            GetWindowRect ((HWND) lParam, &rControl);
            
            // shift rectangles to position the control into "client" area
            
            OffsetRect (&rWindow, -rControl.left, -rControl.top);
            OffsetRect (&rControl, -rControl.left, -rControl.top);

            switch (settings.type) {
                case Windows::CaptionHarness::JustGlass:
                case Windows::CaptionHarness::ClientTitle:
                    break;
                case Windows::CaptionHarness::Basic:
                case Windows::CaptionHarness::FullGlass:
                    int hh = GetSystemMetrics (SM_CYCAPTION) + GetSystemMetrics (SM_CYFRAME);
                    OffsetRect (&rWindow, 0, hh);
                    OffsetRect (&rControl, 0, hh);
                    break;
            };
            
            BackgroundFill (hWnd, (HDC) wParam, &rWindow, &rControl,
                            settings, true);
            
            // the background is rendered here, this effectively tells
            // the control not to erase background any further
            
            *result = (LRESULT) GetStockObject (HOLLOW_BRUSH);
        } return true;// */
        
        // WM_ERASEBKGND
        //  - this one is easy, just paint the background
        
        case WM_ERASEBKGND: {
            RECT rcClient;
            if (GetClientRect (hWnd, &rcClient)) {
                
                bool caption = false;
                switch (settings.type) {
                    case Windows::CaptionHarness::Basic:
                    case Windows::CaptionHarness::FullGlass:
                        caption = true;
                        break;
                    case Windows::CaptionHarness::JustGlass:
                    case Windows::CaptionHarness::ClientTitle:
                        break;
                };

                *result =
                BackgroundFill (hWnd, (HDC) wParam, &rcClient, &rcClient,
                                settings, caption);
            };
        } return true;// */
    };
    
    return false;
};

namespace {

// BaseFill
// LiteFill
// AeroFill
//  - separate rendering for each Windows' window renderer/manager

bool BaseFill (HWND, HDC, const RECT *, const RECT *,
               const Windows::CaptionHarness::Settings &, bool);
bool LiteFill (HWND, HDC, const RECT *, const RECT *,
               const Windows::CaptionHarness::Settings &, bool);
bool AeroFill (HWND, HDC, const RECT *, const RECT *,
               const Windows::CaptionHarness::Settings &, bool);

// BackgroundFill
//  - calls appropriate rendering function from those above based on current
//    window renderer/manager

bool BackgroundFill (HWND hWnd, HDC hDC,
                     const RECT * rcArea, const RECT * rcClip,
                     const Windows::CaptionHarness::Settings & settings,
                     bool caption) {
    
    // type/function
    //  - internal table of different renderers for different window managers
    
    typedef bool (* type) (HWND, HDC, const RECT *, const RECT *,
                           const Windows::CaptionHarness::Settings &, bool);
    static const type function [] = {
        BaseFill,
        LiteFill,
        AeroFill
    };
    
    // dispatch the call
    //  - this is way better (smaller & faster code) than "switch" in GCC 4.5.1

    switch (settings.type) {
        case Windows::CaptionHarness::ClientTitle:
        case Windows::CaptionHarness::JustGlass:
            return function [Windows::UxTheme::Current ()]
                            (hWnd, hDC, rcArea, rcClip, settings, caption);
            
        case Windows::CaptionHarness::Basic:
        case Windows::CaptionHarness::FullGlass: // TODO: on BaseFill and LiteFill fill completely
            RECT rA = *rcArea;
            RECT rC = *rcClip;
            UINT n = GetSystemMetrics (SM_CYCAPTION) + GetSystemMetrics (SM_CYFRAME);
            
            rA.top -= n;
            rC.top -= n;
            
            return function [Windows::UxTheme::Current ()]
                            (hWnd, hDC, &rA, &rC, settings, caption);
    };
    
    return false;
};

bool BaseFill (HWND hWnd, HDC hDC,
               const RECT * rcArea, const RECT * rcClip,
               const Windows::CaptionHarness::Settings & settings,
               bool caption) {

    RECT rcIntersection;
    RECT rcFill = *rcArea;
    
    // fill the window background below the caption
    //  - always, because we already messed-up the upper border
    //  - TODO: discover proper handling for the -2 condition right below
    
    rcFill.bottom = settings.margins.top
                  + GetSystemMetrics (SM_CYFRAME);
    if (!caption) {
        rcFill.top = GetSystemMetrics (SM_CYCAPTION)
                   + GetSystemMetrics (SM_CYFRAME) - 1;
    };
    if (IntersectRect (&rcIntersection, &rcFill, rcClip)) {
        FillRect (hDC, &rcIntersection, GetSysColorBrush (COLOR_BTNFACE));
    };
    
    // left extended border 

    if (settings.margins.left > 0) {
        RECT rcFill = *rcArea;
        rcFill.right = rcFill.left + settings.margins.left;

        if (!caption) {
            rcFill.top = GetSystemMetrics (SM_CYFRAME)
                       + settings.margins.top;
        };
        if (IntersectRect (&rcIntersection, &rcFill, rcClip)) {
            FillRect (hDC, &rcIntersection, GetSysColorBrush (COLOR_BTNFACE));
        };
    };
    
    // right extended border

    if (settings.margins.right > 0) {
        RECT rcFill = *rcArea;
        rcFill.left = rcFill.right - settings.margins.right;

        if (!caption) {
            rcFill.top = GetSystemMetrics (SM_CYFRAME)
                       + settings.margins.top;
        };
        if (IntersectRect (&rcIntersection, &rcFill, rcClip)) {
            FillRect (hDC, &rcIntersection, GetSysColorBrush (COLOR_BTNFACE));
        };
    };
    
    // bottom extended border

    if (settings.margins.bottom > 0) {
        RECT rcFill = *rcArea;
        rcFill.top = rcFill.bottom - settings.margins.bottom;
        
        if (IntersectRect (&rcIntersection, &rcFill, rcClip)) {
            FillRect (hDC, &rcIntersection, GetSysColorBrush (COLOR_BTNFACE));
        };
    };
    
    // render window caption
    //  - this is used for controls
    //  - caption must be rendered last
    
    if (caption) {
        RECT rcCaption = *rcArea;
        rcCaption.top += GetSystemMetrics (SM_CYFRAME);// - 1;
        rcCaption.left += GetSystemMetrics (SM_CXFRAME);
        rcCaption.right -= 3 * GetSystemMetrics (SM_CXSIZE) + 2;
        rcCaption.bottom = rcCaption.top
                         + GetSystemMetrics (SM_CYCAPTION) - 1;
        
        if (IntersectRect (&rcIntersection, &rcCaption, rcClip)) {
            
            DWORD flags = DC_TEXT;
            BOOL gradient = FALSE;
            
            if (SystemParametersInfo (SPI_GETGRADIENTCAPTIONS,
                                      0, &gradient, 0)) {
                if (gradient) {
                    flags |= DC_GRADIENT;
                };
            };
            if (GetProp (hWnd, szPropActive)/*hWnd == GetForegroundWindow ()*/) {
                flags |= DC_ACTIVE;
            };
            if (caption) {
                flags |= DC_ICON;
            };
            DrawCaption (hWnd, hDC, &rcCaption, flags);
        };
    };
    
    return true;
};

bool LiteFill (HWND hWnd, HDC hDC,
               const RECT * rcArea, const RECT * rcClip,
               const Windows::CaptionHarness::Settings & settings,
               bool caption) {
    
    if (HANDLE hTheme = Windows::UxTheme::Open (hWnd, L"Window")) {

//        bool active = GetForegroundWindow () == hWnd;
        bool active = GetProp (hWnd, szPropActive);
        bool maximized = IsZoomed (hWnd);

        UINT type;
        if (active)
            type = 1;
        else
            type = 2;
        
        // upper extended border
        //  - extend left and right borders into extended caption
        
        if (settings.margins.top >
                GetSystemMetrics (SM_CYCAPTION) - GetSystemMetrics (SM_CYFRAME)) {
                    
            RECT rcLeft = {
                rcArea->left
                    - GetSystemMetrics (SM_CXFRAME),
                rcArea->top
                    + GetSystemMetrics (SM_CYCAPTION)
                    + GetSystemMetrics (SM_CYFRAME),
                rcArea->left
                    + (long) settings.split,
                rcArea->bottom
            };
            RECT rcRight = {
                rcArea->left
                    + (long) settings.split,
                rcArea->top
                    + GetSystemMetrics (SM_CYCAPTION)
                    + GetSystemMetrics (SM_CYFRAME),
                rcArea->right
                    + GetSystemMetrics (SM_CXFRAME),
                rcArea->bottom
            };
            RECT rcCaptionClip = *rcClip;
            rcCaptionClip.bottom = rcClip->top
                                 + GetSystemMetrics (SM_CYFRAME)
                                 + GetSystemMetrics (SM_CYCAPTION)
                                 + settings.margins.top;
            
            Windows::UxTheme::DrawBackground (hTheme, hDC, 7, // WP_FRAMELEFT
                                              type, &rcLeft, &rcCaptionClip);
            Windows::UxTheme::DrawBackground (hTheme, hDC, 8, // WP_FRAMERIGHT
                                              type, &rcRight, &rcCaptionClip);
        };
        
        // left extended border

        if (settings.margins.left > 0) {
            RECT rcLeft = {
                rcArea->left
                    - GetSystemMetrics (SM_CXFRAME),
                rcArea->top
                    + GetSystemMetrics (SM_CYCAPTION)
                    + GetSystemMetrics (SM_CYFRAME),
                rcArea->left
                    + settings.margins.left
                    + GetSystemMetrics (SM_CXFRAME),
                rcArea->bottom
            };
            
            Windows::UxTheme::DrawBackground (hTheme, hDC, 7, // WP_FRAMELEFT
                                              type, &rcLeft, rcClip);
        };
        
        // right extended border

        if (settings.margins.right > 0) {
            RECT rcRight = {
                rcArea->right
                    - settings.margins.right
                    - GetSystemMetrics (SM_CXFRAME),
                rcArea->top
                    + GetSystemMetrics (SM_CYCAPTION)
                    + GetSystemMetrics (SM_CYFRAME),
                rcArea->right
                    + GetSystemMetrics (SM_CXFRAME),
                rcArea->bottom
            };
            
            Windows::UxTheme::DrawBackground (hTheme, hDC, 8, // WP_FRAMERIGHT
                                              type, &rcRight, rcClip);
        };
        
        // bottom extended border
        
        if (settings.margins.bottom > 0) {
            RECT rcBottom = {
                rcArea->left
                    - GetSystemMetrics (SM_CXFRAME),
                rcArea->bottom
                    - settings.margins.bottom,
                rcArea->right
                    + GetSystemMetrics (SM_CXFRAME),
                rcArea->bottom
                    + GetSystemMetrics (SM_CYFRAME)
            };

            Windows::UxTheme::DrawBackground (hTheme, hDC, 9, // WP_FRAMEBOTTOM
                                              type, &rcBottom, rcClip);
        };

        // render window caption
        //  - this is used for controls
        //  - caption must be rendered last
        
        if (caption) {
            RECT rcCaption = *rcArea;
            rcCaption.bottom = rcCaption.top
                             + GetSystemMetrics (SM_CYCAPTION)
                             + GetSystemMetrics (SM_CYFRAME);

            if (maximized)
                rcCaption.top += GetSystemMetrics (SM_CYFRAME);
            
            RECT rcIntersection;
            if (IntersectRect (&rcIntersection, &rcCaption, rcClip)) {
                UINT part = 1; // WP_CAPTION
                if (maximized) {
                    part = 5; // WP_MAXCAPTION
                };
                Windows::UxTheme::DrawBackground (hTheme, hDC, part, type,
                                                  &rcCaption, rcClip);
            };
        };

        Windows::UxTheme::Close (hTheme);
        return true;
    } else
        return BaseFill (hWnd, hDC, rcArea, rcClip, settings, caption);
};

bool AeroFill (HWND, HDC hDC, const RECT *, const RECT * rcClip,
               const Windows::CaptionHarness::Settings &, bool) {
    
    // Aero
    //  - just black-erase (setting transparent) the clipping rectangle
    //  - no need to confine anyhow, because window frame is rendered by DWM

    return FillRect (hDC, rcClip, (HBRUSH) GetStockObject (BLACK_BRUSH));
};

};
