#include "Resources_Language.hpp"

/* Emphasize Resources library Language selection routine
// Resources_Language.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.3
//
// Changelog:
//      28.05.2011 - initial version
//      23.06.2016 - fixed to use UI language, not LCID
*/

#include <cstring>
#include <algorithm>

namespace Resources {
    
    // prefered_language
    //  - used as global default override

    unsigned short prefered_language = 0;
};

namespace {
    
    // index of available languages from which is later chosen
    //  - 256 is arbitrarily chosen maximum
    //     - WinNT.h for Windows7 defines 130 languages (1 to 5 variants each)
    //     - no sane application should ever need this number of localizations

    class LanguageIndex {
        private:
            static const unsigned int maximum = 256u;
            
            unsigned short data [maximum];
            unsigned short size;
            
        public:
            LanguageIndex () : size (0u) {};
            
            bool insert (unsigned short language);
            bool contains (unsigned short language) const;
            bool empty () const { return !this->size; };
            unsigned short first () const;
    };
    
    BOOL CALLBACK SelectProperLanguage (HMODULE, LPCTSTR, LPCTSTR,
                                        WORD, LONG_PTR);
};

void Resources::SetPreferredLanguage (unsigned short language) {
    Resources::prefered_language = language;
    return;
};
unsigned short Resources::GetPreferredLanguage () {
    return Resources::prefered_language;
};

#ifdef __MINGW_VERSION
extern "C" BOOL WINAPI EnumResourceLanguagesExW (HMODULE, LPCTSTR, LPCTSTR, ENUMRESLANGPROC, LONG_PTR, DWORD, LANGID);
#endif

unsigned short Resources::Language (HMODULE hModule, LPCTSTR type,
                                    LPCTSTR name, unsigned short language) {
    
    LanguageIndex index;
    
    // populate the index

#if WINVER >= 0x0600
    if (EnumResourceLanguagesExW (hModule, type, name, SelectProperLanguage,
                                  reinterpret_cast <LONG_PTR> (&index), 0, 0)) {
#else    
    if (EnumResourceLanguages (hModule, type, name, SelectProperLanguage,
                               reinterpret_cast <LONG_PTR> (&index))) {
#endif
        if (!index.empty ()) {
            // choose proper language
        
#define RETURN_IF_INDEX_CONTAINS(lang)  \
    if (index.contains (lang)) {        \
/*        printf ("%u: %08X\n", __LINE__, lang);*/ \
        return lang;                    \
    }
        
            // 1] Try selected language first
            //     - when "default", "user default" or "sys default" is not chosen
            
            if (PRIMARYLANGID (language) != LANG_NEUTRAL) {
    
                // a] exactly the requested language
                RETURN_IF_INDEX_CONTAINS (language);
                
                // b] with preferred sublang (if preferred lang is 0)
                if (PRIMARYLANGID (prefered_language) == LANG_NEUTRAL
                     && SUBLANGID (prefered_language) != SUBLANG_NEUTRAL) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (language),
                                                          SUBLANGID (prefered_language)));
                };
    
                // c] with default sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (language), SUBLANG_DEFAULT));
                // d] with neutral sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (language), SUBLANG_NEUTRAL));
    
                // e] with first-available sublang
                for (unsigned char sub = 0x02; sub < 0x40; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (language), sub));
                };
            };
                
            // 2] Try preferred language second
            //     - same steps as above, except preferred sublang obviously
            
            if (PRIMARYLANGID (prefered_language) != LANG_NEUTRAL) {
                
                // a] exactly the requested language
                RETURN_IF_INDEX_CONTAINS (prefered_language);
                // b] with default sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (prefered_language), SUBLANG_DEFAULT));
                // c] with neutral sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (prefered_language), SUBLANG_NEUTRAL));
    
                // d] with first-available sublang
                for (unsigned char sub = 0x02; sub < 0x40; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (prefered_language), sub));
                };
            };
            
            // 3] User default
            //     - same substeps as above
            //     - skip if "sys default" is explicitely chosen
            
            LANGID user_default = GetUserDefaultUILanguage ();
    
            if (PRIMARYLANGID (language) != LANG_NEUTRAL
                    || SUBLANGID (language) != SUBLANG_SYS_DEFAULT) {
                
                // a] exactly the language
                RETURN_IF_INDEX_CONTAINS (user_default);
                // b] with default sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (user_default), SUBLANG_DEFAULT));
                // c] with neutral sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (user_default), SUBLANG_NEUTRAL));
    
                // d] with first-available sublang
                for (unsigned char sub = 0x02; sub < 0x40; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (user_default), sub));
                };
            };
    
            // 4] System default
            //     - same substeps as above
            
                LANGID sys_default = GetSystemDefaultUILanguage ();
        
                // a] exactly the language
                RETURN_IF_INDEX_CONTAINS (sys_default);
                // b] with default sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (sys_default), SUBLANG_DEFAULT));
                // c] with neutral sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (sys_default), SUBLANG_NEUTRAL));
        
                // d] with first-available sublang
                for (unsigned char sub = 0x02; sub < 0x40; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (PRIMARYLANGID (sys_default), sub));
                };
        
            // 5] Neutral
            //     - usually resources common for all localizations
            
                for (unsigned char sub = 0; sub < 3; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (LANG_NEUTRAL, sub));
                };
            
            // 6] US English or any english
            //     - second last resort
    
                // a] with default sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (LANG_ENGLISH, SUBLANG_DEFAULT));
                // b] with neutral sublang
                RETURN_IF_INDEX_CONTAINS (MAKELANGID (LANG_ENGLISH, SUBLANG_NEUTRAL));
                
                // ??? order ???
                //  SUBLANG_SYS_DEFAULT, SUBLANG_CUSTOM_DEFAULT, SUBLANG_CUSTOM_UNSPECIFIED, SUBLANG_UI_CUSTOM_DEFAULT
    
                // c] with first-available sublang
                for (unsigned char sub = 2; sub < 0x40; ++sub) {
                    RETURN_IF_INDEX_CONTAINS (MAKELANGID (LANG_ENGLISH, sub));
                };

#undef RETURN_IF_INDEX_CONTAINS

            // 7] First language found
            //     - just an attempt to retrieve any meaningful resource
            
            return index.first ();
        };
    };
    
    return 0u;
};

namespace {
BOOL CALLBACK SelectProperLanguage (HMODULE, LPCTSTR, LPCTSTR,
                                    WORD language, LONG_PTR _data) {
    
    return reinterpret_cast <LanguageIndex *> (_data) ->insert (language);
};

bool LanguageIndex::insert (unsigned short language) {
    unsigned short * const begin = &this->data [0u];
    unsigned short * const end   = &this->data [this->size];
    unsigned short * const target = std::upper_bound (begin, end, language);
    
    // NOTE: this path will probably never be taken because the resource
    //       languages always come in sorted, thus the comparison
    
    if (target != end) {
        std::memmove (target + 1, target,
                      sizeof (unsigned short) * (end - target));
    };
    
    *target = language;
    ++this->size;
    
    return this->size < this->maximum;
};

bool LanguageIndex::contains (unsigned short language) const {
    const unsigned short * const begin = &this->data [0u];
    const unsigned short * const end   = &this->data [this->size];

    return std::binary_search (begin, end, language);
};

unsigned short LanguageIndex::first () const {
    if (this->size > 1u
     && this->data [0u] == 0x0000u)
        return this->data [1u];
    else
        return this->data [0u];
};
};
