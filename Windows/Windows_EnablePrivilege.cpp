#include "Windows_EnablePrivilege.hpp"

/* Windows EnablePrivilege 
// Windows_EnablePrivilege.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      25.10.2014 - initial version
*/

bool Windows::EnablePrivilege (LPCTSTR name, bool enable) {

    HANDLE hToken;
    if (OpenProcessToken (GetCurrentProcess (),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {

        LUID luid;
        if (LookupPrivilegeValue (NULL, name, &luid)) {

            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            tp.Privileges [0] .Luid = luid;

            if (enable)
                tp.Privileges [0] .Attributes = SE_PRIVILEGE_ENABLED;
            else
                tp.Privileges [0] .Attributes = 0;

            if (AdjustTokenPrivileges (hToken, FALSE, &tp, sizeof (TOKEN_PRIVILEGES),
                                       (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL)) {
                
                CloseHandle (hToken);
                return GetLastError () != ERROR_NOT_ALL_ASSIGNED;
            };
        };

        CloseHandle (hToken);
    };

    return false;
};

bool Windows::DisablePrivilege (LPCTSTR name) {
    return Windows::EnablePrivilege (name, false);
};
