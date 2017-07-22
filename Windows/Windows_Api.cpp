#include "Windows_Api.hpp"

/* Windows Api 
// Windows_Api.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      27.04.2013 - initial version
*/

#include "Windows_Symbol.hpp"
#include "../Resources/Resources_VersionInfo.hpp"
#include <cwchar>

namespace {
    Windows::Api::KnownApis Retrieve ();
};

class Windows::Api Windows::Api;
Windows::Api::Api () : value (Retrieve ()) {};

namespace {
    Windows::Api::KnownApis Retrieve () {
        const char * (WINAPI * wine_get_version) ();
        BOOL (WINAPI * pIsWow64Process) (HANDLE, PBOOL);
        
        if (HMODULE hNtDll = GetModuleHandleW (L"ntdll")) {
            if (Windows::Symbol (hNtDll, wine_get_version, "wine_get_version")) {
                return Windows::Api::Wine;
            } else {
                const Resources::VersionInfo viNtDll (hNtDll);
                if (!std::wcsncmp (viNtDll [L"ProductName"], L"ReactOS", 7)) {
                    return Windows::Api::ReactOS;
                };
            };
        };
    
#ifndef _WIN64
        if (Windows::Symbol (L"kernel32", pIsWow64Process, "IsWow64Process")) {
            BOOL bIsWow64 = FALSE;
            if (pIsWow64Process (GetCurrentProcess (), &bIsWow64)) {
                if (bIsWow64) {
                    return Windows::Api::WOW64;
                };
            };
        };
#endif
       
        return Windows::Api::Native;
    };
};
