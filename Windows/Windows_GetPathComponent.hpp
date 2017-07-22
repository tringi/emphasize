#ifndef WINDOWS_GETPATHCOMPONENT_HPP
#define WINDOWS_GETPATHCOMPONENT_HPP

/* Windows GetPathComponent 
// Windows_GetPathComponent.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Description: Used to retrieve indexed directory or file name from fully
//              qualified or UNC path. For paths prefixed with \\?\ or \\.\
//              returns \? or \. respectively as the first component.
//
// Changelog:
//      08.04.2013 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetPathComponent
    //  - returns pointer to the 'index'-th component in 'path'
    //    and optionally sets the 'length' to length of that component
    
    const wchar_t * GetPathComponent (const wchar_t * path, int index, unsigned int * = NULL);
          wchar_t * GetPathComponent (      wchar_t * path, int index, unsigned int * = NULL);
    
    // GetPathComponent
    //  - this version copies index-th path component to buffer (of len wchars)
    //     - for valid NTFS paths, the path component length is 256 or less
    //  - returns: true on success
    //             false on error, call GetLastError for further details:
    //              - ERROR_INVALID_INDEX - 'index' out of range
    //              - ERROR_INSUFFICIENT_BUFFER - buffer 'length' too small
    
    bool GetPathComponent (const wchar_t * path, int index,
                           wchar_t * buffer, unsigned int length);
    
};

#endif

