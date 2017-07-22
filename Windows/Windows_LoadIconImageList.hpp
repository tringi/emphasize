#ifndef WINDOWS_LOADICONIMAGELIST_HPP
#define WINDOWS_LOADICONIMAGELIST_HPP

/* Windows LoadIconImageList 
// Windows_LoadIconImageList.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      26.03.2014 - initial version
*/

#include <windows.h>
#include <commctrl.h>

namespace Windows {
    
    // LoadIconImageList
    //  - 
    
    HIMAGELIST LoadIconImageList (HMODULE, unsigned int first, unsigned int stride,
                                  unsigned int n = -1u, int x = -1, int y = -1);
    
};

#endif

