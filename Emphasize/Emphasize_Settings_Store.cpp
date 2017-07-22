#include "Emphasize_Settings_Store.hpp"

/* Emphasize Settings_Store 
// Emphasize_Settings_Store.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      16.06.2012 - initial version
*/

#include <windows.h>
#include <cwctype>
#include <cwchar>
#include <cstdio>

Emphasize::Settings::Store::Store (Emphasize::Settings & _settings) noexcept
    :   settings (_settings),
        next (nullptr),
        mark (false),
        open (false),
        failure (ERROR_SUCCESS) {

    // Chain this setting to the end of the (forward) list
    //  - simple: point both last's next and then 'last' itself to 'this'
    //  - if 'this' is first there (settings.first is NULL), just set both

    if (settings.stores.first) {
        settings.stores.last->next = this;
        settings.stores.last = this;
    } else {
        settings.stores.first = this;
        settings.stores.last = this;
    };
    return;
};

Emphasize::Settings::Store::~Store () noexcept {

    // Unlinking this setting from Stores list
    //  - if this is first, point first to our next and we are almost done

    if (this->settings.stores.first == this) {
        this->settings.stores.first = this->next;

        // If this is also last, just reset them both
        //  - this->next is certainly NULL at this point, but using this->next
        //    instead NULL (nullptr) saves 3 bytes, since it already is in
        //    register (%ecx usually)

        if (this->settings.stores.last == this)
            this->settings.stores.last = this->next;

    } else {

        // Not first
        //  - find element that preceedes 'this' in the list

        auto p = this->settings.stores.first;
        while (p->next != this) {
            p = p->next;
        };

        // Release 'this' from list

        p->next = p->next->next;

        // Update settings' last
        //  - if the last (not pointing to next) was actually removed

        if (!p->next) {
            this->settings.stores.last = p;
        };
    };
    return;
};

// Default implementations follows

bool Emphasize::Settings::Store::Read () {
    this->failure = ERROR_CALL_NOT_IMPLEMENTED;
    return false;
};
bool Emphasize::Settings::Store::Write () {
    this->failure = ERROR_CALL_NOT_IMPLEMENTED;
    return false;
};
void Emphasize::Settings::Store::Close () {};

// Query
//  - default implementation never finds the requested setting
//  - the conversion is requested by Convert member temmplate, see _Store.tcc

Emphasize::Settings::Store::Type
Emphasize::Settings::Store::Query (const wchar_t *, const wchar_t *, std::size_t *) {
    return Store::Missing;
};

bool Emphasize::Settings::Store::Get (const wchar_t *, const wchar_t *,
                                      void *, std::size_t) {
    return false;
};
bool Emphasize::Settings::Store::Erase (const wchar_t *, const wchar_t *) {
    return false;
};

// Set
//  - data up-converting default implementations follow

bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name,
                                      const char * string) {
    bool result = false;
    
    int n = MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, string, -1, NULL, 0);
    if (n) {
        auto h = GetProcessHeap ();
        auto wsz = static_cast <wchar_t *> (HeapAlloc (h, 0, n * sizeof (wchar_t)));

        if (wsz) {
            if (MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                                     string, -1, wsz, n)) {
                result = this->Set (path, name, wsz);
            };
            HeapFree (h, 0, wsz);
        };
    };
    
    return result;
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name,
                                      const wchar_t * string) {
    return this->Set (path, name,
                      static_cast <const void *> (string),
                      sizeof (wchar_t) * (std::wcslen (string) + 1u));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name,
                                      const void * data, std::size_t size) {
    auto h = GetProcessHeap ();
    auto cb = (size + 1) * 2 * sizeof (wchar_t);
    auto wsz = static_cast <wchar_t *> (HeapAlloc (h, 0, cb));
    
    if (wsz) {
        for (std::size_t i = 0u; i != size; ++i) {
            _snwprintf (&wsz [i * 2u], 3, L"%02X",
                        static_cast <const unsigned char *> (data) [i]);
        };

        bool result = this->Set (path, name, wsz);
        
        HeapFree (h, 0, wsz);
        return result;
    } else
        return false;
};

bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, bool value) {
    return this->Set (path, name, static_cast <int> (value));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, int value) {
    return this->Set (path, name, static_cast <long> (value));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, long value) {
    return this->Set (path, name, static_cast <long long> (value));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, long long value) {
    wchar_t text [24];
    _snwprintf (text, sizeof text / sizeof text [0], L"%I64d", value);
    return this->Set (path, name, text);
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, unsigned int value) {
    return this->Set (path, name, static_cast <unsigned long> (value));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, unsigned long value) {
    return this->Set (path, name, static_cast <unsigned long long> (value));
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, unsigned long long value) {
    wchar_t text [24];
    _snwprintf (text, sizeof text / sizeof text [0], L"%I64u", value);
    return this->Set (path, name, text);
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, float value) {
    wchar_t text [16];
    _snwprintf (text, sizeof text / sizeof text [0], L"%.8g", value);
    return this->Set (path, name, text);
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, double value) {
    wchar_t text [24];
    _snwprintf (text, sizeof text / sizeof text [0], L"%.16g", value);
    return this->Set (path, name, text);
};
bool Emphasize::Settings::Store::Set (const wchar_t * path, const wchar_t * name, long double value) {
    return this->Set (path, name, static_cast <double> (value));
};
