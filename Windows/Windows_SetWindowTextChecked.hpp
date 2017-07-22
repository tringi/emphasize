#ifndef WINDOWS_SETWINDOWTEXTCHECKED_HPP
#define WINDOWS_SETWINDOWTEXTCHECKED_HPP

/* Windows SetWindowTextChecked 
// Windows_SetWindowTextChecked.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      08.10.2014 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // Set(Window/DlgItem)TextChecked
    //  - compares existing text and sets only if new is different to prevent
    //    redrawing of common controls
    //  - if the string was not update (identical), return value is false
    //    with GetLastError being set to ERROR_DUPLICATE_TAG (2014L)
    
    bool SetWindowTextChecked (HWND, LPCTSTR);
    bool SetDlgItemTextChecked (HWND, UINT, LPCTSTR);
};

#endif

