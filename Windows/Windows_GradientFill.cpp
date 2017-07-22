#include "Windows_GradientFill.hpp"

/* Emphasize Windows GradientFill GDI wrapper
// Windows_GradientFill.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      21.11.2011 - initial version
*/

#include "Windows_Symbol.hpp"

namespace {
    BOOL (WINAPI * GdiGradientFill) (HDC, PTRIVERTEX, ULONG,
                                     PVOID, ULONG, ULONG) = NULL;
};

bool Windows::GradientFill (HDC hDC, const RECT & r, UINT mode,
                            COLORREF color1, COLORREF color2) {
    if (GdiGradientFill == NULL) {
        Windows::Symbol (L"gdi32", GdiGradientFill, "GdiGradientFill");
    };
    
    if (GdiGradientFill) {
        switch (mode) {
            case HS_HORIZONTAL:
                mode = GRADIENT_FILL_RECT_H;
                break;
            case HS_VERTICAL:
                mode = GRADIENT_FILL_RECT_V;
                break;
            default:
                return false;
        };

        GRADIENT_RECT grc = {
            0, 1
        };
        TRIVERTEX tvi [2u] = {
            {
                r.left, r.top,
                (COLOR16) (GetRValue (color1) << 8),
                (COLOR16) (GetGValue (color1) << 8),
                (COLOR16) (GetBValue (color1) << 8),
                0
            },
            {
                r.right, r.bottom,
                (COLOR16) (GetRValue (color2) << 8),
                (COLOR16) (GetGValue (color2) << 8),
                (COLOR16) (GetBValue (color2) << 8),
                0
            }
        };
        
        return GdiGradientFill (hDC, tvi, 2u, &grc, 1u, mode);
    } else
        return false;
};
