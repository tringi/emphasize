#include "Windows_GetDesktopDPI.hpp"

/* Windows GetDesktopDPI 
// Windows_GetDesktopDPI.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      10.12.2012 - initial version
*/

bool Windows::GetDesktopDPI (unsigned int * x, unsigned int * y) {
    if (HDC hDC = GetDC (NULL)) {
        if (x)
            *x = GetDeviceCaps (hDC, LOGPIXELSX);
        if (y)
            *y = GetDeviceCaps (hDC, LOGPIXELSY);
        
        ReleaseDC (NULL, hDC);
        return true;
    } else {
        if (x)
            *x = 96u;
        if (y)
            *y = 96u;
    
        return false;
    };
};

bool Windows::GetDesktopScale (float * x, float * y) {
    if (x || y) {
        unsigned int xx = 0;
        unsigned int yy = 0;
        
        if (Windows::GetDesktopDPI (&xx, &yy)) {
            if (x)
                *x = xx / 96.0f;
            if (y)
                *y = yy / 96.0f;
            
            return true;
        };
    };
    
    return false;
};

