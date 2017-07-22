#include "Resources_Menu.hpp"

/* Emphasize Resources library Menu access routine
// Resources_Menu.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      28.12.2011 - initial version
*/

#include "Resources_Raw.hpp"

HMENU Resources::Menu (HMODULE hModule, LPCTSTR name, unsigned short language) {
    Resources::Raw raw (hModule,
                        MAKEINTRESOURCE (4u), name,
                        language);
    if (raw.data)
        return LoadMenuIndirect (static_cast <const MENUTEMPLATE *> (raw.data));
    else
        return NULL;
};
