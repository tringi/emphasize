#ifndef WINDOWS_ENABLEPRIVILEGE_HPP
#define WINDOWS_ENABLEPRIVILEGE_HPP

/* Windows EnablePrivilege 
// Windows_EnablePrivilege.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      25.10.2014 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // EnablePrivilege
    //  - enables specified privilege
    //  - from Platform SDK example

    bool EnablePrivilege (LPCTSTR name, bool enable = true);
    bool DisablePrivilege (LPCTSTR name);
    
};

#endif

