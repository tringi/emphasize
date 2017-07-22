#include "Windows_LoadIconImageList.hpp"

/* Windows LoadIconImageList 
// Windows_LoadIconImageList.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      26.03.2014 - initial version
*/

#include "Windows_LoadIcon.hpp"
#include <cstring>

namespace {
    HICON CreateTransparentIcon (unsigned int x, unsigned int y) {
        x = ((x + 15) / 16) * 16; // word alignment
        unsigned int iin = x * y / 8u;
        
        if (!iin || iin > 8192)
            return NULL;
        
        unsigned char transparent [iin];
        std::memset (transparent, 0xFF, iin);
        
        ICONINFO ii = {
            FALSE, 0, 0,
            CreateBitmap (x, y, 1u, 1u, transparent),
            NULL
        };
        HICON h = CreateIconIndirect (&ii);
        DeleteObject (ii.hbmMask);
        return h;
    };
};

HIMAGELIST Windows::LoadIconImageList (HMODULE hModule,
                                       unsigned int first, unsigned int stride,
                                       unsigned int n, int x, int y) {
    if (x == -1) x = GetSystemMetrics (SM_CXICON);
    if (y == -1) y = GetSystemMetrics (SM_CYICON);
    
    if (HIMAGELIST h = ImageList_Create (x, y, ILC_COLOR32 | ILC_MASK,
                                         (n != -1u) ? n : 1, 4)) {
        
        HICON hTransparent = CreateTransparentIcon (x, y);
        
        // icon 0 is transparent
        
        ImageList_ReplaceIcon (h, -1, hTransparent);
        
        for (auto i = 0u; i != n; ++i) {
            if (auto hh = Windows::LoadIcon (hModule, first + i * stride, x, y)) {
                ImageList_ReplaceIcon (h, -1, hh);
                DestroyIcon (hh);
            } else {
                if (n != -1u) {
                    ImageList_ReplaceIcon (h, -1, hTransparent);
                } else
                    break;
            };
        };
        
        DestroyIcon (hTransparent);
        return h;
    } else
        return NULL;
};

