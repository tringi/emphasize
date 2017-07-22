#ifndef WINDOWS_CHILDRENCONTAINER_HPP
#define WINDOWS_CHILDRENCONTAINER_HPP

/* Windows ChildrenContainer 
// Windows_ChildrenContainer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      28.03.2014 - initial version
*/

#include <windows.h>

namespace Windows {

    // InitializeChildrenContainer
    //  - 

    ATOM InitializeChildrenContainer (HINSTANCE);
    
    // CreateChildrenContainer
    //  - 
    
    HWND CreateChildrenContainer (HINSTANCE, HWND, UINT id, DWORD style);
    
};

#endif

