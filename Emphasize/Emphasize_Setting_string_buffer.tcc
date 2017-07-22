#ifndef EMPHASIZE_SETTING_STRING_BUFFER_TCC
#define EMPHASIZE_SETTING_STRING_BUFFER_TCC

/* Emphasize Setting - narrow string buffer partial specialization
// Emphasize_Setting_string_buffer.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

template <unsigned int N>
Emphasize::Setting <char[N]> ::Setting (Emphasize::Settings & settings,
                                        const wchar_t * name,
                                        const char * default_value)
    :   Emphasize::Settings::Base (settings, name) {
    
    _snprintf (this->buffer, N, "%s", default_value);
    this->buffer[N] = '\0';
    return;
};
template <unsigned int N>
Emphasize::Setting <char[N]> ::Setting (Emphasize::Settings & settings,
                                        const wchar_t * path, const wchar_t * name,
                                        const char * default_value)
    :   Emphasize::Settings::Base (settings, name, path) {
    
    _snprintf (this->buffer, N+1u, "%s", default_value);
    this->buffer[N] = '\0';
    return;
};

template <unsigned int N>
bool Emphasize::Setting <char[N]> ::load () const {
    return this->settings.load (this, this->buffer, N + 1u);
};
template <unsigned int N>
bool Emphasize::Setting <char[N]> ::save () {
    return this->settings.save (this, this->buffer, N + 1u);
};

#endif

