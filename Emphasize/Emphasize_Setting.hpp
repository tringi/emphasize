#ifndef EMPHASIZE_SETTING_HPP
#define EMPHASIZE_SETTING_HPP

/* Emphasize Setting 
// Emphasize_Setting.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.06.2012 - initial version
*/

#include "Emphasize_Settings_Base.hpp"
#include <initializer_list>

namespace Emphasize {

    // Setting
    //  - connects a variable to Settings machinery
    //  - usage: Setting<T> option (..., L"name", defaultvalue);
    //           Setting<T*> option (..., L"name", &actual_referenced_storage);
    //           Setting<T&> option (..., L"name", actual_referenced_storage);
    //            - where T is bool, char, int, long, long long
    //              all signed or unsigned where appropriate
    //            - T can be std::string or std::wstring
    //            - other types are simply considered binary blobs
    //
    //           Setting<T[N]> options_arr (..., L"name", {defval,defval,...});
    //            - array of options, up to N elements
    //
    //           Setting<char*> option (..., L"name", &storage, length);
    //           Setting<wchar_t*> option (..., L"name", &storage, length);
    //            - string behavior over provided storage
    //
    //           Setting<char[N]> option (..., L"name", "optional default");
    //           Setting<wchar_t[N]> option (..., L"name", L"optional default");
    //            - string behavior with space for N characters allocated inside
    //
    //
    
    template <typename T>
    class Setting
        : private Settings::Base {
        
        private:
            mutable T value;

        public:
            
            // Setting constructor
            //  - initialized into Settings with name and optional default value
            //  - second variant adds path (ini key, registry subkey, etc.)
            
            Setting (Settings &, const wchar_t * name, const T & = T ());
            Setting (Settings &, const wchar_t * path, const wchar_t * name, const T & = T ());
            
            // operator =
            // operator T&
            //  - exposing the value for direct access, and change if neccessary
            
            Setting & operator = (const T &);
            operator const T & () const { return this->value; };
            operator       T & ()       { return this->value; };
            
            // operator ->
            //  - for simple access if T is not intrinsic type
            
            const T * operator -> () const { return &this->value; };
                  T * operator -> ()       { return &this->value; };
            
            // load/save
            //  - instructs parent Settings to load or save this variable
            //  - although 'load' being 'const' may seem counter-intuitive
            //    this allows const-qualified Setting to be re-loaded
            
            virtual bool load () const;
            virtual bool save ();

            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            
            virtual bool erase () { return this->Base::erase (); };
    };
};

#include "Emphasize_Setting.tcc"
#endif
