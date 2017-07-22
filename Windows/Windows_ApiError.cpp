#include "Windows_ApiError.hpp"

/* Windows ApiError 
// Windows_ApiError.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      25.04.2014 - initial version
*/

#include <windows.h>

#include <cstdio>
#include <cstring>

// TODO: Windows::Initialize ();
// TODO: preallocate exception, TODO: better name

#ifdef __GCC__
int Windows_ApiError_initx () { try { throw 0; } catch (int v) { return v; }; };
int __attribute__((used)) Windows_ApiError_x = Windows_ApiError_initx ();
#endif

unsigned int Windows::ApiError::GetLastError () { return ::GetLastError (); };

namespace {
    void BuildComponent (char * out, unsigned int component) {
        auto n = 4u;
        if (component < 0x00100000) n = 3u;
        if (component < 0x00010000) n = 2u;
        if (component < 0x00000100) n = 1u;
        
        switch (n) {
            case 4u:
                *out++ = component >> 24;
            case 3u:
                *out++ = component >> 16;
            case 2u:
                *out++ = component >> 8;
            case 1u:
                *out++ = component >> 0;
            default:
                *out++ = '\0';
        };
        return;
    };
    void BuildErrorCode (char * out, unsigned int code) {
        if (code >= 100000u && code < 0xFFFF1000u) {
            snprintf (out, 12, "0x%08X", code);
        } else {
            snprintf (out, 12, "%d", (int) code);
        };
    };
    
    std::string BuildWhatString (unsigned int code, unsigned int component,
                                 const char * call = nullptr,
                                 std::size_t longer = 0u) {
        char sz1 [5];
        char sz2 [12];
        BuildComponent (sz1, component);
        BuildErrorCode (sz2, code);
        
        std::string s;
        s.reserve (24 + longer + (call ? std::strlen (call) : 0));
        
        s += "[";
        s += sz1;
        s += ":";
        
        if (call) {
            s += call;
        } else {
            s += "?";
        };
        s += "]: ";
        s += sz2;
        return s;
    };
    std::string BuildWhatString (unsigned int code, unsigned int component, const char * call,
                                 const char * parameter, va_list args) {
        char buffer [4096];
        vsnprintf (buffer, sizeof buffer, parameter, args);
        va_end (args);
        
        auto s = BuildWhatString (code, component, call, std::strlen (buffer));
        s += "\n";
        s += buffer;
        return s;
    };;
};

Windows::ApiError::ApiError (unsigned int component, unsigned int code_)
    :   std::runtime_error (BuildWhatString (code_, component)),
        code (code_),
        component (component),
        call (nullptr) {};

Windows::ApiError::ApiError (const char * call, const char * parameter, ...)
    :   std::runtime_error (!parameter
                                ? BuildWhatString (GetLastError (), 0u, call)
                                : BuildWhatString (GetLastError (), 0u, call, parameter,
                                                   (va_start (va, parameter), va))),
        code (GetLastError ()),
        component (0u),
        call (call) {};

Windows::ApiError::ApiError (unsigned int component, const char * call, const char * parameter, ...)
    :   std::runtime_error (!parameter
                                ? BuildWhatString (GetLastError (), component, call)
                                : BuildWhatString (GetLastError (), component, call, parameter,
                                                   (va_start (va, parameter), va))),
        code (GetLastError ()),
        component (component),
        call (call) {};

Windows::ApiError::ApiError (unsigned int component, unsigned int code, const char * call, const char * parameter, ...)
    :   std::runtime_error (!parameter
                                ? BuildWhatString (code, component, call)
                                : BuildWhatString (code, component, call, parameter,
                                                   (va_start (va, parameter), va))),
        code (code),
        component (component),
        call (call) {};

