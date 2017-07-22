#ifndef EMPHASIZE_SETTING_REFERENCE_TCC
#define EMPHASIZE_SETTING_REFERENCE_TCC

/* Emphasize Setting - partial specialization for references
// Emphasize_Setting_reference.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

template <typename T>
Emphasize::Setting <T &> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * name,
                                    T & _reference)
    :   Emphasize::Settings::Base (settings, name),
        reference (_reference) {};

template <typename T>
Emphasize::Setting <T &> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * path,
                                    const wchar_t * name,
                                    T & _reference)
    :   Emphasize::Settings::Base (settings, name, path),
        reference (_reference) {};

template <typename T>
bool Emphasize::Setting <T &> ::load () const {
    return this->settings.load (this, &this->reference, sizeof (T));
};

template <typename T>
bool Emphasize::Setting <T &> ::save () {
    return this->settings.save (this, &this->reference, sizeof (T));
};

#endif
