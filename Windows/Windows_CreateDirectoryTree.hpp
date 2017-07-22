#ifndef WINDOWS_CREATEDIRECTORYTREE_HPP
#define WINDOWS_CREATEDIRECTORYTREE_HPP

/* Windows CreateDirectoryTree 
// Windows_CreateDirectoryTree.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.01.2015 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CreateDirectoryTree
    //  - recursively creates directory tree
    //  - returns: 0 on error (check for GetLastError () == ERROR_ALREADY_EXISTS)
    //             number of created sub-directories on success
    
    unsigned int CreateDirectoryTree (const wchar_t *, LPSECURITY_ATTRIBUTES = NULL);

};

#endif

