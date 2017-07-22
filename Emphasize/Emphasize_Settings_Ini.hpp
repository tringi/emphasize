#ifndef EMPHASIZE_SETTINGS_INI_HPP
#define EMPHASIZE_SETTINGS_INI_HPP

/* Emphasize Settings INI
// Emphasize_Settings_Ini.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      17.05.2012 - initial version
*/

#include "Emphasize_Settings.hpp"
#include "Emphasize_Settings_Store.hpp"

namespace Emphasize {
    
    // Settings::Ini
    //  - Settings Store representing a single INI file
    
    class Settings::Ini
        : public Settings::Store {
            
        public:
            enum Mode {
                WriteExisting, // only existing settings are read and/or updated
                WriteAlways,   // file is created and all settings are written
                               //  - an absolute path MUST be provided
            };

        private:
            const Mode  mode;
            const bool  absp; // path is absolute, not searching
            
            wchar_t *   orig;
            wchar_t *   path;
            wchar_t *   main;
            
        public:
            
            // Ini
            //  - constructs INI file accessing store
            //  - parameters: name - ini and main section name, e.g. "Emphasize"
            //                        - file is searched in all common locations
            //                        - use NULL for default name (as .exe)
            //                     - or absolute file path C:\... or \\.\...
            //                        - last component without .ini is then
            //                          used as a main section name
            //                mode - WriteExisting - updates values that are
            //                        already in INI file and fails for others
            //                     - WriteAlways - all writes go to INI file
            //                        - name MUST be absolute path!
            
            explicit Ini (Settings & settings,
                          const wchar_t * name = NULL, Mode = WriteExisting) noexcept;
            virtual ~Ini () noexcept;
        
        public:
            
            // Find
            //  - searches for a INI file named 'name' (without .ini extension)
            //  - returns absolute path of the file by either:
            //     - writting it to provided buffer (wchar_t/unsigned int)
            //     - or allocating (HeapAlloc) the string and returning pointer
            
            static wchar_t * Find (const wchar_t * name,
                                   wchar_t * = NULL, unsigned int = 0u);

            // FindName
            //  - returns allocated copy of a name without path and extension
            //  - if path is NULL, the executable name is returned (without .exe)

            static wchar_t * FindName (const wchar_t * path);

            // ListXxx
            //  - section parameter defaults (when NULL) to main section
            //  - returns process-heap-allocated buffer of zero terminated strings
            //  - release using HeapFree (GetProcessHeap (), 0, ptr);

            wchar_t * ListSections (); // [abc]\0[def]\0...
            wchar_t * ListKeys (const wchar_t * section = NULL); // 
            wchar_t * ListKeyValues (const wchar_t * section = NULL);

        private:
            
            // Store interface implementation follows
            
            virtual bool Read () override;
            virtual bool Write () override;
            virtual void Close () override;
            
            virtual bool Set (const wchar_t *, const wchar_t *, const wchar_t *) override;
            virtual bool Get (const wchar_t *, const wchar_t *, void *, std::size_t) override;
            virtual bool Erase (const wchar_t *, const wchar_t *) override;
            virtual Type Query (const wchar_t *, const wchar_t *, std::size_t * size) override {
                *size = -1;
                return Store::StringType;
            };

        private:
            bool IsPrivateProfileString (LPCTSTR, LPCTSTR);
    };
};

#endif

