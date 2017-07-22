#ifndef RESOURCES_RAW_HPP
#define RESOURCES_RAW_HPP

/* Emphasize Resources library Raw resource access
// Resources_Raw.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.2
//
// Changelog:
//      28.05.2011 - initial version
//      28.07.2015 - 'size' is 32-bit even on 64-bit OS (int thus saves 8 bytes)
*/

#include <windows.h>
#include <cstddef>

namespace Resources {
    
    // Raw
    //  - simple accessor class, loads pointer and size of the resource data
    //  - the data are "const" since resources are copy-on-write memory
    //  - the language member is set to the actual resource language
    
    class Raw {
        public:
            const void *    data;
            unsigned int    size;
            unsigned short  language;
            
        public:
            Raw (HMODULE, LPCTSTR, LPCTSTR, unsigned short language = 0u);
            Raw (const Raw &);
    };
};

#endif
