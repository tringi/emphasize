#ifndef EMPHASIZE_SETTING_WSTRING_BUFFER_TCC
#define EMPHASIZE_SETTING_WSTRING_BUFFER_TCC

/* Emphasize Setting - wide string buffer partial specialization
// Emphasize_Setting_wstring_buffer.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

template <unsigned int N>
Emphasize::Setting <wchar_t[N]> ::Setting (Emphasize::Settings & settings,
                                           const wchar_t * name)
    :   Emphasize::Settings::Base (settings, name) {
    
    this->buffer[0] = L'\0';
    this->buffer[N] = L'\0';
    return;
};
template <unsigned int N>
Emphasize::Setting <wchar_t[N]> ::Setting (Emphasize::Settings & settings,
                                           const wchar_t * path, const wchar_t * name,
                                           const wchar_t * default_value)
    :   Emphasize::Settings::Base (settings, name, path) {
    
    _snwprintf (this->buffer, N+1u, L"%s", default_value);
    this->buffer[N] = L'\0';
    return;
};

template <unsigned int N>
bool Emphasize::Setting <wchar_t[N]> ::load () const {
    return this->settings.load (this, this->buffer, N + 1u);
};
template <unsigned int N>
bool Emphasize::Setting <wchar_t[N]> ::save () {
    return this->settings.save (this, this->buffer, N + 1u);
};

#endif

