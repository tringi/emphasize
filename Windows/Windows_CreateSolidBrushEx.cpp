#include "Windows_CreateSolidBrushEx.hpp"

/* Emphasize Windows CreateSolidBrushEx GDI extension
// Windows_CreateSolidBrushEx.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      14.10.2011 - initial version
*/

HBRUSH Windows::CreateSolidBrushEx (COLORREF color) {
    return Windows::CreateSolidBrushEx (color, unsigned (color) >> 24u);
};

HBRUSH Windows::CreateSolidBrushEx (COLORREF color, unsigned char alpha) {
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
    
    return CreateDIBPatternBrushPt (&bi, DIB_RGB_COLORS);
};



