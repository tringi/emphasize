#include "Emphasize_Settings_Main.hpp"

/* Emphasize Settings class main implementation
// Emphasize_Settings_Main.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      20.06.2012 - initial version
*/

#include "Emphasize_Settings_Base.hpp"
#include "Emphasize_Settings_Store.hpp"
#include "Emphasize_Setting.hpp"

#include <cstdio>
#include <cwctype>
#include <algorithm>

Emphasize::Settings::Settings () noexcept {
    this->settings.first = NULL;
    this->settings.last = NULL;
    this->stores.first = NULL;
    this->stores.last = NULL;
    this->stores.active = NULL;
    return;
};

void Emphasize::Settings::initialize_defaults () {
    auto setting = this->settings.first;

    while (setting) {
        setting->after_load ();
        setting = setting->next;
    };
};

// cleanup
//  - implemented recursively to first close last Store

void Emphasize::Settings::cleanup () const {
    this->cleanup (this->stores.first);
    this->stores.active = NULL;
    return;
};
void Emphasize::Settings::cleanup (Store * store) const {
    if (store && store->mark) {
        this->cleanup (store->next);
        
        if (store->open) {
            store->Close ();
            store->open = false;
        };
        store->mark = false;
    };
    return;
};

bool Emphasize::Settings::load () const {
    bool result = true;
    auto setting = this->settings.first;
    
    while (setting) {
        result &= this->load_step (setting);
        setting = setting->next;
    };
    
    this->cleanup ();
    return result;
};

bool Emphasize::Settings::save () {
    bool result = true;
    auto setting = this->settings.first;
    
    while (setting) {
        result &= this->save_step (setting);
        setting = setting->next;
    };
    
    this->cleanup ();
    return result;
};

bool Emphasize::Settings::erase () {
    bool result = true;
    auto setting = this->settings.first;
    
    while (setting) {
        result &= this->erase_step (setting);
        setting = setting->next;
    };
    
    this->cleanup ();
    return result;
};

bool Emphasize::Settings::load (const Settings::Base * setting) {
    bool result = this->load_step (setting);
    this->cleanup ();
    return result;
};
bool Emphasize::Settings::save (Settings::Base * setting) {
    bool result = this->save_step (setting);
    this->cleanup ();
    return result;
};
bool Emphasize::Settings::erase (Settings::Base * setting) {
    if (this->stores.active) {
        return this->stores.active->Erase (setting->path, setting->name);
        
    } else {
        bool result = this->erase_step (setting);
        this->cleanup ();
        return result;
    };
};

bool Emphasize::Settings::load_step (const Settings::Base * setting) const {
    auto store = this->stores.first;
    while (store) {
        
        // Open
        //  - for read-only access
        //  - only if not tried in this batch load (.mark member)
        
        if (!store->mark && !store->open) {
            if (store->Read ()) {
                store->open = true;
            };
            store->mark = true;
        };
        
        // Read the setting data
        //  - load function call one of our overloaded version of .load
        //  - break stores loop when loaded correctly
        
        if (store->open) {
            this->stores.active = store;
            if (setting->load ()) {
                setting->after_load ();
                return true;
            };
        };
        
        // Try next store
        //  - if not successfully read
        
        store = store->next;
    };
    return false;
};

bool Emphasize::Settings::save_step (Settings::Base * setting) {
    
    auto store = this->stores.first;
    while (store) {
        
        // Open
        //  - for write access
        //  - only if not tried in this batch save (.mark member)
        
        if (!store->mark && !store->open) {
            if (store->Write ()) {
                store->open = true;
            };
            store->mark = true;
        };
        
        // Write the setting data
        //  - save function call one of our overloaded version of .save
        //  - break stores loop once saved correctly
        
        if (store->open) {
            this->stores.active = store;
            setting->before_save ();
            if (setting->save ())
                return true;
        };
        
        // Try next store
        //  - if not successfully saved
        
        store = store->next;
    };
    return false;
};

bool Emphasize::Settings::erase_step (Settings::Base * setting) {
    
    auto store = this->stores.first;
    while (store) {
        
        // Open
        //  - for write access
        //  - only if not tried in this batch save (.mark member)
        
        if (!store->mark && !store->open) {
            if (store->Write ()) {
                store->open = true;
            };
            store->mark = true;
        };
        
        // Erase the setting data
        //  - break stores loop once erased correctly... is this right ????
        
        if (store->open) {
            this->stores.active = store;
            if (setting->erase ())
                return true;
        };
        
        // Try next store
        //  - if not successfully erased ...is this right ????
        
        store = store->next;
    };
    return false;
};

bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                char * string, std::size_t length) {
    return this->save (setting, const_cast <const char *> (string), length);
};
bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                wchar_t * string, std::size_t length) {
    return this->save (setting, const_cast <const wchar_t *> (string), length);
};

bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                const char * string, std::size_t n) {
    if (this->stores.active) {
        if (n == sizeof (char)) {
            char temp [2] = { *string, '\0' };
            return this->stores.active->Set (setting->path, setting->name, temp);
        } else
            return this->stores.active->Set (setting->path, setting->name, string);
    } else
        return this->save (setting);
};

bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                const wchar_t * string, std::size_t n) {
    if (this->stores.active) {
        if (n == sizeof (wchar_t)) {
            wchar_t temp [2] = { *string, '\0' };
            return this->stores.active->Set (setting->path, setting->name, temp);
        } else
            return this->stores.active->Set (setting->path, setting->name, string);
    } else
        return this->save (setting);
};

int Emphasize::Settings::HexOffset (const wchar_t * str) {
    switch (str [0]) {
        case L'%':
        case L'#':
        case L'$':
            return 1;
        case L'0':
            switch (str [1]) {
                case L'x':
                case L'X':
                case L'h':
                case L'H':
                    return 2;
                default:
                    return 0;
            };
        case L'&':
            switch (str [1]) {
                case L'H':
                    return 2;
                case L'#':
                    if (str [2] == L'x')
                        return 3;
                    else
                        return 1;
                default:
                    return 1;
            };
        default:
            return 0;
    };
};

#define DEFINE_FORWARD_DISPATCH(TT,From,To)\
bool Emphasize::Settings::TT (const wchar_t * path, const wchar_t * name,   \
                              To * value,                                   \
                              std::size_t length, std::size_t size) {       \
        From result;                                                        \
        if (this->TT (path, name, &result, length, size)) {                 \
            *value = (To) result;                                           \
            return true;                                                    \
        } else                                                              \
            return false;                                                   \
    }

    DEFINE_FORWARD_DISPATCH (LoadStr,   signed long,   signed char);
    DEFINE_FORWARD_DISPATCH (LoadStr, unsigned long, unsigned char);
    DEFINE_FORWARD_DISPATCH (LoadStr,   signed long,   signed short);
    DEFINE_FORWARD_DISPATCH (LoadStr, unsigned long, unsigned short);
    DEFINE_FORWARD_DISPATCH (LoadStr,   signed long,   signed int);
    DEFINE_FORWARD_DISPATCH (LoadStr, unsigned long, unsigned int);
#undef DEFINE_FORWARD_DISPATCH

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   bool * value, std::size_t, std::size_t) {
    wchar_t string [12];
    if (this->stores.active->Get (path, name, string, sizeof string)) {

        int offset = Settings::HexOffset (string);
        wchar_t * rest = NULL;
        
        if (std::wcstol (string + offset, &rest, offset ? 16 : 10) == 0
                && *rest == '\0') {
            
            *value = false;
            return true;
        };
        
        if (std::wcstod (string, &rest) == 0.0
                && *rest == '\0') {

            *value = false;
            return true;
        };
        
        for (auto i = 0u; i < sizeof string / sizeof string [0]; ++i) {
            if (std::iswupper (string [i]))
                string [i] = std::towlower (string [i]);
        };
        
        if ((string [0] == L'f' ||
             string [0] == L'n' ||
             string [0] == L'o' ||
             string [0] == L'x') && string [1] == L'\0') {
            
            *value = false;
            return true;
        };

        if (   std::wcscmp (string, L"false") == 0
            || std::wcscmp (string, L"no") == 0) {
            
            *value = false;
            return true;
        };

        *value = true;
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   signed long * value,
                                   std::size_t, std::size_t) {
    wchar_t string [12];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        
        int offset = Settings::HexOffset (string);
        *value = std::wcstol (string + offset, NULL, offset ? 16 : 10);
        
        return true;
    } else
        return false;
};
    
bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   unsigned long * value,
                                   std::size_t, std::size_t) {
    wchar_t string [12];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        
        int offset = Settings::HexOffset (string);
        *value = std::wcstoul (string + offset, NULL, offset ? 16 : 10);
        
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   signed long long * value,
                                   std::size_t, std::size_t) {
    wchar_t string [24];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        
        int offset = Settings::HexOffset (string);
        *value = std::wcstoll (string + offset, NULL, offset ? 16 : 10);
        
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   unsigned long long * value,
                                   std::size_t, std::size_t) {
    wchar_t string [24];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        
        int offset = Settings::HexOffset (string);
        *value = std::wcstoull (string + offset, NULL, offset ? 16 : 10);
        
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   float * value,
                                   std::size_t, std::size_t) {
    wchar_t string [24];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        *value = std::wcstof (string, NULL);
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   double * value,
                                   std::size_t, std::size_t) {
    wchar_t string [32];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        *value = std::wcstod (string, NULL);
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   long double * value,
                                   std::size_t, std::size_t) {
    wchar_t string [48];
    if (this->stores.active->Get (path, name, string, sizeof string)) {
        *value = std::wcstold (string, NULL);
        return true;
    } else
        return false;
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   wchar_t * value,
                                   std::size_t length, std::size_t size) {
    
    if (length == 1u * sizeof (wchar_t)
        && (size == 2u * sizeof (wchar_t) || size == 1u * sizeof (wchar_t))) {
        
        // special case for reading single character into plain variable
        //  - Query might return 1 or 2 (for NUL-terminator)
    
        wchar_t temporary [2u];
        if (this->stores.active->Get (path, name, temporary, size)) {
            *value = temporary [0u];
            return true;
        } else
            return false;
    } else
        return this->stores.active->Get (path, name, value, length * sizeof (wchar_t));
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   wchar_t ** value,
                                   std::size_t length, std::size_t size) {
    
    const auto heap = GetProcessHeap ();

    if (size == (std::size_t) -1)
        size = 65536u;
    
    // check buffer
    //  - there is usually none when the Setting does not use default

    if (*value == NULL) {
        *value = static_cast <wchar_t *> (HeapAlloc (heap, 0, size));

        if (*value != NULL) {
            length = size;
        } else
            return false;
        
    } else {
        
        // check buffer space
        //  - re-allocate upwards if neccessary
        //  - also re-allocate downwards if there would be large waste of memory
        
        if (length < size
                || (length > 32 && (size < length / 4))) {
                    
            if (wchar_t * re = static_cast <wchar_t *>
                                (HeapReAlloc (heap, 0, *value, size))) {
                *value = re;
                length = size;
            } else
                return false;
        };
    };
    
    return this->stores.active->Get (path, name, *value, length);
};

// StringType -> char*
//  - converting from wide-char string
//  - size - size in bytes of data retrieved by .Query
//  - length - number of characters we can write into 'value' (NUL not included)

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   char * value,
                                   std::size_t length, std::size_t size) {
    if (size == (std::size_t) -1)
        size = 65536u; // TODO: shrinking for auto-sized buffers?

    static const unsigned int NSZ = 32768u;
    if (size <= NSZ * sizeof (wchar_t)) {
        
        // short strings are converted through stack buffer

        wchar_t wstring [NSZ];
        return this->stores.active->Get (path, name, wstring, size)
            && WideCharToMultiByte (CP_ACP, 0, wstring, -1,
                                    value, (int) length, NULL, NULL);
    } else {
        
        // allocate memory for long strings
        
        auto h = GetProcessHeap ();
        if (auto wstring = static_cast <wchar_t *> (HeapAlloc (h, 0, size))) {
            
            bool result = this->stores.active->Get (path, name, wstring, size)
                       && WideCharToMultiByte (CP_ACP, 0, wstring, -1,
                                               value, (int) length, NULL, NULL);
            HeapFree (h, 0, wstring);
            return result;
        } else
            return false;
    };
};

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   char ** value,
                                   std::size_t length, std::size_t size) {
    
    const auto heap = GetProcessHeap ();

    if (size == (std::size_t) -1) // TODO: shrinking for auto-sized buffers (INI)
        size = 65536u;
    
    size /= sizeof (wchar_t);
    
    // check buffer space
    //  - re-allocate upwards if neccessary
    //  - also re-allocate downwards if there would be large waste of memory
    
    if (length < size
            || (length > 32 && (size < length / 4))) {
                
        if (char * re = static_cast <char *>
                            (HeapReAlloc (heap, 0, *value, size))) {
            *value = re;
            length = size;
        } else
            return false;
    };
    
    // forward to normal loading
    //  - for wchar -> char conversion
    
    return this->LoadStr (path, name, *value, length, size * sizeof (wchar_t));
};


// StringType -> void*
//  - ensures that the stored 'size' match our variable's 'length'
//  - retrieves string to temporary buffer and decodes it byte by byte

bool Emphasize::Settings::LoadStr (const wchar_t * path, const wchar_t * name,
                                   void * value,
                                   std::size_t length, std::size_t size) {
    if (size == (std::size_t) -1)
        size = 2u * length;
    else
    if (size != 2u * length)
        return false;

    bool result = false;
    auto heap = GetProcessHeap ();
    auto string = static_cast <wchar_t *> (HeapAlloc (heap, 0,
                                            (size + 1) * sizeof (wchar_t)));
    
    if (this->stores.active->Get (path, name, string,
                                  (size + 1) * sizeof (wchar_t))) {
        
        struct hexa {
            static bool is (wchar_t c) {
                return (c >= L'0' && c <= L'9')
                    || (c >= L'a' && c <= L'f')
                    || (c >= L'A' && c <= L'F');
            };
            static unsigned int value (wchar_t c) {
                if (c >= L'0' && c <= L'9') return c - L'0';
                if (c >= L'a' && c <= L'f') return c - L'a' + 10;
                if (c >= L'A' && c <= L'F') return c - L'A' + 10;

                return 0;
            };
        };
        
        // check for validity
        
        result = true;
        for (std::size_t i = 0u; i != size; ++i)
            if (!hexa::is (string [i])) {
                result = false;
                break;
            };
        
        // decode
        
        if (result)
            for (std::size_t i = 0u; i != length; ++i) {
                static_cast <unsigned char *> (value) [i]
                    = hexa::value (string [2u * i + 0u]) * 16
                    + hexa::value (string [2u * i + 1u]);
            };
    };

    HeapFree (heap, 0, string);
    return result;
};

bool Emphasize::Settings::LoadBin (const wchar_t * path, const wchar_t * name,
                                   void * value,
                                   std::size_t length, std::size_t size) {
    if (length <= size)
        return this->stores.active->Get (path, name, value, length);
    else
        return false;
};

bool Emphasize::Settings::LoadBin (const wchar_t * path, const wchar_t * name,
                                   char * value,
                                   std::size_t length, std::size_t size) {
    if (length <= size)
        return this->stores.active->Get (path, name, value, length);
    else
        return false;
};
bool Emphasize::Settings::LoadBin (const wchar_t * path, const wchar_t * name,
                                   wchar_t * value,
                                   std::size_t length, std::size_t size) {
    if (length <= size) {
        if (this->stores.active->Get (path, name, value, length)) {
            
            value [std::min (length / sizeof (wchar_t) - 1u, size)] = L'\0';
            return true;
        };
    };
    return false;
};

bool Emphasize::Settings::LoadInt (const wchar_t * path, const wchar_t * name,
                                   void * value,
                                   std::size_t length, std::size_t size) {
    if (length == size) {
        return this->stores.active->Get (path, name, value, length);
        
    } else {
        unsigned char temporary [4];
        if (length <= sizeof temporary) {
            
            std::memset (temporary, 0, sizeof temporary);
            if (this->stores.active->Get (path, name, temporary, sizeof temporary)) {
                
                std::memcpy (value, temporary, length);
                return true;
                
            } else
                return false;
        } else
            return false;
    };
};
