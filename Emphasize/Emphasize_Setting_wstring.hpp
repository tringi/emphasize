#ifndef EMPHASIZE_SETTING_WSTRING_HPP
#define EMPHASIZE_SETTING_WSTRING_HPP

/* Emphasize Setting - wide string pointer specialization
// Emphasize_Setting_wstring.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

#include "Emphasize_Setting.hpp"

namespace Emphasize {
        
    // Setting <wchar_t*>
    //  - connects a string to Settings machinery through its pointer
    //  - usage: Setting<wchar_t*> option (settings, L"name", &storage, length);
    //           Setting<wchar_t*> option (settings, L"path", L"name", &storage, length);

    template <>
    class Setting <wchar_t *>
        : private Settings::Base  {
        
        private:
            wchar_t * const     buffer;
        public:
            unsigned int const  size;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with name and
            //    a reference to a buffer and length that is to be used
            
            Setting (Settings &, const wchar_t *, wchar_t *, unsigned int);
            Setting (Settings &, const wchar_t *, const wchar_t *, wchar_t *, unsigned int);

            // operator wchar_t *
            //  - exposing the value for direct access, and change if neccessary
            
            operator const wchar_t * () const { return this->buffer; };
            operator       wchar_t * ()       { return this->buffer; };
            
            // load/save
            //  - instructs parent Settings to load or save this variable
            
            virtual bool load () const;
            virtual bool save ();

            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            
            virtual bool erase () { return this->Base::erase (); };
    };
};

#endif

