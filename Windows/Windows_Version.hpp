#ifndef WINDOWS_VERSION_HPP
#define WINDOWS_VERSION_HPP

/* Emphasize Windows Version checking tools
// Windows_Version.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      09.07.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    extern class Version {
        public:
            explicit Version (DWORD version = GetCodedVersionNumbers ());
        
        public:
            const unsigned char  Major;
            const unsigned char  Minor;
            const unsigned short Build;
        
        public:
            enum KnownVersions {
                WindowsNT4,
                Windows2000,
                WindowsXP,
                Server2003,
                WindowsVista,
                    Server2008 = WindowsVista,
                Windows7,
                    Server2008R2 = Windows7,
                Windows8,
                    Server2012 = Windows8,
                Windows8_1,
                    Server2012R2 = Windows8_1,
                NumberOfKnownVersions
            };
        
        public:
            bool operator < (KnownVersions);
            bool operator > (KnownVersions);
            bool operator <= (KnownVersions);
            bool operator >= (KnownVersions);
            bool operator == (KnownVersions);
            bool operator != (KnownVersions);
        
        public:
            static DWORD GetCodedVersionNumbers ();
        
    } Version;
};

#endif
