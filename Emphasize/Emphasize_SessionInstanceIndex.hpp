#ifndef EMPHASIZE_SESSIONINSTANCEINDEX_HPP
#define EMPHASIZE_SESSIONINSTANCEINDEX_HPP

/* Emphasize Index of Session Instances
// Emphasize_SessionInstanceIndex.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: 
//
// Changelog:
//      15.05.2011 - initial version
*/

#include <windows.h>

namespace Emphasize {
namespace SessionInstanceIndex {
    
    // Initialize
    //  - mutex and share are names of local session objects
    //  - again, no destructor is neccessary since this "singleton" lives
    //    for the lifetime of the process
    
    bool Initialize (const wchar_t * mutex, const wchar_t * share);
    
    // Lock/Release
    //  - enters/leaves mutex protected sequence where "name"
    //    can be safely written and read and "hWnd" registered or removed
    
    bool Lock ();
    void Release ();
    
    // (S/G)etName
    //  - copies the file name (max 32768 wchars) to shared memory
    //  - retrieves pointer to temporary block with the shared file name
    
    bool SetName (const wchar_t * filename);
    const wchar_t * GetName ();
    
    // Enumerate
    //  - calls 'callback' function with handles of windows registered by
    //    all instances, 'void * parameter' is forwarded
    //  - on background does also checks and removes invalid handles
    //    (of instances that crashed or have been killed)
    
    void Enumerate (bool (* callback) (HWND, DWORD, void *), void * parameter);
    
    // Register/Remove
    //  - adds or removes window handle from shared memory
    
    bool Register (HWND hWnd);
    void Remove (HWND hWnd);
    
};
};

#endif
