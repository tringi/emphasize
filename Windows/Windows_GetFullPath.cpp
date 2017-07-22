#include "Windows_GetFullPath.hpp"

/* Emphasize Windows routine building full paths
// Windows_GetFullPath.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      17.05.2011 - initial version
//      04.07.2011 - added branch for already full-path input
//      15.01.2015 - added SetLastError (ERROR_BUFFER_OVERFLOW) reporting
*/

#ifdef UNICODE
#include <cwchar>

bool Windows::GetFullPath (LPCTSTR lpFileName, DWORD nBufferLength,
                           LPTSTR lpBuffer, LPTSTR* lpFilePart) {
    
    // check if path has \\?\ prefix and is thus already a full path
    //  - then just copy it to output and easily find last component
    
    if (   lpFileName[0] == L'\\'
        && lpFileName[1] == L'\\'
        && lpFileName[2] == L'?'
        && lpFileName[3] == L'\\') {
        
        if (std::wcslen (lpFileName) < nBufferLength) {
            std::wcscpy (lpBuffer, lpFileName);
            
            if (lpFilePart) {
                if (const wchar_t * p = std::wcsrchr (lpFileName, L'\\'))
                    *lpFilePart = lpBuffer + (p - lpFileName) + 1;
                else
                    *lpFilePart = NULL;
            };
            
            return true;
        } else {
            SetLastError (ERROR_BUFFER_OVERFLOW);
            return false;
        };
    };
    
    const bool unc = lpFileName[0] == L'\\'
                  && lpFileName[1] == L'\\';
    
    // sanity check for output buffer size
    //  - \\?\UNC\a\b\c - 14 characters with NUL terminator
    //  - \\?\C:\a - 9 characters with NUL terminator
    
    if (   (unc == true  && nBufferLength >= 14u)
        || (unc == false && nBufferLength >= 9u)) {
        
        // prefix length
        //  - UNC use 6 (not 7) because first '\\' is replaced by 'C'
        
        const int prefix = unc ? 6 : 4;
        const DWORD r = GetFullPathName (lpFileName,
                                         nBufferLength - prefix, lpBuffer + prefix,
                                         lpFilePart);

        if (r) {
            if (r < nBufferLength - prefix) {
    
                lpBuffer [0] = L'\\';
                lpBuffer [1] = L'\\';
                lpBuffer [2] = L'?';
                lpBuffer [3] = L'\\';
            
                if (unc) {
                    lpBuffer [4] = L'U';
                    lpBuffer [5] = L'N';
                    lpBuffer [6] = L'C';
                };
                
                return true;
            } else {
                SetLastError (ERROR_BUFFER_OVERFLOW);
            };
        };
    } else {
        SetLastError (ERROR_BUFFER_OVERFLOW);
    };
    
    return false;
};

#else

bool Windows::GetFullPath (LPCTSTR lpFileName, DWORD nBufferLength,
                           LPTSTR lpBuffer, LPTSTR* lpFilePart) {
    return GetFullPathName (lpFileName, nBufferLength, lpBuffer, lpFilePart);
};

#endif

