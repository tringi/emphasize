#ifndef EMPHASIZE_SETTING_WSTRING_BUFFER_HPP
#define EMPHASIZE_SETTING_WSTRING_BUFFER_HPP

/* Emphasize Setting - wide string buffer partial specialization
// Emphasize_Setting_wstring_buffer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

namespace Emphasize {
    
    // Setting <wchar_t[N]>
    //  - 
    //  - to avoid ambiguous parameters, the default value can and must be
    //    provided when the optional path is used
    //  - usage: Setting<wchar_t[N]> option (settings, L"name");
    //           Setting<wchar_t[N]> option (settings, L"path", L"name", L"default");

    template <unsigned int N>
    class Setting <wchar_t [N]>
        : private Settings::Base  {
        
        private:
            mutable wchar_t buffer [N+1u];
        public:
            static const unsigned int size = N+1u;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with name or initialized with
            //    all the path, name and default value
            //  - NULL can be used for path in the second case
            
            Setting (Settings &, const wchar_t *);
            Setting (Settings &, const wchar_t *, const wchar_t *, const wchar_t *);

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

    template <>
    class Setting <wchar_t []>
        : private Settings::Base  {
        
        private:
            mutable wchar_t * buffer;
        
        public:
            
            // Setting constructor
            //  - initialized into Settings with name or initialized with
            //    all the path, name and default value
            
            Setting (Settings &, const wchar_t *);
            Setting (Settings &, const wchar_t *, const wchar_t *, const wchar_t *);
            ~Setting ();

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
            
            // resize
            //  - resizes buffer to (sizeof (wchar_t) * (n + 1u)) bytes
            
            bool resize (unsigned int n);
    };
};

#include "Emphasize_Setting_wstring_buffer.tcc"
#endif

