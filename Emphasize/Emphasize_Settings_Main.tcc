#ifndef EMPHASIZE_SETTINGS_MAIN_TCC
#define EMPHASIZE_SETTINGS_MAIN_TCC

/* Emphasize Settings 
// Emphasize_Settings.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      19.05.2012 - initial version
*/

#include <cstdio>

template <typename T>
bool Emphasize::Settings::load (const Emphasize::Settings::Base * setting,
                                T * value, std::size_t length) {
    if (this->stores.active) {
        
        std::size_t size = 0;
        switch (this->stores.active->Query (setting->path, setting->name, &size)) {

            case Store::BinaryType:
                return LoadBin (setting->path, setting->name, value, length, size);
            case Store::StringType:
                return LoadStr (setting->path, setting->name, value, length, size);
            case Store::IntegerType:
                return LoadInt (setting->path, setting->name, value, length, size);
            
            default:
                return false;
        };
        
    } else
        return this->load (setting);
};

template <typename C>
bool Emphasize::Settings::load (const Base * setting,
                                std::basic_string <C> * string, std::size_t) {
    if (this->stores.active) {
    
        std::size_t size = 0;
        switch (this->stores.active->Query (setting->path, setting->name, &size)) {
            
//            case Store::BinaryType:
                // well, why not
//                break;
            
            case Store::StringType:
                if (size == -1)
                    size = 65535;
                
                if (string->length () < size) {
                    string->resize (size);
                };
                
                if (LoadStr (setting->path, setting->name,
                             &((*string)[0]),
                             size, sizeof (wchar_t) * (size + 1))) {
                    
                    const auto * p = string->c_str ();
                    const auto * b = p;
                    while (*p)
                        ++p;
                    
                    string->resize (p - b);
                    string->shrink_to_fit ();
                    return true;
                } else {
                    return false;
                };
    
//            case Store::IntegerType:
//                string->resize (4u * size + 1u);
                // snprintf
//                break;
            
            default:
                return false;
        };
    } else
        return this->load (setting);
};

// SaveDispatch
//  - based on Settings::Store::SetByValue <T> ::enabled, see .save function,
//    calls one of the Store::Set overloads:
//     - when enabled == true: store->Set (path, name, pointer, length) 
//     - when enabled == false: store->Set (path, name, *pointer)
//       and normal integral/floating promotions applies

namespace Emphasize {
    template <typename T, bool>
    struct Settings::SaveDispatch {};
    
    template <typename T>
    struct Settings::SaveDispatch <T, true> {
        static bool Set (Store * store,
                         const wchar_t * path, const wchar_t * name,
                         T * pointer, std::size_t) {
            return store->Set (path, name, *pointer);
        };
    };
    template <typename T>
    struct Settings::SaveDispatch <T, false> {
        static bool Set (Store * store,
                         const wchar_t * path, const wchar_t * name,
                         T * pointer, std::size_t length) {
            return store->Set (path, name, pointer, length);
        };
    };
};

template <typename T>
bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                T * value, std::size_t length) {
    if (this->stores.active) {
        return SaveDispatch <T, Settings::Store::SetByValue <T> ::enabled>
                     :: Set (this->stores.active,
                             setting->path, setting->name, value, length);
    } else
        return this->save (setting);
};

template <typename C>
bool Emphasize::Settings::save (Emphasize::Settings::Base * setting,
                                std::basic_string <C> * string,
                                std::size_t length) {
    return this->save (setting, string->c_str (), length);
};

#endif
