#include "Emphasize_SessionInstanceIndex.hpp"

/* Emphasize Index of Session Instances
// Emphasize_SessionInstanceIndex.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: 
//
// Changelog:
//      15.05.2011 - initial version
*/

#include <cstring>
#include <cwchar>
#include <cstdio>

namespace {
    HANDLE hMutex = NULL;
    void * pShare = NULL;
    
    struct Instance {
        HWND    hWnd;
        DWORD   pid;
        DWORD   tid;
        ATOM    atom;
        WORD    reserved;
        
        void Clear () volatile {
            this->hWnd = NULL;
            this->pid = 0u;
            this->tid = 0u;
            this->atom = 0u;
        };
    };
    
    unsigned int         locked = false;
    volatile Instance *  instances = NULL;
    volatile wchar_t *   string = NULL;
    
    // prefix size
    //  - 3072 means that in single allocation page of 4096 bytes there is
    //    space for 192 (3584/sizeof(instance)) instances,
    //    while allowing filenames of 511 wchars without extending the mapping
    //    which is ~2 times longer than explorer and/or old shell will allow
    
    static const unsigned int prefix = 3072u;
    static const unsigned int maximum = prefix / sizeof (Instance);
};

bool Emphasize::SessionInstanceIndex::Initialize (const wchar_t * mutex_name,
                                                  const wchar_t * share_name) {
    
    // size
    //  - mapping object size
    //  - keeps instances index + max size of "broadcasted" string (path max)
    
    static const unsigned int size = prefix + sizeof (wchar_t) * 32768u;
    
    // create neccessary handles
    //  - TODO: we will probably need EVERYONE security descriptor here!
    
    if ((hMutex = CreateMutex (NULL, FALSE, mutex_name))) {
        if (HANDLE hShare = CreateFileMapping (INVALID_HANDLE_VALUE, NULL,
                                               PAGE_READWRITE | SEC_RESERVE,
                                               0, size, share_name)) {
            if ((pShare = MapViewOfFile (hShare, FILE_MAP_ALL_ACCESS, 0,0,0))) {
                if (VirtualAlloc (pShare, 4096u, MEM_COMMIT, PAGE_READWRITE)) {
                    
                    instances = static_cast <volatile Instance *> (pShare);
                    string = static_cast <wchar_t *> (pShare)
                           + prefix / sizeof (wchar_t);
                    return true;
                };
            };
            
            // handle can be closed, no longer necessary
            CloseHandle (hShare);
        };
    };
    
    // on initialization error set hMutex to NULL
    //  - application will probably want to continue anyway
    
    hMutex = NULL;
    return false;
};

bool Emphasize::SessionInstanceIndex::Lock () {
    if (WaitForSingleObject (hMutex, INFINITE) != WAIT_FAILED) {
        ++locked;
        return true;
    } else
        return false;
};
void Emphasize::SessionInstanceIndex::Release () {
    if (hMutex && locked) {
        ReleaseMutex (hMutex);
        --locked;
    };
    return;
};

bool Emphasize::SessionInstanceIndex::SetName (const wchar_t * filename) {
    if (locked) {
        auto length = sizeof (wchar_t) * (std::wcslen (filename) + 1u);
        
        if (VirtualAlloc (pShare, prefix + length, MEM_COMMIT, PAGE_READWRITE)) {
            std::memcpy (const_cast <wchar_t *> (string), filename, length);
            return true;
        };
    };
    return false;
};
const wchar_t * Emphasize::SessionInstanceIndex::GetName () {
    return const_cast <wchar_t *> (string);
};

void Emphasize::SessionInstanceIndex::Enumerate (bool (* cb) (HWND, DWORD, void *),
                                                 void * parameter) {
    if (locked) {
        
        // iterating backwards
        //  - this way the queries will not interfere much with older opened
        //    windows (better user experience and also swaping optimization)
        //  - forward iteration: "for (unsigned int i = 0u; i < maximum; ++i)"

        for (unsigned int i = maximum; i--; )
        if (instances[i].hWnd) {
            
            // validate window handles
            //  - this memory is volatile, some of the instances could crash
            //    or be killed while writting it
            //  - if the window handle or pid and tid numbers were recycled
            //    they won't probably be bound together, nevertheless further
            //    check is neccessary
            
            if (IsWindow (instances[i].hWnd)) {
                
                DWORD pid = 0;
                DWORD tid = GetWindowThreadProcessId (instances[i].hWnd, &pid);
                ATOM atom = (ATOM) GetClassLongPtr (instances[i].hWnd, GCW_ATOM);
                
                if (   instances[i].pid == pid
                    && instances[i].tid == tid
                    && instances[i].atom == atom) {
                    
                    if (cb (instances[i].hWnd, instances[i].pid, parameter))
                        break;
                    
                } else {
                    instances[i].Clear ();
                };
            } else {
                instances[i].Clear ();
            };
        };
    };
    return;
};

bool Emphasize::SessionInstanceIndex::Register (HWND hWnd) {
    if (locked) {
        for (auto i = 0u; i != maximum; ++i)
            if (instances[i].hWnd == NULL) {
                instances[i].hWnd = hWnd;
                instances[i].atom = (ATOM) GetClassLongPtr (hWnd, GCW_ATOM);
                instances[i].pid = GetCurrentProcessId ();
                instances[i].tid = GetCurrentThreadId ();
                return true;
            };
    };
    return false;
};

void Emphasize::SessionInstanceIndex::Remove (HWND hWnd) {
    if (hMutex)
    for (auto i = 0u; i != maximum; ++i)
        if (instances[i].hWnd == hWnd) {
            instances[i].Clear ();
            break;
        };
    return;
};


