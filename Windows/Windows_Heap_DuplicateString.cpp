#include "Windows_Heap_DuplicateString.hpp"

/* Windows Heap_DuplicateString 
// Windows_Heap_DuplicateString.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.2
//
// Changelog:
//      20.08.2012 - initial version
//      30.03.2013 - added heap parameter
*/

#include <cstring>
#include <algorithm>

char * Windows::Heap::DuplicateString (const char * string, HANDLE heap) {
    return Windows::Heap::DuplicateString (string, -1u, heap);
};
wchar_t * Windows::Heap::DuplicateString (const wchar_t * string, HANDLE heap) {
    return Windows::Heap::DuplicateString (string, -1u, heap);
};

wchar_t * Windows::Heap::DuplicateString (const wchar_t * string,
                                          unsigned int maximum,
                                          HANDLE heap) {
    if (string) {
        const auto length = std::min (std::wcslen (string), (std::size_t) maximum) + 1u;
        const auto size = sizeof (wchar_t) * length;
        
        if (heap == NULL)
            heap = GetProcessHeap ();
        
        if (auto * memory = static_cast <wchar_t *> (HeapAlloc (heap, 0, size))) {
            
            // two phase copy
            //  - first copy the string
            //  - then terminate by NUL, because if 'maximum' is enforced, last
            //    character most likely isn't NUL
            
            std::wmemcpy (memory, string, length - 1u);
            memory [length - 1u] = L'\0';
            
            return memory;
        };
    };
    return NULL;
};

char * Windows::Heap::DuplicateString (const char * string,
                                       unsigned int maximum,
                                       HANDLE heap) {
    if (string) {
        const auto size = std::min (std::strlen (string), (std::size_t) maximum) + 1u;

        if (heap == NULL)
            heap = GetProcessHeap ();
        
        if (auto * memory = static_cast <char *> (HeapAlloc (heap, 0, size))) {
            
            // two phase copy
            //  - first copy the string
            //  - then terminate by NUL, because if 'maximum' is enforced, last
            //    character most likely isn't NUL
            
            std::memcpy (memory, string, size);
            memory [size - 1u] = L'\0';
            
            return memory;
        };
    };
    return NULL;
};
