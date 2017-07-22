#ifndef EMPHASIZE_SETTING_STRING_BUFFER_HPP
#define EMPHASIZE_SETTING_STRING_BUFFER_HPP

/* Emphasize Setting - narrow string buffer partial specialization
// Emphasize_Setting_string_buffer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

namespace Emphasize {
    
    // Setting <char[N]>
    //  - 
    //  - usage: Setting<char[N]> option (settings, L"name", "default");
    //           Setting<char[N]> option (settings, L"path", L"name", "default");

    template <unsigned int N>
    class Setting <char [N]>
        : private Settings::Base  {
        
        private:
            mutable char buffer [N+1u];
        public:
            static const unsigned int size = N+1u;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with (path and) name and
            //    optionally filled with provided default
            
            Setting (Settings &, const wchar_t *, const char * = "");
            Setting (Settings &, const wchar_t *, const wchar_t *, const char * = "");

            // operator wchar_t *
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
    
    template <>
    class Setting <char []>
        : private Settings::Base  {
        
        private:
            mutable char * buffer;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with (path and) name and
            //    optionally filled with provided default
            
            Setting (Settings &, const wchar_t *, const char * = "");
            Setting (Settings &, const wchar_t *, const wchar_t *, const char * = "");
            ~Setting ();

            // operator wchar_t *
            //  - exposing the value for direct access, and change if neccessary
            
            operator const char * () const { return this->buffer; };
//            operator       char * ()       { return this->buffer; };
            
            // load/save
            //  - instructs parent Settings to load or save this variable
            
            virtual bool load () const;
            virtual bool save ();

            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            
            virtual bool erase () { return this->Base::erase (); };
    };
};

#include "Emphasize_Setting_string_buffer.tcc"
#endif

