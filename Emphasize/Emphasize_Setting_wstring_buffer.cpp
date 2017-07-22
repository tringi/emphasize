#include "Emphasize_Setting.hpp"
#include "Emphasize_Setting_wstring_buffer.hpp"

/* Emphasize Setting - wide string buffer partial specialization
// Emphasize_Setting_wstring_buffer.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      20.08.2012 - initial version
*/

#include <windows.h>
#include "Emphasize_Settings_Main.hpp"
#include "../Windows/Windows_Heap_DuplicateString.hpp"

Emphasize::Setting <wchar_t[]> ::Setting (Emphasize::Settings & settings,
                                          const wchar_t * name)
    :   Emphasize::Settings::Base (settings, name),
        buffer (Windows::Heap::DuplicateString (L"")) {};

Emphasize::Setting <wchar_t[]> ::Setting (Emphasize::Settings & settings,
                                          const wchar_t * path, const wchar_t * name,
                                          const wchar_t * default_value)
    :   Emphasize::Settings::Base (settings, name, path),
        buffer (Windows::Heap::DuplicateString (default_value)) {};

Emphasize::Setting <wchar_t[]> ::~Setting () {
    if (this->buffer) {
        HeapFree (GetProcessHeap (), 0, this->buffer);
    };
    return;
};

bool Emphasize::Setting <wchar_t[]> ::load () const {
    std::size_t size = this->buffer
                     ? HeapSize (GetProcessHeap (), 0, this->buffer)
                     : 0u;
    if (size != -1)
        return this->settings.load (this, &this->buffer, size);
    else
        return false;
};
bool Emphasize::Setting <wchar_t[]> ::save () {
    return this->settings.save (this, this->buffer ? this->buffer : L"", 0);
};

bool Emphasize::Setting <wchar_t[]> ::resize (unsigned int n) {
    if (auto * re = (wchar_t *) HeapReAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY,
                                             this->buffer, sizeof (wchar_t) * (n + 1u))) {
        this->buffer = re;
        return true;
    } else
        return false;
};

#include "Emphasize_Settings.hpp"
