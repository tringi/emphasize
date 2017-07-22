#ifndef EMPHASIZE_SETTINGS_BASE_HPP
#define EMPHASIZE_SETTINGS_BASE_HPP

/* Emphasize Settings Base class (for Emphasize::Setting template)
// Emphasize_Settings_Base.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.06.2012 - initial version
*/

#include <windows.h>
#include "Emphasize_Settings_Main.hpp"

namespace Emphasize {
    
    // Settings::Base
    //  - base common implementation for Setting template
    //  - provides backward reference to Settings and takes part in particular
    //    forward linked list
    
    class Settings::Base {
        friend class Settings;
        
        protected:
            Settings &      settings;
            Base *          next;
            const wchar_t * name;
            const wchar_t * path;
            
        protected:
            
            // Constructor
            //  - settings reference and option name
            //  - NOTE: only pointers to names are copied, not 
            
            Base (Settings &, const wchar_t *, const wchar_t * = NULL) noexcept;
            ~Base () noexcept;

            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            //  - default implementation is available
            
            virtual bool erase ();
            
            // rename
            
            void rename (const wchar_t * _name) { this->name = _name; };
            void redirect (const wchar_t * _path) { this->path = _path; };
            
        private:
            
            // load/save
            //  - overriden by Setting template

            virtual bool load () const = 0;
            virtual bool save () = 0;
            
            // after_load/before_save
            //  - callbacks

            virtual void after_load () const {};
            virtual void before_save () {};

            // copy-construction/assignment disabled
            
            Base (const Base &) = delete;
            Base & operator = (const Base &) = delete;
    };
};

#endif

