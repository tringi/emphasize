#include "Emphasize_Setting_wstring.hpp"

/* Emphasize Setting - wide string pointer specialization
// Emphasize_Setting_wstring.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

#include "Emphasize_Settings.hpp"

Emphasize::Setting <wchar_t *> ::Setting (Emphasize::Settings & settings,
                                          const wchar_t * name,
                                          wchar_t * _buffer, unsigned int _size)
    :   Emphasize::Settings::Base (settings, name),
        buffer (_buffer),
        size (_size) {
    
    this->buffer [this->size - 1] = L'\0';
    return;
};
Emphasize::Setting <wchar_t *> ::Setting (Emphasize::Settings & settings,
                                          const wchar_t * path, const wchar_t * name,
                                          wchar_t * _buffer, unsigned int _size)
    :   Emphasize::Settings::Base (settings, name, path),
        buffer (_buffer),
        size (_size) {
    
    this->buffer [this->size - 1] = L'\0';
    return;
};

bool Emphasize::Setting <wchar_t *> ::load () const {
    return this->settings.load (this, this->buffer, this->size - 1);
};
bool Emphasize::Setting <wchar_t *> ::save () {
    return this->settings.save (this, this->buffer, this->size);
};

