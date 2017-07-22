#include "Emphasize_Setting_string.hpp"

/* Emphasize Setting - narrow string pointer specialization
// Emphasize_Setting_string.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

#include "Emphasize_Settings.hpp"

Emphasize::Setting <char *> ::Setting (Emphasize::Settings & settings,
                                       const wchar_t * name,
                                       char * _buffer, unsigned int _size)
    :   Emphasize::Settings::Base (settings, name),
        buffer (_buffer),
        size (_size) {
    
    this->buffer [this->size - 1] = '\0';
    return;
};
Emphasize::Setting <char *> ::Setting (Emphasize::Settings & settings,
                                       const wchar_t * path, const wchar_t * name,
                                       char * _buffer, unsigned int _size)
    :   Emphasize::Settings::Base (settings, name, path),
        buffer (_buffer),
        size (_size) {
    
    this->buffer [this->size - 1] = '\0';
    return;
};

bool Emphasize::Setting <char *> ::load () const {
    return this->settings.load (this, this->buffer, this->size - 1);
};
bool Emphasize::Setting <char *> ::save () {
    return this->settings.save (this, this->buffer, this->size);
};


