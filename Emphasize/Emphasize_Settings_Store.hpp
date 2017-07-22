#ifndef EMPHASIZE_SETTINGS_STORE_HPP
#define EMPHASIZE_SETTINGS_STORE_HPP

/* Emphasize Settings_Store 
// Emphasize_Settings_Store.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      16.06.2012 - initial version
*/

#include "Emphasize_Settings_Main.hpp"

namespace Emphasize {
    
    // Settings::Store
    //  - base common implementation for settings stores
    //  - provides backward reference to Settings and takes part in particular
    //    forward linked list

    class Settings::Store {
        friend class Settings;
        
        private:
            Settings &  settings;
            Store *     next;
            bool        mark;
        protected:
            bool        open;
            unsigned    failure;
            
        protected:
            explicit Store (Settings &) noexcept;
            virtual ~Store () noexcept;

        private:
            
            // Interface for Stores to override
            //  - functions return a meaningful error code or 0 on success
            
            // Open/Close
            //  - overrides need not update .open boolean flag
            
            virtual bool Read ();   // open for read-only access
            virtual bool Write ();  // open for write access
            virtual void Close ();  // close, return value ignored
            
        protected:
            
            // Query/Type
            //  - 
            
            enum Type {
                Missing,
                BinaryType,
                StringType,
                IntegerType,
            };
            
            // Query
            //  - set 'size' to bytes (sizeof) required by full data element
            //     - for strings that includes NUL-terminator byte(s)
            //  - set 'size' to -1u to request enough space to convert to target
            //    type that is 2*(n+1) for binary data, 24 for integers, ...

            virtual Type Query (const wchar_t * path,
                                const wchar_t * name, std::size_t * size);
            
            // Erase
            //  - requests removal of value (path can be NULL)
            //  - return true on success, false on error
            
            virtual bool Erase (const wchar_t * path, const wchar_t * name);
            
            // Get
            //  - requests data to be written into provided buffer/size
            //  - the data must be in format indicated through .Query
            //  - path can be NULL
            //  - return true on success, false on error
            
            virtual bool Get (const wchar_t * path, const wchar_t * name, void *, std::size_t);
            
            // Set
            //  - 
            //  - default implementations convert as follows:
            //     - bool -> int
            //     - int -> long
            //     - long -> long long (prefer overriding long over int)
            //     - long long -> wchar_t string
            //     - double -> wchar_t string
            //     - long double -> double (note the backward conversion)
            //     - char string -> wchar_t string (CP_ACP -> UTF-16)
            //     - !!! wchar_t string -> raw data
            //     - !!! raw data -> wchar_t string
            
            virtual bool Set (const wchar_t * path, const wchar_t * name, bool value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, int value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, long value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, long long value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, unsigned int value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, unsigned long value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, unsigned long long value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, float value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, double value);
            virtual bool Set (const wchar_t * path, const wchar_t * name, long double value);
            
            virtual bool Set (const wchar_t * path, const wchar_t * name, const char *);
            virtual bool Set (const wchar_t * path, const wchar_t * name, const wchar_t *);
            virtual bool Set (const wchar_t * path, const wchar_t * name, const void *, std::size_t);

        private:

            // SetByValue
            //  - internal Store base type behavior policy, see _Store.tcc
            
            template <typename T>
            struct SetByValue { static const bool enabled = false; };
    };

};

#include "Emphasize_Settings_Store.tcc"
#endif
