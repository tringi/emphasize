#include "Windows_SetWindowTextChecked.hpp"

/* Windows SetWindowTextChecked 
// Windows_SetWindowTextChecked.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      08.10.2014 - initial version
*/

#include <cstring>
#include <cwchar>

#include "../ext/buffer"

bool Windows::SetDlgItemTextChecked (HWND hWnd, UINT id, LPCTSTR string) {
    if (HWND hCtrl = GetDlgItem (hWnd, id)) {
        return Windows::SetWindowTextChecked (hCtrl, string);
    } else
        return false;
};

bool Windows::SetWindowTextChecked (HWND hWnd, LPCTSTR string) {
    
    // if existing text is of the same length
    //  - if not, overwrite because it is obviously different
    
    const unsigned int length = GetWindowTextLength (hWnd);
    if (length == std::wcslen (string)) {
        
        // if existing text can be retrieved
        //  - if not, overwrite
        
        ext::buffer <wchar_t> buffer (length + 1u);
        if (GetWindowText (hWnd, buffer, buffer.size)) {
            
            // if existing and new text compare identicaly
            
            if (!std::wcscmp (string, buffer)) {
                
                // identical text, don't set
                SetLastError (2014L); // ERROR_DUPLICATE_TAG
                return false;
            };
        };
    };
    return SetWindowText (hWnd, string);
};

