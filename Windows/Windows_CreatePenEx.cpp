#include "Windows_CreatePenEx.hpp"

/* Emphasize Windows CreatePen GDI extension
// Windows_CreatePenEx.cpp
//
// Author: Jan Ringos, http://tringi.trimcore.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      11.12.2017 - initial version
*/

HPEN Windows::CreatePenEx (DWORD style, DWORD width, COLORREF color) {
    return Windows::CreatePenEx (style, width, color, unsigned (color) >> 24u);
};

HPEN Windows::CreatePenEx (DWORD style, DWORD width, COLORREF color, unsigned char alpha) {
    BITMAPINFO bi;

    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = 1u;
    bi.bmiHeader.biHeight = 1u;
    bi.bmiHeader.biPlanes = 1u;
    bi.bmiHeader.biBitCount = 32u;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0u;
    bi.bmiHeader.biXPelsPerMeter = 0u;
    bi.bmiHeader.biYPelsPerMeter = 0u;
    bi.bmiHeader.biClrUsed = 0u;
    bi.bmiHeader.biClrImportant = 0u;
    
    bi.bmiColors[0].rgbBlue     = MulDiv (GetBValue (color), alpha, 0xFFu);
    bi.bmiColors[0].rgbGreen    = MulDiv (GetGValue (color), alpha, 0xFFu);
    bi.bmiColors[0].rgbRed      = MulDiv (GetRValue (color), alpha, 0xFFu);
    bi.bmiColors[0].rgbReserved = alpha;

    LOGBRUSH logbrush;

    logbrush.lbStyle = BS_DIBPATTERNPT;
    logbrush.lbColor = DIB_RGB_COLORS;
    logbrush.lbHatch = (ULONG_PTR) &bi;

    style &= ~(PS_TYPE_MASK | PS_STYLE_MASK);
    style |= PS_GEOMETRIC | PS_SOLID;

    return ExtCreatePen (style, width, &logbrush, 0, NULL);
};



