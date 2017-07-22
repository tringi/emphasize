#include "Windows_SetPixelEx.hpp"

/* Emphasize Windows SetPixelEx GDI extension
// Windows_SetPixelEx.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      15.10.2011 - initial version
*/

#include "Windows_CreateSolidBrushEx.hpp"

bool Windows::SetPixelEx (HDC hDC, int x, int y, COLORREF color) {
    bool result = false;
    
    if (HBRUSH hBrush = Windows::CreateSolidBrushEx (color)) {
        result = Windows::SetPixelEx (hDC, x, y, hBrush);
        DeleteObject (hBrush);
    };
    
    return result;
};
bool Windows::SetPixelEx (HDC hDC, int x, int y,
                          COLORREF color, unsigned char alpha) {
    bool result = false;
    
    if (HBRUSH hBrush = Windows::CreateSolidBrushEx (color, alpha)) {
        result = Windows::SetPixelEx (hDC, x, y, hBrush);
        DeleteObject (hBrush);
    };
    
    return result;
};

bool Windows::SetPixelEx (HDC hDC, int x, int y, HBRUSH hBrush) {
    RECT r = { x, y, x + 1, y + 1 };
    return FillRect (hDC, &r, hBrush);
};

