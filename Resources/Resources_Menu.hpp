#ifndef RESOURCES_MENU_HPP
#define RESOURCES_MENU_HPP

/* Emphasize Resources library Menu access routine
// Resources_Menu.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      28.12.2011 - initial version
*/

#include <windows.h>

namespace Resources {

    // Menu
    //  - works just like LoadMenu Windows API function but additionally
    //    allows user to choose a particular 'language'
    //  - selects language with Resources::Language
    //  - returns menu handle on success, or NULL on error

    HMENU Menu (HMODULE, LPCTSTR, unsigned short language = 0u);
};

#endif
