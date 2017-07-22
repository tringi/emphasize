#ifndef WINDOWS_BLANKWNDCLASS_HPP
#define WINDOWS_BLANKWNDCLASS_HPP

/* Emphasize Windows blank window class
// Windows_BlankWndClass.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: Provides the module a L"BLANK" window class that it can use for
//              quick helper windows. LPVOID lpParam of the CreateWindow call
//              must point to WNDPROC that will be assigned to the new window.
//
// Changelog:
//      01.08.2011 - initial version
*/

#include <windows.h>

namespace Windows {
namespace BlankClass {
    
    // Initialize
    //  - registers the L"BLANK" windows class for the HMODULE provided
    
    ATOM Initialize (HMODULE);
    
    // Name
    //  - pointer to string L"BLANK" which is a name of the blank window class
    //  - call CreateWindow with Name as a class-name and provide WNDPROC
    //    procedure pointer as LPVOID lpParam parameter
    
    extern const LPCTSTR Name;
    
};
};

#endif
