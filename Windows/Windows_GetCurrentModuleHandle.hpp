#ifndef WINDOWS_GETCURRENTMODULEHANDLE_HPP
#define WINDOWS_GETCURRENTMODULEHANDLE_HPP

/* Emphasize Windows current module helper
// Windows_GetCurrentModuleHandle.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: See http://my.opera.com/Tringi/blog/getcurrentmodulehandle
//
// Changelog:
//      17.05.2011 - initial version
//      10.06.2016 - constexpr
*/

extern "C" char __ImageBase;
namespace Windows {
    
    // GetCurrentModuleHandle
    //  - retrieves handle to the EXE or DLL in which this function resides
    //  - requires MinGW or MSVC to get __ImageBase symbol
    
    constexpr static inline HMODULE GetCurrentModuleHandle () {
        return reinterpret_cast <HMODULE> (&__ImageBase);
    };
};

#endif
