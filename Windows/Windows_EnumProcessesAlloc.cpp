#include "Windows_EnumProcessesAlloc.hpp"

/* Windows EnumProcessesAlloc 
// Windows_EnumProcessesAlloc.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      08.04.2013 - initial version
*/

#include <psapi.h>

DWORD * Windows::EnumProcessesAlloc () {
    HANDLE heap = GetProcessHeap ();
    DWORD size = 64u * sizeof (DWORD);
    DWORD * p = (DWORD *) HeapAlloc (heap, 0, size);
    
    if (p) {
        bool rv;
        DWORD rets;
        while ((rv = EnumProcesses (p, size, &rets)) && (rets == size)) {
            
            size *= 2u;
            if (DWORD * q = (DWORD *) HeapReAlloc (heap, 0, p, size)) {
                p = q;
            } else {
                HeapFree (heap, 0, p);
                return NULL;
            };
        };
        
        if (rv) {
            p [rets / sizeof (DWORD)] = 0; // sentinel zero
            return p;
        } else {
            HeapFree (heap, 0, p);
            return NULL;
        };
    } else
        return NULL;
};

