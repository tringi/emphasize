#ifndef EMPHASIZE_SETTING_REFERENCE_HPP
#define EMPHASIZE_SETTING_REFERENCE_HPP

/* Emphasize Setting - partial specialization for references
// Emphasize_Setting_reference.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

namespace Emphasize {
    
    // Setting <T&>
    //  - connects a variable to Settings machinery through its reference
    //  - usage: Setting<T&> option (settings, L"name", variable);
    //           Setting<T&> option (settings, L"path", L"name", variable);
    //            - where T is bool, char, int, long, long long
    //              all signed or unsigned where appropriate
    //            - other types are simply considered binary blobs
    
    template <typename T>
    class Setting <T &>
        : private Settings::Base {
        
        private:
            T & reference;

        public:
            
            // Setting constructor
            //  - initialized into Settings with name
            //    and reference to actual storage
            
            Setting (Settings &, const wchar_t *, T &);
            Setting (Settings &, const wchar_t *, const wchar_t *, T &);
            
            // operator T&
            //  - exposing the value for direct access, and change if neccessary
            
            operator const T & () const { return this->reference; };
            operator       T & ()       { return this->reference; };
            
            // operator ->
            //  - for simple access if T is not intrinsic type
            
            const T * operator -> () const { return &this->reference; };
                  T * operator -> ()       { return &this->reference; };
            
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

#include "Emphasize_Setting_reference.tcc"
#endif

