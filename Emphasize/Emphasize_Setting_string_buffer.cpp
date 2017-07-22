#include "Emphasize_Setting.hpp"
#include "Emphasize_Setting_string_buffer.hpp"

/* Emphasize Setting - narrow string buffer partial specialization
// Emphasize_Setting_string_buffer.cpp
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

Emphasize::Setting <char[]> ::Setting (Emphasize::Settings & settings,
                                       const wchar_t * name,
                                       const char * default_value)
    :   Emphasize::Settings::Base (settings, name),
        buffer (Windows::Heap::DuplicateString (default_value)) {};

Emphasize::Setting <char[]> ::Setting (Emphasize::Settings & settings,
                                       const wchar_t * path, const wchar_t * name,
                                       const char * default_value)
    :   Emphasize::Settings::Base (settings, name, path),
        buffer (Windows::Heap::DuplicateString (default_value)) {};

Emphasize::Setting <char[]> ::~Setting () {
    HeapFree (GetProcessHeap (), 0, this->buffer);
    return;
};

bool Emphasize::Setting <char[]> ::load () const {
    auto size = HeapSize (GetProcessHeap (), 0, this->buffer);
    if (size != -1)
        return this->settings.load (this, &this->buffer, size);
    else
        return false;
};
bool Emphasize::Setting <char[]> ::save () {
    return this->settings.save (this, this->buffer, 0);
};

#include "Emphasize_Settings.hpp"
