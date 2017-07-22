#ifndef EMPHASIZE_SETTINGS_REGISTRY_HPP
#define EMPHASIZE_SETTINGS_REGISTRY_HPP

/* Emphasize Settings_Registry 
// Emphasize_Settings_Registry.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      17.05.2012 - initial version
*/

#include <windows.h>

#include "Emphasize_Settings.hpp"
#include "Emphasize_Settings_Store.hpp"

namespace Emphasize {

    // Settings::Registry
    //  - Settings Store representing a key in registry

    class Settings::Registry
        : public Settings::Store {
        
        private:
            const HKEY      hkey;
            const wchar_t * path;
            HKEY            handle;

        public:

            // Registry
            //  - constructs registry accessing store
            //  - parameters: parent - HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
            //                       - or other, used directly
            //                path - key path
            //                     - NOTE that only pointer is copied here!!!
            
            explicit Registry (Settings & settings,
                               HKEY parent, const wchar_t * path);

        private:
            virtual bool Read () override;   // open for read-only access
            virtual bool Write () override;  // open for write access
            virtual void Close () override;

            virtual bool Erase (const wchar_t *, const wchar_t *) override;
            virtual Type Query (const wchar_t *, const wchar_t *, std::size_t *) override;
            virtual bool Get (const wchar_t *, const wchar_t *, void *, std::size_t) override;

            virtual bool Set (const wchar_t *, const wchar_t * name, long value) override;
            virtual bool Set (const wchar_t *, const wchar_t * name, long long value) override;
            virtual bool Set (const wchar_t *, const wchar_t * name, unsigned long value) override;
            virtual bool Set (const wchar_t *, const wchar_t * name, unsigned long long value) override;
            virtual bool Set (const wchar_t *, const wchar_t * name, const wchar_t *) override;
            virtual bool Set (const wchar_t *, const wchar_t * name, const void *, std::size_t) override;
        
        private:
            bool Write (const wchar_t *, const wchar_t *, DWORD, const void *, std::size_t);
    };
};
#endif

