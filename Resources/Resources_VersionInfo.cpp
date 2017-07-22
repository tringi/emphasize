#include "Resources_VersionInfo.hpp"

/* Emphasize Resources library Version Info resource access
// Resources_VersionInfo.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      24.07.2011 - initial version
*/

#include <cstdio>
#include <cstring>
#include <cwchar>
#include <algorithm>

namespace {
    struct Header {
        WORD  wLength; 
        WORD  wValueLength; 
        WORD  wType; 
    };
    struct VS_VERSIONINFO : public Header { 
        WCHAR szKey [sizeof "VS_VERSION_INFO"]; // 15 characters
        WORD  Padding1 [1]; 
        VS_FIXEDFILEINFO Value;
    };
    static const VS_FIXEDFILEINFO defaultffi = {
        0xFEEF04BDu, 0x00010000u, 0,0,0,0,
        VS_FF_INFOINFERRED, VS_FF_INFOINFERRED,
        VOS_NT_WINDOWS32, VFT_UNKNOWN, 0, 0,0
    };
};

Resources::VersionInfo::VersionInfo (HMODULE hModule, unsigned short language)
    :   Resources::Raw (hModule, RT_VERSION, MAKEINTRESOURCE (1), language),
        length         (0u),
        strings        (NULL),
        translation    ({ 0, 0 }),
        ffi            (&defaultffi) {
    
    if (this->size >= sizeof (VS_VERSIONINFO))
    if (const Header * vi = static_cast <const Header *> (this->data)) {
        
        // retrieve VS_FIXEDFILEINFO
        //  - find 32-bit defaultffi.dwSignature marking the beginning of the data
        
        if (vi->wValueLength) {
            const unsigned * p = static_cast <const unsigned *> (this->data);
            const unsigned * e = p + (this->size - sizeof (VS_VERSIONINFO))
                               / sizeof *p;
            
            p = std::find (p, e, defaultffi.dwSignature);
            if (p != e) {
                this->ffi = reinterpret_cast <const VS_FIXEDFILEINFO *> (p);
            };
        };
        
        // Children after VS_VERSIONINFO
        //  - zero or one StringFileInfo and zero or one VarFileInfo
        //  - counting real vi->wValueLength instead of sizeof (VS_FIXEDFILEINFO)
        
        const unsigned char * p = static_cast <const unsigned char *> (this->data)
                                + sizeof (VS_VERSIONINFO)
                                + sizeof (Header)
                                - sizeof (VS_FIXEDFILEINFO)
                                + vi->wValueLength;
        if (!std::wcscmp ((const wchar_t *) p, L"StringFileInfo")) {
            
            p += sizeof (L"StringFileInfo");
            
            // Length
            //  - length of 'strings', determines first byte not to be touched
            //  - strategically positioned as first in this class to fill up two
            //    padding bytes left by Resources::Raw
            
            this->length = reinterpret_cast <const Header *> (p) ->wLength
                         - sizeof (Header)
                         - sizeof (L"00000000");
            
            p += sizeof (Header);
            
            // Parse codepage and language IDs
            //  - NOTE: Technically there can be more translations of the same
            //          string data, but as far as I know it is not implemented
            //          anywhere, and since Windows are moving towards MUI
            //          translations rather, I won't bother to implement it too.
            //  - NOTE: Technically we should examine VarFileInfo and then
            //          choose the correct StringFileInfo table, but since there
            //          is always one, we won't bother with VarFileInfo at all.
            
            const wchar_t * s = reinterpret_cast <const wchar_t *> (p);
            const unsigned int n = std::wcstoul (s, NULL, 16u);
            
            this->translation.language = (n >> 16) & 0xFFFF;
            this->translation.codepage = (n >>  0) & 0xFFFF;
            
            // Pointer to string table is simple here
            
            p += sizeof (L"00000000");
            this->strings = p;
        };
    };
    
    return;
};

Resources::VersionInfo::VersionInfo (const Resources::VersionInfo & vi)
    :   Resources::Raw (vi),
        length         (vi.length),
        strings        (vi.strings),
        translation    (vi.translation),
        info           (vi.info) {};

const wchar_t * Resources::VersionInfo::operator [] (unsigned int index) const {
    if (this->strings) {

        const Header * header;
        const unsigned char * p = static_cast <const unsigned char *> (this->strings);
        const unsigned char * e = p + this->length;

        // Searching for 'name' in String Table

        do {
            header = reinterpret_cast <const Header *> (p);
            
            if (index == 0u) {
                
                // Find value string
                //  - simply length of the structure decremented by
                //    value string length

                return reinterpret_cast <const wchar_t *>
                                        (p + header->wLength
                                           - header->wValueLength * sizeof (wchar_t));
            };
            
            // Not found
            //  - aligning again to 32-bit boundary and continuing if not
            //    already exhausted all strings

            p += header->wLength;
            if (header->wLength % 4u)
                p += 4u - (header->wLength % 4u);

            --index;
        } while (p < e && header->wLength);
    };
    return NULL;
};

const wchar_t * Resources::VersionInfo::operator [] (const wchar_t * name) const {
    if (this->strings) {

        const Header * header;
        const unsigned char * p = static_cast <const unsigned char *> (this->strings);
        const unsigned char * e = p + this->length;
        
        // Searching for 'name' in String Table
        
        do {
            header = reinterpret_cast <const Header *> (p);
            if (!std::wcscmp (reinterpret_cast <const wchar_t *>
                                               (p + sizeof (Header)), name)) {
                
                // Find value string
                //  - adjusting pointer due to padding added because of
                //    alignment rules for version information resources
                
                const auto namelen = std::wcslen (name);
                return reinterpret_cast <const wchar_t *>
                                        (p + sizeof (Header)
                                           + sizeof (wchar_t) * (namelen + 1)
                                           + 2 * (namelen % 2));
            };
            
            // Not found
            //  - aligning again to 32-bit boundary and continuing if not
            //    already exhausted all strings
            
            p += header->wLength;
            if (header->wLength % 4u)
                p += 4u - (header->wLength % 4u);
            
        } while (p < e && header->wLength);
    };
    return NULL;
};
