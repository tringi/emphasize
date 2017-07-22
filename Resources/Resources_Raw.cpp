#include "Resources_Raw.hpp"

/* Emphasize Resources library Raw resource access
// Resources_Raw.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      28.05.2011 - initial version
*/

#include "Resources_Language.hpp"

Resources::Raw::Raw (HMODULE hModule, LPCTSTR type, LPCTSTR name,
                     unsigned short _language)
    :   data (NULL),
        size (0u),
        language (Resources::Language (hModule, type, name, _language)) {
    
    if (this->language) {
        if (HRSRC hRsrc = FindResourceEx (hModule, type, name, this->language)) {
            if (HGLOBAL hGlobal = LoadResource (hModule, hRsrc)) {
                this->data = LockResource (hGlobal);
                this->size = SizeofResource (hModule, hRsrc);
            };
        };
    };
    
    return;
};

Resources::Raw::Raw (const Raw & r)
    :   data (r.data),
        size (r.size),
        language (r.language) {};
