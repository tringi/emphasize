#include "Windows_GetPathComponent.hpp"

/* Windows GetPathComponent 
// Windows_GetPathComponent.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      08.04.2013 - initial version
*/

#include <cstddef>

namespace {
    bool Implementation (const wchar_t * path, int index,
                         std::ptrdiff_t & offset, unsigned int * length);
};

const wchar_t * Windows::GetPathComponent (const wchar_t * path,
                                           int index, unsigned int * length) {
    std::ptrdiff_t offset = 0;
    if (Implementation (path, index, offset, length)) {
        return path + offset;
    } else {
        SetLastError (ERROR_INVALID_INDEX);
        return NULL;
    };
};

wchar_t * Windows::GetPathComponent (wchar_t * path,
                                     int index, unsigned int * length) {
    std::ptrdiff_t offset = 0;
    if (Implementation (path, index, offset, length)) {
        return path + offset;
    } else {
        SetLastError (ERROR_INVALID_INDEX);
        return NULL;
    };
};

bool Windows::GetPathComponent (const wchar_t * path, int index,
                                wchar_t * buffer, unsigned int length) {
    unsigned int size;
    if (const wchar_t * ptr = Windows::GetPathComponent (path, index, &size)) {
        if (size < length) {
            
            for (unsigned int i = 0u; i != size; ++i) {
                buffer [i] = ptr [i];
            };
            buffer [size] = L'\0';
            return true;
        } else {
            SetLastError (ERROR_INSUFFICIENT_BUFFER);
        };
    };
    
    return false;
};

namespace {
    int ComponentCount (const wchar_t * path) {
        int n = 0;
        if (path) {
            ++n;
            do {
                if (*path == L'\\' || *path == L'/') {
                    ++n;
                };
            } while (*path++);
        };
        return n;
    };

    const wchar_t * ComponentNext (const wchar_t * p) {
        while (*p && (*p != L'\\') && (*p != L'/')) {
            ++p;
        };
        return p;
    };
    
    unsigned int ComponentLength (const wchar_t * path) {
        return ComponentNext (path) - path;
    };
    
    bool Implementation (const wchar_t * path, int index,
                         std::ptrdiff_t & offset, unsigned int * length) {
        const wchar_t * p = path;
        
        bool prefixed = false;
        std::ptrdiff_t fix = 0;
        
        if (*p == L'\\') {
            ++p;
            if (*p == L'\\') {
                prefixed = true;
                
                if (index >= 0)
                    index += 1;
            };
        } else
        if (*p == L'/') {
            ++p;
        };
        
        if (index < 0) {
            index = ComponentCount (p) + index;
        };
        
        if (prefixed) {
            if (index == 1) {
                fix = 1;
            };
            if (index == 0)
                return false;
        };
        
        if (index) {
            do {
                p = ComponentNext (p);
                if (!*p)
                    return false;
                
                ++p;
            } while (--index);
        };
        
        offset = p - path - fix;
        if (length)
            *length = ComponentLength (p) + fix;
        
        return true;
    };
};
