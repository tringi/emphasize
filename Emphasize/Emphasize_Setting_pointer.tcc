#ifndef EMPHASIZE_SETTING_POINTER_TCC
#define EMPHASIZE_SETTING_POINTER_TCC

/* Emphasize Setting - partial specialization for pointers
// Emphasize_Setting_pointer.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

template <typename T>
Emphasize::Setting <T *> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * name,
                                    T * _pointer)
    :   Emphasize::Settings::Base (settings, name),
        pointer (_pointer) {};
        
template <typename T>
Emphasize::Setting <T *> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * path,
                                    const wchar_t * name,
                                    T * _pointer)
    :   Emphasize::Settings::Base (settings, name, path),
        pointer (_pointer) {};

template <typename T>
bool Emphasize::Setting <T *> ::load () const {
    return this->settings.load (this, this->pointer, sizeof (T));
};

template <typename T>
bool Emphasize::Setting <T *> ::save () {
    return this->settings.save (this, this->pointer, sizeof (T));
};

#endif
