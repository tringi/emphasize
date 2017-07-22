#ifndef EMPHASIZE_SETTING_STRING_HPP
#define EMPHASIZE_SETTING_STRING_HPP

/* Emphasize Setting - narrow string pointer specialization
// Emphasize_Setting_string.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

#include "Emphasize_Setting.hpp"

namespace Emphasize {
    
    // Setting <char*>
    //  - connects a string to Settings machinery through its pointer
    //  - usage: Setting<char*> option (settings, L"name", &storage, length);
    //           Setting<char*> option (settings, L"path", L"name", &storage, length);

    template <>
    class Setting <char *>
        : private Settings::Base {
        
        private:
            char * const        buffer;
        public:
            unsigned int const  size;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with name and
            //    a reference to a buffer and length that is to be used
            
            Setting (Settings &, const wchar_t *, char *, unsigned int);
            Setting (Settings &, const wchar_t *, const wchar_t *, char *, unsigned int);
            
            // operator char *
            //  - exposing the value for direct access, and change if neccessary
            
            operator const char * () const { return this->buffer; };
            operator       char * ()       { return this->buffer; };
            
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

