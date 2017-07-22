#ifndef WINDOWS_ENUMPROCESSESALLOC_HPP
#define WINDOWS_ENUMPROCESSESALLOC_HPP

/* Windows EnumProcessesAlloc 
// Windows_EnumProcessesAlloc.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      08.04.2013 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // EnumProcessesAlloc
    //  - returns HeapAlloc-ated vector of all process IDs or NULL on error
    //     - the vector is terminated by value 0
    //     - use HeapFree to release the memory
    //     - on error call GetLastError () for details
    
    DWORD * EnumProcessesAlloc ();
    
};

#endif

