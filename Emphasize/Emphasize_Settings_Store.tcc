#ifndef EMPHASIZE_SETTINGS_STORE_TCC
#define EMPHASIZE_SETTINGS_STORE_TCC

/* Emphasize Settings_Store 
// Emphasize_Settings_Store.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      16.06.2012 - initial version
*/

namespace Emphasize {
    
    // SetByValue/SET_BY_VALUE
    //  - for each type defined below, the Emphasize::Settings will call
    //    simple Store::Set(name,value);
    //  - for types not defined here the Store::Set(name,pointer,size)
    //    is called

#define SET_BY_VALUE(T) \
    template <> struct Settings::Store::SetByValue <T> { static const bool enabled = true; }

    SET_BY_VALUE (bool);
    SET_BY_VALUE (signed char);
    SET_BY_VALUE (unsigned char);
    SET_BY_VALUE (signed short);
    SET_BY_VALUE (unsigned short);
    SET_BY_VALUE (signed int);
    SET_BY_VALUE (unsigned int);
    SET_BY_VALUE (signed long);
    SET_BY_VALUE (unsigned long);
    SET_BY_VALUE (signed long long);
    SET_BY_VALUE (unsigned long long);
    SET_BY_VALUE (float);
    SET_BY_VALUE (double);
    SET_BY_VALUE (long double);
    SET_BY_VALUE (char);
    SET_BY_VALUE (wchar_t);
    SET_BY_VALUE (const char *);
    SET_BY_VALUE (const wchar_t *);
#undef SET_BY_VALUE
};

#endif
