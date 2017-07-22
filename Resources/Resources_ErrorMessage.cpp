#include "Resources_ErrorMessage.hpp"

/* Emphasize Resources library standard Error Message support
// Resources_ErrorMessage.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.2
//
// Changelog:
//      29.05.2011 - initial version
*/

#include "Resources_String.hpp"
#include <cstring>

namespace {
    void * override_parameter = NULL;
    void * supplement_parameter = NULL;
    
    Resources::ErrorMessageAugmentFunction override = NULL;
    Resources::ErrorMessageAugmentFunction supplement = NULL;
};

void Resources::SetErrorMessageOverride (ErrorMessageAugmentFunction function,
                                         void * parameter) {
    override = function;
    override_parameter = parameter;
    return;
};
void Resources::SetErrorMessageSupplement (ErrorMessageAugmentFunction function,
                                           void * parameter) {
    supplement = function;
    supplement_parameter = parameter;
    return;
};

unsigned int Resources::ErrorMessage (wchar_t * buffer, unsigned int length,
                                      unsigned int code, unsigned int _lang) {
    
    // user override for the code
    //  - if override function is set
    //  - if such code is overrided
    
    if (override) {
        if (unsigned int id = override (code, override_parameter)) {
            
            unsigned int size = 0u;
            const wchar_t * string = Resources::String (id, &size, _lang);
            
            if (string) {
                if (size >= length)
                    size = length - 1u;
                
                std::memcpy (buffer, string, size * sizeof (wchar_t));
                buffer [size] = L'\0';
                return size + 1u;
            };
        };
    };
    
    // request string for the error code from system
    //  - querying default system message table and ntdll.dll table
    
    const DWORD base = FORMAT_MESSAGE_IGNORE_INSERTS
                     | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    const struct {
        DWORD   flags;
        HMODULE handle;
    } parameters [] = {
        { base | FORMAT_MESSAGE_FROM_SYSTEM,  NULL },
        { base | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle (L"ntdll") },
        { base | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle (L"wininet") },
        { base | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle (L"tapiui") },
    };
    
    for (unsigned i = 0u; i < sizeof parameters / sizeof parameters[0]; ++i) {
        if (i == 0u || parameters [i] .handle) {
        
            unsigned int n = 0u;
            unsigned int language = _lang;
            do {
                SetLastError (0);
                n = FormatMessage (parameters [i].flags, parameters [i] .handle,
                                   code, language, buffer, length, NULL);
                if (n) {
                    return n;
                } else {
                        
                    // 1] on first error, set sublang to SUBLANG_DEFAULT
                    // 2] on second error, set sublang to SUBLANG_NEUTRAL
                    // 3] on third error, set language to 0
                    //     - system documented behavior tries neutral, user, system
                    //       and english language
                    
                    if (language) {
                        if (SUBLANGID (language) != SUBLANG_NEUTRAL) {
                            if (SUBLANGID (language) != SUBLANG_DEFAULT)
                                language = MAKELANGID (PRIMARYLANGID (language),
                                                       SUBLANG_DEFAULT);
                            else
                            if (SUBLANGID (language) != SUBLANG_NEUTRAL)
                                language = MAKELANGID (PRIMARYLANGID (language),
                                                       SUBLANG_NEUTRAL);
                        } else
                            language = 0;
                    } else
                        break;
                };
            } while (language || !n);
        };
    };
    
    // no string is available from system, try user supplement callback
    //  - if supplement function is set
    //  - if such code is supplemented
    
    if (supplement) {
        if (unsigned int id = supplement (code, supplement_parameter)) {
            
            unsigned int size = 0u;
            const wchar_t * string = Resources::String (id, &size, _lang);
            
            if (string) {
                if (size >= length)
                    size = length - 1u;
                
                std::memcpy (buffer, string, size * sizeof (wchar_t));
                buffer [size] = L'\0';
                return size + 1u;
            };
        };
    };
    
    return 0u;
};
