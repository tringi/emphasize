#include "Shadow_Chart.hpp"

/* Shadow Chart 
// Shadow_Chart.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      26.05.2014 - initial version
*/

#include <cwchar>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "../ext/interpolation"

//using namespace Windows;
using namespace Shadow::Chart;

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    LRESULT SendRequest (HWND hWnd, UINT code, NMHDR * nm = NULL);
};

ATOM Shadow::Chart::Initialize (HINSTANCE hInstance) {
    if (!atom) {
        WNDCLASS wc;
        
        wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = Procedure;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TEXT ("Shadow::Chart");

        atom = RegisterClass (&wc);
    };
    return atom;
};

HWND Shadow::Chart::Create (HINSTANCE hInstance, HWND hParent, UINT id) {
    return CreateWindow ((LPCTSTR) (INT) atom, TEXT(""),
                         WS_CHILD | WS_CLIPSIBLINGS, 0,0,0,0,
                         hParent, (HMENU) id, hInstance, NULL);
};

namespace {
HFONT hFont = (HFONT) GetStockObject (SYSTEM_FONT); // TODO: per window

LRESULT OnCreate (HWND, LPVOID);
LRESULT OnPaint (HWND, HDC, RECT);

LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return OnCreate (hWnd, reinterpret_cast <const CREATESTRUCT *>
                                                    (lParam) ->lpCreateParams);
            
        case WM_SIZE:
        case WM_STYLECHANGED:
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            InvalidateRect (hWnd, NULL, FALSE);
            break;
        
        case WM_SETFONT:
            hFont = (HFONT) wParam;
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            if (HDC hDC = BeginPaint (hWnd, &ps)) {
                
                // TODO: Add this double-buffering to Windows::UxTheme
                
                RECT rc;
                bool offscreen = false;
                
                if (GetClientRect (hWnd, &rc)) {
                    HDC hOffDC = CreateCompatibleDC (hDC);
                    HBITMAP hBitmap = CreateCompatibleBitmap (hDC, rc.right, rc.bottom);
                    
                    if (hOffDC && hBitmap) {
                        if (auto hOld = SelectObject (hOffDC, hBitmap)) {
                            
                            OnPaint (hWnd, hOffDC, ps.rcPaint);
                            if (BitBlt (hDC,
                                        ps.rcPaint.left, ps.rcPaint.top,
                                        ps.rcPaint.right - ps.rcPaint.left,
                                        ps.rcPaint.bottom - ps.rcPaint.top,
                                        hOffDC,
                                        ps.rcPaint.left, ps.rcPaint.top,
                                        SRCCOPY)) {
                                
                                offscreen = true;
                            };
                            SelectObject (hOffDC, hOld);
                        };
                        
                        if (hBitmap)
                            DeleteObject (hBitmap);
                        if (hOffDC)
                            DeleteDC (hOffDC);
                    };
                };
                
                if (!offscreen) {
                    OnPaint (hWnd, hDC, ps.rcPaint);
                };
            };
            EndPaint (hWnd, &ps);
        } break;
        case WM_PRINTCLIENT: {
            RECT rc;
            if (GetClientRect (hWnd, &rc)) {
                OnPaint (hWnd, (HDC) wParam, rc);
            };
        } break;

        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT OnCreate (HWND /*hWnd*/, LPVOID /*param*/) {
//    SetWindowLongPtr (hWnd, 0, (LONG_PTR) param);
    return 0;
};

// magnitude
//  - decimal not-really-a-magnitude of a number
//  - whole numbers are considered of lower magnitude
//     - e.g. for 100.0 the result is 10.0

template <typename T>
T magnitude (T value) {
    T m = T(1);
    if (value > T(1)) {
        while ((value /= T(10)) > T(1)) {
            m *= T(10);
        };
    } else {
        do {
            m /= T(10);
        } while ((value *= T(10)) <= T(1));
    };
    return m;
};
template <typename T>
int sgn (T value) {
    return (T(0) < value) - (value < T(0));
};

std::vector <std::pair <float, float>> storage;

LRESULT OnPaint (HWND hWnd, HDC hDC, RECT rDraw) {
    FillRect (hDC, &rDraw, (HBRUSH)
              SendMessage (GetParent (hWnd), WM_CTLCOLORSTATIC,
                           (WPARAM) hDC, (LPARAM) hWnd));
    RECT r;
    GetClientRect (hWnd, &r);

    Shadow::Chart::Layout layout;
    std::memset (&layout, 0, sizeof layout);
    
    if (SendRequest (hWnd, Shadow::Chart::Request::Layout, &layout.nm)) {
        const ext::interpolation <float, long> inY (layout.y.minimum, layout.y.maximum,
                                                    r.top + layout.margin.top, r.bottom - layout.margin.bottom);
        const ext::interpolation <float, long> inX (layout.x.minimum, layout.x.maximum,
                                                    r.left + layout.margin.left, r.right - layout.margin.right);
        const ext::interpolation <long, float> reX (r.left + layout.margin.left, r.right - layout.margin.right,
                                                    layout.x.minimum, layout.x.maximum);

        const auto dcPen = GetStockObject (DC_PEN);
        const auto prevPen = SelectObject (hDC, dcPen);
        const auto prevFont = SelectObject (hDC, hFont);
        const auto prevBrush = SelectObject (hDC, GetStockObject (HOLLOW_BRUSH));
        
        SetBkMode (hDC, TRANSPARENT);
        SetDCBrushColor (hDC, 0x00FFFFFF);

        // Y grid
        
        Shadow::Chart::Marker marker;
        std::memset (&marker, 0, sizeof marker);
        
        while (SendRequest (hWnd, Shadow::Chart::Request::Marker, &marker.nm)) {
            const auto y = inY (marker.y);
            
            SetDCPenColor (hDC, marker.color);
            MoveToEx (hDC, r.left + layout.margin.left - 2, y, NULL);
            LineTo   (hDC, r.right - layout.margin.right + 2, y);

            SetTextColor (hDC, marker.textcolor);
            SetTextAlign (hDC, TA_RIGHT | TA_BASELINE);
            
            TextOut (hDC, r.left + layout.margin.left - 4, y + 4,
                          marker.text, std::wcslen (marker.text));
            
            ++marker.index;
        };
        
        // X grid
        //  - moving along with X axis values, TODO: jak?
        //  - compute three levels of grid lines
        
        const auto xspan = std::abs (layout.x.maximum - layout.x.minimum);
        const auto xgrid = magnitude (xspan);
        const auto xsubg = xgrid / 5.0f;

        long xp = -1000;
        long xs = -1000;
        for (auto x = r.left + layout.margin.left;
                  x < r.right - layout.margin.right; ++x) {

            long xf = reX (x) / xgrid;
            long xsf = reX (x) / xsubg;
            
            if (xp != xf) {
                SetDCPenColor (hDC, 0x00AAAAAA);
                MoveToEx (hDC, x, r.top + layout.margin.top, NULL);
                LineTo   (hDC, x, r.bottom - layout.margin.bottom);
            } else {
                if (xs != xsf) {
                    SetDCPenColor (hDC, 0x00DDDDDD);
                    MoveToEx (hDC, x, r.top + layout.margin.top + 2, NULL);
                    LineTo   (hDC, x, r.bottom - layout.margin.bottom - 2);
                };
                xs = xsf;
            };
            xp = xf;
            xs = xsf;
        };
        
        // fix
        SetDCPenColor (hDC, 0x00AAAAAA);
        if (inX (0) > r.left + layout.margin.left && inX (0) < r.right - layout.margin.right) {
            MoveToEx (hDC, inX (0), r.top + layout.margin.top, NULL);
            LineTo   (hDC, inX (0), r.bottom - layout.margin.bottom);
        };

        SetDCPenColor (hDC, 0);
        MoveToEx (hDC, r.left + layout.margin.left, r.top + layout.margin.top, NULL);
        LineTo   (hDC, r.left + layout.margin.left, r.bottom - layout.margin.bottom);
        
        // charts
        
        for (auto chart = 0u; chart != layout.charts; ++chart) {

            Shadow::Chart::Info info;
            std::memset (&info, 0, sizeof info);
            
            info.chart = chart;
            info.width = 1u;
            info.data = &storage;
            storage.clear ();
            
            if (SendRequest (hWnd, Shadow::Chart::Request::Info, &info.nm)) {
                
                if (!storage.empty ()) {
                    
                    // pen for the chart
                    //  - fall back to DC pen on error
                    
                    HPEN hPen = CreatePen (PS_SOLID, info.width, info.color);
                    if (hPen) {
                        SelectObject (hDC, hPen);
                    } else {
                        SetDCPenColor (hDC, info.color);
                    };
                    
                    // draw
                    //  - clip to prevent line pixels overreaching into margins
    
                    SaveDC (hDC);
                    IntersectClipRect (hDC, r.left + layout.margin.left + 1,
                                            r.top + layout.margin.top,
                                            r.right - layout.margin.right,
                                            r.bottom - layout.margin.bottom);
                    
                    // convert in groups of 1024 points
                    
                    const auto M = 1024;
                    const auto n = storage.size () - 1u;
                    POINT pts [M];
                    
                    MoveToEx (hDC, inX (storage[0].first), inY (storage[0].second), NULL);
                    
                    for (auto chunk = 0u; chunk != n / M; ++chunk) {
                        for (auto i = 0u; i != M; ++i) {
                            pts [i] .x = inX (storage[chunk * M + i + 1].first);
                            pts [i] .y = inY (storage[chunk * M + i + 1].second);
                        };
                        PolylineTo (hDC, pts, M);
                    };
                    for (auto i = 0u; i != n % M; ++i) {
                        pts [i] .x = inX (storage[(n / M) * M + i + 1].first);
                        pts [i] .y = inY (storage[(n / M) * M + i + 1].second);
                    };
                    PolylineTo (hDC, pts, n % M);
                    
                    RestoreDC (hDC, -1);
                    
                    // free pen (if any)
                    
                    if (hPen) {
                        SelectObject (hDC, dcPen);
                        DeleteObject (hPen);
                    };
                };
            };
        };

        SelectObject (hDC, prevBrush);
        SelectObject (hDC, prevFont);
        SelectObject (hDC, prevPen);
    };
    return 0;
};

LRESULT SendRequest (HWND hWnd, UINT code, NMHDR * nm) {
    NMHDR temp;
    if (!nm)
        nm = &temp;
    
    nm->hwndFrom = hWnd;
    nm->idFrom = GetDlgCtrlID (hWnd);
    nm->code = code;
    
    return SendMessage (GetParent (hWnd), WM_NOTIFY, nm->idFrom, (LPARAM) nm);
};
};
