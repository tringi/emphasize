#ifndef RESOURCES_LANGUAGE_HPP
#define RESOURCES_LANGUAGE_HPP

/* Emphasize Resources library Language selection routine
// Resources_Language.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Rationale: MSDN:
//            The catch here is that if the thread locale is the same as the
//            currently selected user locale, system’s resource loader will by
//            default use the language ID 0 (neutral). If the desired resource
//            is defined as a neutral language, then this value will be
//            returned. Otherwise, all of the language resources will be
//            enumerated (in language ID order) and the first matching
//            resource ID – regardless of its language – will be returned.    
//
// Changelog:
//      28.05.2011 - initial version
*/

#include <windows.h>

namespace Resources {
    
    // (S/G)etPreferredLanguage
    //  - sets or retrieves application preferred language
    //     - this is NOT thread-local variable, but since you should anyway
    //       have only single UI thread I guess it's okay
    //  - the preferred language is used when call to Resources::Language
    //    sets 'language' to 0 or a language that is not present
    //  - not used when set to 0
    //  - when set to MAKELANGID (LANG_NEUTRAL, sub), and attempt to choose
    //    a MAKELANGID (PRIMARYLANGID (language), sub) is made if 'language'
    //    exactly is not present
    
    void SetPreferredLanguage (unsigned short);
    unsigned short GetPreferredLanguage ();
    
    // Language
    //  - searches available languages for the provided resource type&name
    //    and returns the best match in the following order:
    //  - 1] provided language parameter (when not zero)
    //        a] exactly
    //        b] with preferred langugage sublang when preferred language
    //           it not zero and its primary language ID is LANG_NEUTRAL
    //        c] with default sublang
    //        d] with neutral sublang
    //        e] with first available sublang
    //  - 2] preferred language (when not zero)
    //        a] exactly
    //        b] with default sublang
    //        c] with neutral sublang
    //        d] with first available sublang
    //  - 3] user default language (only if 'language' is not
    //                              LANG_NEUTRAL + SUBLANG_SYS_DEFAULT)
    //        a] exactly
    //        b] with default sublang
    //        c] with neutral sublang
    //        d] with first available sublang
    //  - 4] system default language
    //        a] exactly
    //        b] with default sublang
    //        c] with neutral sublang
    //        d] with first available sublang
    //  - 5] neutral language
    //        a] LANG_NEUTRAL : SUBLANG_NEUTRAL
    //        b] LANG_NEUTRAL : SUBLANG_DEFAULT
    //        c] LANG_NEUTRAL : SUBLANG_SYS_DEFAULT
    //  - 6] English
    //        a] Default (US) English
    //        b] Neutral English
    //        c] first available English sublang
    //  - 7] The first language in enumeration
    //  - 8] Zero if no language is available
    //        - note that this condition is actually checked first to avoid
    //          traversing the whole tree to find nothing
    
    unsigned short Language (HMODULE hModule, LPCTSTR, LPCTSTR,
                             unsigned short language = 0u);
};

#endif
