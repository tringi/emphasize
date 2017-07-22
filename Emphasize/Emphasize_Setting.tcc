#ifndef EMPHASIZE_SETTING_TCC
#define EMPHASIZE_SETTING_TCC

/* Emphasize Setting 
// Emphasize_Setting.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.06.2012 - initial version
*/

#include <cstring>
#include <cwchar>
#include <new>

// General implementation

template <typename T>
Emphasize::Setting <T> ::Setting (Emphasize::Settings & settings,
                                  const wchar_t * name,
                                  const T & default_value)
    :   Emphasize::Settings::Base (settings, name),
        value (default_value) {};

template <typename T>
Emphasize::Setting <T> ::Setting (Emphasize::Settings & settings,
                                  const wchar_t * path,
                                  const wchar_t * name,
                                  const T & default_value)
    :   Emphasize::Settings::Base (settings, name, path),
        value (default_value) {};

template <typename T>
Emphasize::Setting <T> & Emphasize::Setting <T> ::operator = (const T & _value) {
    this->value = _value;
    return *this;
};
template <typename T>
bool Emphasize::Setting <T> ::load () const {
    return this->settings.load (this, &this->value, sizeof (T));
};
template <typename T>
bool Emphasize::Setting <T> ::save () {
    return this->settings.save (this, &this->value, sizeof (T));
};

#endif
