#ifndef WINDOWS_GETSUBMENU_HPP
#define WINDOWS_GETSUBMENU_HPP

/* Windows GetSubMenu 
// Windows_GetSubMenu.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      09.11.2012 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetSubMenu
    //  - finds index-th submenu and returns its handle
    //  - returns: menu handle or NULL on error
    //             (usually index is too large)
    //  - similar to GetSubMenu API function, but finds index-th submenu, not
    //    n-th menu item and returning NULL if that is not a submenu
    
    HMENU GetSubMenu (HMENU menu, UINT index);
    
};

#endif

