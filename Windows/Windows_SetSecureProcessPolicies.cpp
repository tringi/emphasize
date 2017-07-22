#include "Windows_SetSecureProcessPolicies.hpp"

/* Windows SetSecureProcessPolicies 
// Windows_SetSecureProcessPolicies.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      12.03.2013 - initial version
//      31.03.2013 - added HeapEnableTerminationOnCorruption
//      03.04.2014 - added activation of low-fragmentation heap
//      24.11.2014 - added RegDisablePredefinedCache(Ex)
//      01.09.2016 - static guards, Server 2016 Nano support
*/

#include "../Windows/Windows_Symbol.hpp"

#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x00000001
#endif

#ifndef PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION
#define PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION 0x00000002
#endif

#ifndef BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE
#define BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE 0x1
#endif

#ifndef BASE_SEARCH_PATH_PERMANENT
#define BASE_SEARCH_PATH_PERMANENT 0x8000
#endif

bool Windows::SetSecureProcessPolicies () {
    auto success = true;
    auto kernel32 = GetModuleHandleW (L"KERNEL32");

    SetErrorMode (SEM_FAILCRITICALERRORS);

    // SetProcessDEPPolicy is not available in Server Nano 2016 and not supported in 64-bit Windows at all
#ifndef _WIN64
#if WINVER >= _WIN32_WINNT_WINXP
    SetProcessDEPPolicy (PROCESS_DEP_ENABLE | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION); // XP SP3+, Vista SP1+
#else

    BOOL (WINAPI * pfnSetProcessDEPPolicy) (DWORD);
    if (Windows::Symbol (kernel32, pfnSetProcessDEPPolicy, "SetProcessDEPPolicy")) {
        if (!pfnSetProcessDEPPolicy (PROCESS_DEP_ENABLE | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION)) {
            success = false;
        };
    };
#endif
#endif

    // SetDllDirectory is not available staring Windows Server 2016 Nano
#if WINVER >= _WIN32_WINNT_WINXP && WINVER < _WIN32_WINNT_WIN10
    SetDllDirectory (L""); // XP SP1+
#else
    BOOL (WINAPI * pfnSetDllDirectory) (LPCTSTR);
    if (Windows::Symbol (kernel32, pfnSetDllDirectory, "SetDllDirectoryW")) {
        if (!pfnSetDllDirectory (L"")) {
            success = false;
        };
    };
#endif

    // SetSearchPathMode is not available staring Windows Server 2016 Nano
#if WINVER >= _WIN32_WINNT_WIN7 && WINVER < _WIN32_WINNT_WIN10
    SetSearchPathMode (BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT); // Windows 7+
#else
    BOOL (WINAPI * pfnSetSearchPathMode) (DWORD);
    if (Windows::Symbol (kernel32, pfnSetSearchPathMode, "SetSearchPathMode")) {
        if (!pfnSetSearchPathMode (BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT)) {
            success = false;
        };
    };
#endif

    // RegDisablePredefinedCache(Ex)
    //  - Ex is only on Vista+
    //  - when not using advapi32.dll these calls will get skipped

#if WINVER >= _WIN32_WINNT_VISTA
    RegDisablePredefinedCacheEx ();
#else
    auto advapi32 = GetModuleHandleW (L"ADVAPI32");

    LONG (WINAPI * pfnRegDisablePredefinedCacheEx) ();
    if (Windows::Symbol (advapi32, pfnRegDisablePredefinedCacheEx, "RegDisablePredefinedCacheEx")) {
        auto error = pfnRegDisablePredefinedCacheEx ();
        if (error != ERROR_SUCCESS) {
            success = false;
            SetLastError (error);
        };
    } else {
#if WINVER >= _WIN32_WINNT_WIN2K
        RegDisablePredefinedCache ();
#else
        LONG (WINAPI * pfnRegDisablePredefinedCache) ();
        if (Windows::Symbol (advapi32, pfnRegDisablePredefinedCache, "RegDisablePredefinedCache")) {
            auto error = pfnRegDisablePredefinedCache ();
            if (error != ERROR_SUCCESS) {
                success = false;
                SetLastError (error);
            };
        };
#endif
    };
#endif

    // low-fragmentation heap
    //  - Win2k SP4 and WinXP
    //  - required for HeapEnableTerminationOnCorruption

#if WINVER >= _WIN32_WINNT_WINXP
#if WINVER < _WIN32_WINNT_VISTA
    ULONG mode = 2;
    HeapSetInformation (GetProcessHeap (), HeapCompatibilityInformation, &mode, sizeof mode);
#endif
    HeapSetInformation (GetProcessHeap (), HeapEnableTerminationOnCorruption, NULL, 0);
#else
    BOOL (WINAPI * pfnHeapSetInformation) (HANDLE, UINT, PVOID, SIZE_T);

    if (Windows::Symbol (kernel32, pfnHeapSetInformation, "HeapSetInformation")) {
        
        
        if (!pfnHeapSetInformation (heap, HeapCompatibilityInformation, &mode, sizeof mode)) {
            success = false;
        };
        if (!pfnHeapSetInformation (heap, HeapEnableTerminationOnCorruption, NULL, 0)) {
            success = false;
        };
    };
#endif

    return success;
};

