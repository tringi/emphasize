#ifndef WINDOWS_WINDOWEXTRAMEMORY_HPP
#define WINDOWS_WINDOWEXTRAMEMORY_HPP

/* Emphasize Windows window extra memory accessor templates
// Windows_WindowExtraMemory.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: Data structure is stored as window extra bytes (cbWndExtra)
//              Current implementation does not support (and fails to compile
//              for) data members larger than LONG_PTR.
//
// Changelog:
//      07.04.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetWindowExtraSize
    //  - computes proper value of WNDCLASS.cbWndExtra so the access functions
    //    can safelly set/get even the last member variable of a structure Data
    //  - you can safely use sizeof(Data) only if the last member variable of
    //    the Data structure is of the size of LONG_PTR type.

    template <typename Data>
    UINT GetWindowExtraSize ();
    
    // GetWindowExtra
    //  - retrieves a value from extra window memory as-if that was of the same
    //    layout as the provided Data structure
    //  - call syntax: Windows::GetWindowExtra (hWnd, &StructType::memberName);
    
    template <typename T, typename Data>
    T GetWindowExtra (HWND hWnd, T Data::* Offset);
    
    // SetWindowExtra
    //  - stores a value into extra window memory as-if that was of the same
    //    layout as the provided Data structure
    //  - returns: previous value of the member variable
    
    template <typename T, typename Data, typename S>
    T SetWindowExtra (HWND hWnd, T Data::* Offset, S value);
    
};

#include "Windows_WindowExtraMemory.tcc"
#endif
