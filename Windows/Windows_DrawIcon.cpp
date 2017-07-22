#include "Windows_DrawIcon.hpp"

/* Windows DrawIcon 
// Windows_DrawIcon.cpp
//
// Author: Jan Ringos, http://Tringi.TrimCore.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      02.04.2015 - initial version
*/

#include <cstring>
#include <cstdio>

bool Windows::GetIconSize (HICON hIcon, SIZE * size, UINT * bpp) {
    ICONINFO info;
    if (GetIconInfo (hIcon, &info)) {
        
        bool result = false;
        
        BITMAP bmp;
        std::memset (&bmp, 0, sizeof bmp);

        if (info.hbmColor) {
            if (GetObject (info.hbmColor, sizeof bmp, &bmp)) {
                if (size) {
                    size->cx = bmp.bmWidth;
                    size->cy = bmp.bmHeight;
                };
                if (bpp) {
                    *bpp = bmp.bmBitsPixel;
                };
                result = true;
            };
        } else
        if (info.hbmMask) {
            if (GetObject (info.hbmMask, sizeof bmp, &bmp)) {
                if (size) {
                    size->cx = bmp.bmWidth;
                    size->cy = bmp.bmHeight / 2;
                };
                if (bpp) {
                    *bpp = 1;
                };
                result = true;
            };
        };
    
        if (info.hbmColor)
            DeleteObject (info.hbmColor);
        if (info.hbmMask)
            DeleteObject (info.hbmMask);
        
        return result;
    } else
        return false;
};

namespace {
    bool DrawIconImplementation (HDC, HICON, const SIZE &, const RECT &, UCHAR);
};

bool Windows::DrawIcon (HDC hDC, HICON hIcon, int x, int y, UCHAR alpha) {
    
    if (alpha == 255u)
        return DrawIconEx (hDC, x, y, hIcon, 0, 0, 0, NULL, DI_NORMAL);
    
    SIZE size;
    if (Windows::GetIconSize (hIcon, &size)) {
        return DrawIconImplementation (hDC, hIcon, size, { x, y, size.cx, size.cy }, alpha);
    } else
        return false;
};

bool Windows::DrawIcon (HDC hDC, HICON hIcon, const RECT & r, UCHAR alpha) {
    SIZE size;
    if (Windows::GetIconSize (hIcon, &size)) {
        return DrawIconImplementation (hDC, hIcon, size, r, alpha);
    } else
        return false;
};

namespace {
bool DrawIconImplementation (HDC hDC, HICON hIcon,
                             const SIZE & size, const RECT & r, UCHAR alpha) {
    bool result = false;

    if (HDC hMemoryDC = CreateCompatibleDC (hDC)) {
    
        BITMAPINFO bitmap;
        std::memset (&bitmap, 0, sizeof bitmap);
        
        bitmap.bmiHeader.biSize = sizeof bitmap;
        bitmap.bmiHeader.biWidth = size.cx;
        bitmap.bmiHeader.biHeight = -size.cy;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = 32;
        bitmap.bmiHeader.biCompression = BI_RGB;

        if (HBITMAP dib = CreateDIBSection (hDC, &bitmap, DIB_RGB_COLORS,
                                            NULL, NULL, 0u)) {
            if (SaveDC (hDC)) {
                SelectObject (hMemoryDC, dib);
                SetStretchBltMode (hDC, HALFTONE);
                
                if (DrawIconEx (hMemoryDC, 0, 0, hIcon, 0, 0, 0, NULL, DI_NORMAL)) {
                    
                    BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
                    if (AlphaBlend (hDC, r.left, r.top, r.right, r.bottom,
                                    hMemoryDC, 0, 0, size.cx, size.cy, blend)) {
                        
                        result = true;
                    };
                };
                
                RestoreDC (hDC, -1);
            };
            DeleteObject (dib);
        };
        DeleteDC (hMemoryDC);
    };

    return result;
};
};

