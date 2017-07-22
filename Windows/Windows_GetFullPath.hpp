#ifndef WINDOWS_GETFULLPATH_HPP
#define WINDOWS_GETFULLPATH_HPP

/* Emphasize Windows routine building full paths
// Windows_GetFullPath.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: When build with UNICODE, converts any path (relative to current
//              directory) to full path with \\?\ or \\?\UNC\ prefix to support
//              up to 32767 characters long file names.
//
// Changelog:
//      17.05.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetFullPath
    //  - calls GetFullPathName and prepends proper prefix
    //  - parameters: LPCTSTR - input path
    //                DWORD   - output buffer length (in TCHARs)
    //                LPTSTR  - output buffer pointes
    //                LPTSTR* - if not NULL, receives pointer to last component
    //                          inside the output buffer
    //  - returns: true on success,
    //             false on failure, GetLastError:
    //              - ERROR_BUFFER_OVERFLOW if the buffer was too small
    //              - other code set by GetFullPathName
    
    bool GetFullPath (LPCTSTR, DWORD, LPTSTR, LPTSTR* = NULL);
    
};

#endif
