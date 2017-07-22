#include "Shadow_Graph.hpp"

/* Emphasize Shadow Controls Library - Graph
// Shadow_Graph.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      06.03.2012 - initial version
*/

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cwchar>

#include <vector>
#include <map>

#include "../ext/interpolation"

#include "../Windows/Windows_W2kCompatibility.hpp"
#include "../Windows/Windows_UxTheme.hpp"

using namespace Windows;
using namespace Shadow::Graph;

namespace {
    ATOM atom = 0u;
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
    
    static const RECT rNul = { 0,0,0,0 };
};

ATOM Shadow::Graph::Initialize (HINSTANCE hInstance) {
    if (!atom) {
        WNDCLASS wc;
        
        wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = Procedure;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof (Shadow::Graph::Interface *);
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TEXT ("Shadow::Graph");

        atom = RegisterClass (&wc);
    };
    return atom;
};

HWND Shadow::Graph::Create (HINSTANCE hInstance, HWND hParent,
                            UINT id, Shadow::Graph::Interface & i,
                            DWORD style, DWORD extra) {
    
    return CreateWindowEx (extra, (LPCTSTR) (INT) atom, TEXT(""),
                           style | WS_CHILD | WS_CLIPSIBLINGS, 0,0,0,0,
                           hParent, (HMENU) id, hInstance, &i);
};

RECT         Shadow::Graph::Interface::GraphMargin () const { return rNul; };
unsigned int Shadow::Graph::Interface::GraphCount () const { return 0u; };
double       Shadow::Graph::Interface::GraphStart () const { return 0.0; };
double       Shadow::Graph::Interface::GraphEnd() const { return 1.0; };
std::wstring Shadow::Graph::Interface::GraphNote (unsigned int, double, unsigned int) const { return L""; };
double       Shadow::Graph::Interface::GraphNoteWidth (unsigned int) const { return 0.0; };
std::wstring Shadow::Graph::Interface::GraphAxisNote (double) const { return L""; };
double       Shadow::Graph::Interface::GraphGridStep (unsigned int) const { return 0.0; };
bool         Shadow::Graph::Interface::GraphValidGridRow (unsigned int, double) const { return true; };
double       Shadow::Graph::Interface::GraphExtraGridRow (unsigned int, unsigned int) const { return 0.0; };
double       Shadow::Graph::Interface::GraphRightGridRow (unsigned int, unsigned int) const { return 0.0; };
bool         Shadow::Graph::Interface::GraphSampleValid (unsigned int, unsigned int) const { return true; };
bool         Shadow::Graph::Interface::GraphSampleBreak (unsigned int, unsigned int) const { return false; };
const wchar_t * Shadow::Graph::Interface::GraphUnits (unsigned int) const { return L""; };

namespace {
HFONT hFont = (HFONT) GetStockObject (SYSTEM_FONT); // TODO: !!!
    
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

        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    };
    
    return 0;
};

LRESULT OnCreate (HWND hWnd, LPVOID param) {
    SetWindowLongPtr (hWnd, 0, (LONG_PTR) param);
    return 0;
};

void OnPaintFinishBezier (HDC hDC, std::vector <POINT> & points, int ds, bool bz) {
    static const double coef = 0.1;
    
    if (!points.empty ()) {
        if (!bz) {
            Polyline (hDC, &*points.begin (), points.size ());
            
        } else {
            std::vector <POINT> bezier;
    
            unsigned int pi = 1u;
            unsigned int pe = points.size ();
    
            bezier.push_back (points [0u]);
            bezier.push_back (points [0u]);
            for (; pi < pe; ++pi) {
    
                if (pi > 1u) {
                    ext::interpolation <double, double> piX (0.0, 1.0, points[pi-2].x, points[pi-1].x);
                    ext::interpolation <double, double> piY (0.0, 1.0, points[pi-2].y, points[pi-1].y);
    
                    bezier.push_back ({ (long) piX (1.0+coef), (long) piY (1.0+coef) });
                };
    
                if (pi != pe - 1u) {
                    ext::interpolation <double, double> piX (0.0, 1.0, points[pi].x, points[pi+1].x);
                    ext::interpolation <double, double> piY (0.0, 1.0, points[pi].y, points[pi+1].y);
    
                    bezier.push_back ({ (long) piX (-coef), (long) piY (-coef) });
                };
    
                bezier.push_back (points[pi]);
            };
            bezier.push_back (points[pe - 1u]);
            PolyBezier (hDC, &*bezier.begin (), bezier.size ());
        };
        
        for (auto point : points) {
            POINT diamond [] = {
                { point.x +  0, point.y - ds },
                { point.x + ds, point.y +  0 },
                { point.x +  0, point.y + ds },
                { point.x - ds, point.y +  0 },
            };
            Polygon (hDC, diamond, 4);
        };
        
        points.clear ();
    };
    return;
};

LRESULT OnPaint (HWND hWnd, HDC _hDC, RECT) {
    using namespace Shadow::Graph;
    auto cb = reinterpret_cast <Interface *> (GetWindowLongPtr (hWnd, 0));
    
    RECT rc;
    GetClientRect (hWnd, &rc);

    const RECT margin = cb->GraphMargin ();
    const unsigned int graphs = cb->GraphCount ();
    
    HDC hDC = NULL;
    HANDLE hBuffered = UxTheme::BeginBufferedPaint (_hDC, &rc, &hDC);

    if (!hBuffered)
        hDC = _hDC;

    HGDIOBJ hPrevPen = SelectObject (hDC, GetStockObject (DC_PEN));
    HGDIOBJ hPrevBru = SelectObject (hDC, GetStockObject (HOLLOW_BRUSH));
    HGDIOBJ hPrevFon = SelectObject (hDC, hFont);
    
    auto overflow = margin.top / 2u;
    
    const auto start = cb->GraphStart ();
    const auto end   = cb->GraphEnd ();
    
    SetBkMode (hDC, TRANSPARENT);
    FillRect (hDC, &rc, (HBRUSH) GetStockObject (WHITE_BRUSH));
    
    auto px_start = margin.left + 1;
    auto px_end = rc.right - margin.right;

    ext::interpolation <unsigned int, double> inGX (px_start, px_end, start, end);
    ext::interpolation <double, int> inX (start, end, px_start, px_end);

    // vertical grid

    {   unsigned int minimaldiff = 0u;
        unsigned int differences = 0u;
        double difference [5u];
        double previous [5u];
        int noteright = 0u;
        
        for (auto i = 0u; i < sizeof difference / sizeof difference [0]; ++i) {
            if ((difference [i] = cb->GraphGridStep (i))) {
                previous [i] = start;
                ++differences;
                
                if (inGX (px_start + 4u) - inGX (px_start) >= difference [i])
                    ++minimaldiff;
            } else
                break;
        };
    
        for (auto px = px_start; px < px_end; ++px) {
            auto draw = 0u;
            auto gx = inGX (px);
            
            for (auto i = minimaldiff; i < differences; ++i)
                if (previous [i] < gx - difference [i]) {
                    previous [i] += difference [i];
                    draw = i + 1 - minimaldiff;
                    SetDCPenColor (hDC, RGB (0xFF - (8 << draw),
                                             0xFF - (8 << draw),
                                             0xFF - (8 << draw)));
                };
            
            if (draw) {
                MoveToEx (hDC, px, margin.top - 4 * draw, NULL);
                LineTo   (hDC, px, rc.bottom - margin.bottom + 4 * draw);
                
                // grid note
                
                if (draw >= 2) {
                    std::wstring note = cb->GraphAxisNote (gx);
                    RECT rText = { 0,0,0,0 };
                    DrawText (hDC, note.c_str (), note.length (),
                              &rText, DT_CALCRECT | DT_NOPREFIX);
                    
                    rText.top = rc.bottom - 3 * margin.bottom / 4;
                    rText.left = px - rText.right / 2;
                    rText.right = px + rText.right / 2 + 4;
                    rText.bottom += rText.top;
                    
                    if (noteright < rText.left) {
                        noteright = rText.right;
                        
                        DrawText (hDC, note.c_str (), note.length (),
                                  &rText, DT_CENTER | DT_NOPREFIX);
                    };
                };
            };
        };
    };
    
    // texts
    //  - reduce until the text fits

    {   unsigned int ni = 0u;
        double time = start;

        while (time < end) {

            unsigned int r = 0u;
            const double width = cb->GraphNoteWidth (ni);
            std::wstring note;

            bool draw = true;
            RECT rText = { 0,0,0,0 };

            do {
                note = cb->GraphNote (ni, time, r++);
                if (!note.length ()) {
                    draw = false;
                    break;
                };

                DrawText (hDC, note.c_str (), note.length (),
                          &rText, DT_CALCRECT | DT_NOPREFIX);
            } while (rText.right > (inX (time+width) - inX(time) - 12));

            if (draw) {
                auto th = rText.bottom - rText.top;
                rText.top = margin.top - th / 2;
                rText.left = inX(time);
                rText.right = inX(time+width);
                rText.bottom = rText.top + th;

                DrawText (hDC, note.c_str (), note.length (),
                          &rText, DT_CENTER | DT_NOPREFIX);
            };

            time += width;
            ++ni;
        };
    };
    
    // horizontal graph lines
    
    {   std::map <int, std::pair <COLORREF, std::wstring>> hgrid;
        std::map <int, std::pair <COLORREF, std::wstring>> hrgrid;

        for (unsigned int gi = 0u; gi < graphs; ++gi) {
            const COLORREF cr = cb->GraphColor (gi);
            const double maximum = cb->GraphMaximum (gi);
            const double minimum = cb->GraphMinimum (gi);
            const double difference = maximum - minimum;
            double rank;
    
                 if (difference > 200.0) rank = 100.0;
            else if (difference > 100.0) rank = 50.0;
            else if (difference > 50.0) rank = 25.0;
            else if (difference > 20.0) rank = 10.0;
            else if (difference > 10.0) rank = 5.0;
            else if (difference > 4.0) rank = 2.0;
            else rank = 1.0;
    
            ext::interpolation <double, int>
                inY (cb->GraphMinimum (gi), 1.05 * cb->GraphMaximum (gi),
                     rc.bottom - margin.bottom, margin.top);
    
            wchar_t string [32];
            const wchar_t * units = cb->GraphUnits (gi);
            
            for (double ry = rank; ry < cb->GraphMaximum (gi); ry += rank) {
                if (cb->GraphValidGridRow (gi, ry)) {
                    snwprintf (string, 32, L"%.0f%s", ry, units);
                    hgrid [inY (ry)] = std::make_pair (cr, std::wstring (string));
                };
            };
            
            {   unsigned int extra = 0u;
                double extra_value = 0.0;
                while ((extra_value = cb->GraphExtraGridRow (gi, extra)) > 0.1) {
                    snwprintf (string, 32, L"%.1f%s", extra_value, units);
                    hgrid [inY (extra_value)] = std::make_pair (cr, std::wstring (string));
                    ++extra;
                };
            };
            {   unsigned int extra = 0u;
                double extra_value = 0.0;
                while ((extra_value = cb->GraphRightGridRow (gi, extra)) > 0.1) {
                    snwprintf (string, 32, L"%.1f%s", extra_value, units);
                    hrgrid [inY (extra_value)] = std::make_pair (cr, std::wstring (string));
                    ++extra;
                };
            };
        };
        
        int prevy = 0;
        for (const auto & hgy : hgrid) {
            SetDCPenColor (hDC, RGB ((GetRValue (hgy.second.first) + 3 * 0xFF) / 4u,
                                     (GetGValue (hgy.second.first) + 3 * 0xFF) / 4u,
                                     (GetBValue (hgy.second.first) + 3 * 0xFF) / 4u));
            MoveToEx (hDC, px_start - 8, hgy.first, NULL);
            LineTo   (hDC, px_end, hgy.first);
            
            SetTextColor (hDC, hgy.second.first);

            if (hgy.first <= prevy + 12) {
                SetTextAlign (hDC, TA_RIGHT | TA_TOP);
                prevy = hgy.first + 12;
            } else {
                SetTextAlign (hDC, TA_RIGHT | TA_BOTTOM);
                prevy = hgy.first;
            };
            
            TextOut (hDC, px_start - 2, hgy.first,
                     hgy.second.second.c_str (), hgy.second.second.length ());
        };

        prevy = 0;
        for (const auto & hgy : hrgrid) {
            SetDCPenColor (hDC, RGB ((GetRValue (hgy.second.first) + 3 * 0xFF) / 4u,
                                     (GetGValue (hgy.second.first) + 3 * 0xFF) / 4u,
                                     (GetBValue (hgy.second.first) + 3 * 0xFF) / 4u));
            MoveToEx (hDC, px_start - 8, hgy.first, NULL);
            LineTo   (hDC, px_end, hgy.first);

            SetTextColor (hDC, hgy.second.first);

            if (hgy.first <= prevy + 12) {
                SetTextAlign (hDC, TA_LEFT | TA_TOP);
                prevy = hgy.first + 12;
            } else {
                SetTextAlign (hDC, TA_LEFT | TA_BOTTOM);
                prevy = hgy.first;
            };

            TextOut (hDC, px_end - 2, hgy.first,
                     hgy.second.second.c_str (), hgy.second.second.length ());
        };
    };
    

    // render each graph

    SaveDC (hDC);
    IntersectClipRect (hDC, px_start, 0, px_end, rc.bottom - margin.bottom);
    
    for (unsigned int pass = 0u; pass < 2u; ++pass) {
        for (unsigned int _gi = 0u; _gi < graphs; ++_gi) {
            const unsigned int gi = graphs - _gi - 1u;
    
            const auto sn = cb->GraphSampleCount (gi);
            if (!sn)
                continue;
            
            int diamond_size = (px_end - px_start) / sn / 6;
            bool bz = diamond_size > 0;
            if (diamond_size > 8)
                diamond_size = 8;
            if (diamond_size < 2)
                diamond_size = 2;
            
            ext::interpolation <double, int>
                inY (cb->GraphMinimum (gi), 1.05 * cb->GraphMaximum (gi),
                     rc.bottom - margin.bottom, margin.top);
    
            const COLORREF cr = cb->GraphColor (gi);
            
            // graph
            
            std::vector <POINT> points;

            HPEN hPen = NULL;
            if (pass) {
                LOGBRUSH lb = { BS_SOLID, cr, 0u };
                hPen = ExtCreatePen (PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND,
                                     2u, &lb, 0u, NULL);
            } else {
                LOGBRUSH lb = { BS_HATCHED,
                                RGB ((GetRValue (cr) + 3 * 0xFF) / 4u,
                                     (GetGValue (cr) + 3 * 0xFF) / 4u,
                                     (GetBValue (cr) + 3 * 0xFF) / 4u),
                                gi ? HS_FDIAGONAL : HS_BDIAGONAL };
                hPen = ExtCreatePen (PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND,
                                     24u, &lb, 0u, NULL);
            };
            
            HGDIOBJ hPrev = SelectObject (hDC, hPen);
            
            for (auto si = 0u; si < sn; ++si)
                if (cb->GraphSampleValid (gi, si)) {
                    auto px = inX (cb->GraphSampleKey (gi, si));
                    auto py = inY (cb->GraphSampleValue (gi, si));
                    
                    if (!si || cb->GraphSampleBreak (gi, si)) {
                        OnPaintFinishBezier (hDC, points, diamond_size, bz);
                    };
                    points.push_back ({ px, py });
                };
                
            OnPaintFinishBezier (hDC, points, diamond_size, bz);
            
            if (hPrev)
                SelectObject (hDC, hPrev);
            DeleteObject (hPen);
        };

    };
    RestoreDC (hDC, -1);


    // main lines

    SelectObject (hDC, GetStockObject (DC_PEN));
    SetDCPenColor (hDC, 0x000000);
    
    MoveToEx (hDC, margin.left, rc.bottom - margin.bottom + overflow, NULL);
    LineTo   (hDC, margin.left, margin.top - overflow);
    MoveToEx (hDC, margin.left - overflow, rc.bottom - margin.bottom, NULL);
    LineTo   (hDC, rc.right - margin.right + overflow, rc.bottom - margin.bottom);
    
    if (hPrevPen) SelectObject (hDC, hPrevPen);
    if (hPrevBru) SelectObject (hDC, hPrevBru);
    if (hPrevFon) SelectObject (hDC, hPrevFon);
    
    if (hBuffered)
        UxTheme::EndBufferedPaint (hBuffered);
    
    return 0;
};
};
