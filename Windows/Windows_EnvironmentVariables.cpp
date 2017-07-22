#include "Windows_EnvironmentVariables.hpp"

/* Emphasize Windows Environment Variables helper functions
// Windows_EnvironmentVariables.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      08.05.2011 - initial version
*/

#include <cstring>
#include <cwchar>

bool Windows::CompareEnvironmentVariable (LPCTSTR lpName, LPCTSTR lpValue) {
    TCHAR value [32768u];
    if (GetEnvironmentVariable (lpName, value,
                                sizeof value / sizeof value [0])) {
#ifdef UNICODE
        if (!std::wcscmp (value, lpValue))
#else
        if (!std::strcmp (value, lpValue))
#endif
            return true;
    };
    
    return false;
};

UINT Windows::GetEnvironmentVariableUint (LPCTSTR lpName) {
    TCHAR value [1536u];
    value [0] = L'\0';
    
    if (GetEnvironmentVariable (lpName, value,
                                sizeof value / sizeof value [0])) {
#ifdef UNICODE
        return std::wcstoul (value, NULL, 0);
#else
        return std::strtoul (value, NULL, 0);
#endif
    } else
        return 0u;
};

