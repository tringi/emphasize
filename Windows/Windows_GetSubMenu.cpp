#include "Windows_GetSubMenu.hpp"

/* Windows GetSubMenu 
// Windows_GetSubMenu.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      09.11.2012 - initial version
*/

HMENU Windows::GetSubMenu (HMENU menu, UINT index) {
    UINT position = 0;
    
    MENUITEMINFO mii;
    mii.cbSize = sizeof mii;
    mii.fMask = MIIM_SUBMENU;
    
    while (GetMenuItemInfo (menu, position++, TRUE, &mii)) {
        if (mii.hSubMenu)
            if (index-- == 0)
                return mii.hSubMenu;
    };
    
    return NULL;
};

