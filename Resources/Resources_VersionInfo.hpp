#ifndef RESOURCES_VERSIONINFO_HPP
#define RESOURCES_VERSIONINFO_HPP

/* Emphasize Resources library Version Info resource access
// Resources_VersionInfo.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      24.07.2011 - initial version
*/

#include <windows.h>
#include "Resources_Raw.hpp"

namespace Resources {
    
    // VersionInfo
    //  - Raw.data and Raw.size can be directly used by VerQueryValue function,
    //    as this class supports only already loaded 32-bit modules
    //  - Usage: Resources::VersionInfo version (m);
    //           version.info->product.major;
    //           version.info->product.minor;
    //           version.info->product.release;
    //           version.info->product.build;
    //           version.info->file.major;
    //           version.info->file.minor;
    //           version.info->file.release;
    //           version.info->file.build;
    //           version.info->timestamp;
    
    class VersionInfo
        : protected Resources::Raw {

        protected:
            unsigned short  length;
            const void *    strings;

        public:
            using Resources::Raw::language;
            
            struct Translation {
                unsigned short  codepage;
                unsigned short  language;
            } translation;

            union {
                
                // fixed file info
                //  - access through names defined by Windows API
                
                const VS_FIXEDFILEINFO * ffi;
                
                // named structure members
                //  - more descriptive access through simply named members
                
                const struct {
                    DWORD header [2u];
                    
                    struct {
                        unsigned short minor;
                        unsigned short major;
                        unsigned short build;
                        unsigned short release;
                    } file,
                      product;
                    
                    DWORD flags [5u];
                    
                    // timestamp
                    //  - big endian, otherwise a FILETIME number
                    
                    unsigned long long timestamp;
                } * info;
            };
        
        public:
            explicit VersionInfo (HMODULE, unsigned short language = 0u);
                     VersionInfo (const VersionInfo &);

        public:
            
            // Access operation
            //  - unsigned int - returns string by its index
            //  - const wchar_t * - returns string by name
            
            const wchar_t * operator [] (unsigned int) const;
            const wchar_t * operator [] (const wchar_t *) const;

    };
};

#endif
