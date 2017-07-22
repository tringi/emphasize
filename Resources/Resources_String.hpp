#ifndef RESOURCES_STRING_HPP
#define RESOURCES_STRING_HPP

/* Emphasize Resources library String access routine
// Resources_String.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      28.05.2011 - initial version
//      06.05.2015 - completely rewritten
*/

#include <windows.h>

namespace Resources {
    
    // Initialize
    //  - no longer used
    
    inline void Initialize () {};
    
    // ClearStringCache
    //  - clears cached strings
    //  - NOTE: all previously returned pointers may be no longer valid!
    //  - call when preferred language changes (Resources::SetPreferredLanguage)
    
    void ClearStringCache ();
    
    // String
    //  - returns pointer to zero-terminated string of provided ID
    //    loaded from the current module
    
    const wchar_t * String (unsigned int id);
    const wchar_t * String (unsigned int id, unsigned short language);
    
    // String
    //  - returns pointer to resource string (not zero-terminated) in the
    //    CURRENT MODULE (e.g. DLL) and stores the actual length in *length
    //  - the length is not optional and cannot be NULL
    //  - use "language" LANGID (MAKELANGID) as a hint to language selection
    //  - note that this version is faster and uses no additional memory
    
    const wchar_t * String (unsigned int id, unsigned int * length,
                            unsigned short language = 0u);

    // String
    //  - HMODULE variants (explicitely choosen) e.g. for use in DLLs etc.
    //  - if hModule is NULL the string is loaded from the module that was
    //    used to create the current process

    const wchar_t * String (HMODULE hModule, unsigned int id,
                            unsigned short language = 0u);
    const wchar_t * String (HMODULE hModule, unsigned int id,
                            unsigned int * length, unsigned short language = 0u);
};

#endif
