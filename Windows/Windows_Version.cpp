#include "Windows_Version.hpp"

/* Emphasize Windows Version checking tools
// Windows_Version.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      09.07.2011 - initial version
//      28.03.2012 - properly set WinXP to 5.2 on 64-bit system,
//                   staticaly when native _WIN64, dynamicaly when under WOW64
//      05.01.2013 - added NT4
//      06.08.2013 - added 8 and 8.1
//                 - plain initialization now prefers RtlGetNtVersionNumbers
//                   over GetVersion which is deprecated as of NT 6.3
*/

#include "Windows_Symbol.hpp"

class Windows::Version Windows::Version;

namespace {
#ifndef _WIN64
    bool initialized = false;
#else
    static const
#endif
    unsigned char numbers [Windows::Version::NumberOfKnownVersions][2] = {
        { 4u, 0u }, // Windows NT4
        { 5u, 0u }, // Windows 2000
#ifdef _WIN64
        { 5u, 2u }, // Windows XP (64-bit in 5.1)
#else
        { 5u, 1u }, // Windows XP
#endif
        { 5u, 2u }, // Windows Server 2003 (note also Windows XP 64-bit)
        { 6u, 0u }, // Windows Vista, Windows Server 2008
        { 6u, 1u }, // Windows 7, Windows Server 2008 R2
        { 6u, 2u }, // Windows 8, Windows Server 2012
        { 6u, 3u }, // Windows 8.1, Windows Server 2012 R2
    };
    
    unsigned char major (DWORD version) {
        return LOBYTE (LOWORD (version)) & 0x7F;
    };
    unsigned char minor (DWORD version) {
        return HIBYTE (LOWORD (version)) & 0x7F;
    };
    unsigned short build (DWORD version) {
        return HIWORD (version) & 0x7FFF;
    };
};

Windows::Version::Version (DWORD version)
    :   Major (major (version)),
        Minor (minor (version)),
        Build (build (version)) {
#ifndef _WIN64
    if (!initialized) {
        initialized = true;
        
        BOOL (WINAPI * pIsWow64Process) (HANDLE, PBOOL) = NULL;
        if (Windows::Symbol (L"kernel32", pIsWow64Process, "IsWow64Process")) {
            BOOL bIsWow64 = FALSE;
            if (pIsWow64Process (GetCurrentProcess (), &bIsWow64)) {
                if (bIsWow64) {
                    
                    // Set version number of Windows XP to 5.2
                    // for further comparisons
                    //  - kernel has IsWow64Process that returns TRUE means we
                    //    are running on 64-bit version of Windows
                    
                    numbers [WindowsXP][1u] = 2u;
                };
            };
        };
    };
#endif
    return;
};

bool Windows::Version::operator < (Windows::Version::KnownVersions v) {
    return this->Major < numbers [v][0]
        || (this->Major == numbers [v][0] && this->Minor < numbers [v][1]);
};

bool Windows::Version::operator > (Windows::Version::KnownVersions v) {
    return this->Major > numbers [v][0]
        || (this->Major == numbers [v][0] && this->Minor > numbers [v][1]);
};

bool Windows::Version::operator <= (Windows::Version::KnownVersions v) {
    return *this == v
        || *this < v;
};

bool Windows::Version::operator >= (Windows::Version::KnownVersions v) {
    return *this == v
        || *this > v;
};

bool Windows::Version::operator == (Windows::Version::KnownVersions v) {
    return this->Major == numbers [v][0]
        && this->Minor == numbers [v][1];
};

bool Windows::Version::operator != (Windows::Version::KnownVersions v) {
    return this->Major != numbers [v][0]
        || this->Minor != numbers [v][1];
};

DWORD Windows::Version::GetCodedVersionNumbers () {
    void (WINAPI * pfnRtlGetNtVersionNumbers) (LPDWORD, LPDWORD, LPDWORD);
    
    if (Windows::Symbol (L"ntdll", pfnRtlGetNtVersionNumbers, "RtlGetNtVersionNumbers")) {
        DWORD major = 0;
        DWORD minor = 0;
        DWORD build = 0;
        
        pfnRtlGetNtVersionNumbers (&major, &minor, &build);
        return (major & 0xFF)
             | ((minor & 0xFF) << 8)
             | ((build & 0xFFFF) << 16);
    };
    
    return GetVersion ();
};
