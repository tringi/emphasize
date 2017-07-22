#include "Windows_CreateBitmapFromIcon.hpp"

/* Windows CreateBitmapFromIcon 
// Windows_CreateBitmapFromIcon.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      31.10.2012 - initial version
*/

#include "Windows_Version.hpp"
#include <cstring>

HBITMAP Windows::CreateBitmapFromIcon (HICON hIcon, HBRUSH hBackground) {
    ICONINFO ii;
    if (!GetIconInfo (hIcon, &ii))
        return NULL;

    BITMAPINFO info;
    std::memset (&info, 0, sizeof info);

    info.bmiHeader.biSize = sizeof info;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    
    if (ii.hbmColor) {
        BITMAP ibcolor;
        if (GetObject (ii.hbmColor, sizeof ibcolor, &ibcolor)) {
            info.bmiHeader.biWidth = ibcolor.bmWidth;
            info.bmiHeader.biHeight = ibcolor.bmHeight;
        };
    } else {
        BITMAP ibmask;
        if (GetObject (ii.hbmMask, sizeof ibmask, &ibmask)) {
            info.bmiHeader.biWidth = ibmask.bmWidth;
            info.bmiHeader.biHeight = ibmask.bmHeight / 2u;
        };
    };
    
    bool success = false;
    HBITMAP dib = NULL;
    
    if (info.bmiHeader.biWidth)
    if (HDC hDC = CreateCompatibleDC (NULL)) {
        
        // dib
        //  - the actual bitmap
        
        void * pBits = NULL;
        if ((dib = CreateDIBSection (hDC, &info, DIB_RGB_COLORS, &pBits, NULL, 0))) {
            
            auto hbPrevious = SelectObject (hDC, dib);
            if (hbPrevious) {
                
                RECT rc = {
                    0, 0,
                    info.bmiHeader.biWidth,
                    info.bmiHeader.biHeight
                };

                // clear buffer background
                //  - BeginBufferedPaint usually returns cached dirty buffer
                //  - default brush is menu-check-icon friendly
                
                if (hBackground == NULL) {
                    if (Version >= Version::WindowsVista) {
                        hBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
                    } else {
                        hBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
                    };
                };
                FillRect (hDC, &rc, hBackground);

                // draw the icon into buffer
                //  - Windows will take care of various icon formats here
                
                if (DrawIconEx (hDC, 0, 0, hIcon,
                                rc.right, rc.bottom, 0, NULL, DI_NORMAL)) {
                    success = true;
                    
                    // the following code only converts mask to alpha
                    //  - if neccessary

                    const auto ptr = static_cast <DWORD *> (pBits);
                    const auto size = rc.right * rc.bottom;
                    
                    // first check if painting wrote any alpha blended pixels
                    //  - if not, copy them from mask bitmap of the icon
                    //  - GdiFlush is required to ensure DrawIconEx finished
                    
                    GdiFlush ();
                    
                    bool alpha = false;
                    
                    {   auto i = size;
                        auto p = ptr;
                        while (i--)
                            if (*p++ & 0xFF000000) {
                                alpha = true;
                                break;
                            };
                    };
                    
                    // alpha channel present
                    //  - update with data from icon mask layer
                    
                    if (!alpha) {
                        const auto heap = GetProcessHeap ();
                        
                        if (auto * mask = static_cast <DWORD *>
                                (HeapAlloc (heap, 0, sizeof (DWORD) * size))) {
                            
                            if (GetDIBits (hDC, ii.hbmMask, 0, rc.bottom,
                                           mask, &info, DIB_RGB_COLORS) == rc.bottom) {
                                
                                auto i = size;
                                auto p = ptr;
                                while (i--)
                                    if (*mask++) {
                                        *p++ &= ~0xFF000000;
                                    } else {
                                        *p++ |= 0xFF000000;
                                    };
                            };
                    
                            HeapFree (heap, 0, mask);
                        };
                    };
                };
                
                SelectObject (hDC, hbPrevious);
            };
            
            // if something failed while a bitmap was held
            //  - delete it and return NULL
            
            if (!success) {
                DeleteObject (dib);
                dib = NULL;
            };
        };
        DeleteDC (hDC);
    };
    
    // cleanup before returning
    //  - GetIconInfo returns copies of its bitmaps
    
    if (ii.hbmColor) {
        DeleteObject (ii.hbmColor);
    };
    DeleteObject (ii.hbmMask);
    return dib;
};

