#include "Windows_GetWindowInstance.hpp"

/* Windows GetWindowInstance 
// Windows_GetWindowInstance.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      19.11.2012 - initial version
*/

HINSTANCE Windows::GetWindowInstance (HWND hWnd) {
    return reinterpret_cast <HINSTANCE> (GetWindowLongPtr (hWnd, GWLP_HINSTANCE));
};

