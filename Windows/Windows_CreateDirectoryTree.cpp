#include "Windows_CreateDirectoryTree.hpp"

/* Windows CreateDirectoryTree 
// Windows_CreateDirectoryTree.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.01.2015 - initial version
*/

#include "Windows_GetFullPath.hpp"
#include <cwchar>

namespace {
    bool CreateStep (const wchar_t *, unsigned &, LPSECURITY_ATTRIBUTES);
};

unsigned int Windows::CreateDirectoryTree (const wchar_t * tree,
                                           LPSECURITY_ATTRIBUTES sa) {
    
    wchar_t path [32768u];
    if (Windows::GetFullPath (tree, sizeof path / sizeof path [0], path, NULL)) {
        
        unsigned n = 1u;
        if (CreateStep (path, n, sa))
            return n;
    };
    
    return 0u;
};

namespace {
bool CreateStep (const wchar_t * path, unsigned & n, LPSECURITY_ATTRIBUTES sa) {
    
    if (CreateDirectory (path, sa))
        return true;
    
    if (GetLastError () == ERROR_PATH_NOT_FOUND) {
        if (wchar_t * slash = (wchar_t *) std::wcsrchr (path, L'\\')) {
            
            *slash = L'\0';
            if (CreateStep (path, n, sa)) {
                *slash = L'\\';

                if (CreateDirectory (path, sa)) {
                    ++n;
                    return true;
                };
            };
        };
    };
    
    return false;
};
};
