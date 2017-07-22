#ifndef WINDOWS_HEAP_DUPLICATESTRING_HPP
#define WINDOWS_HEAP_DUPLICATESTRING_HPP

/* Windows Heap_DuplicateString 
// Windows_Heap_DuplicateString.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.2
//
// Changelog:
//      20.08.2012 - initial version
//      30.03.2013 - added heap parameter
*/

#include <windows.h>

namespace Windows {
namespace Heap {
    
    // Heap::DuplicateString
    //  - allocates memory on process default heap and copies string into it
    //  - parameter 'maximum' can optionally limit number of characters
    //    (NUL-termination is ensured)
    //  - use 'heap' parameter to override default (process) heap
    
    char *    DuplicateString (const char *, HANDLE heap = NULL);
    wchar_t * DuplicateString (const wchar_t *, HANDLE heap = NULL);
    
    char *    DuplicateString (const char *, unsigned int maximum, HANDLE heap = NULL);
    wchar_t * DuplicateString (const wchar_t *, unsigned int maximum, HANDLE heap = NULL);
};
};

#endif
