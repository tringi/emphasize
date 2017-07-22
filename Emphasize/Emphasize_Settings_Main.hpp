#ifndef EMPHASIZE_SETTINGS_MAIN_HPP
#define EMPHASIZE_SETTINGS_MAIN_HPP

/* Emphasize Settings class main declaration file 
// Emphasize_Settings_Main.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      20.06.2012 - initial version
*/

#include <string>

namespace Emphasize {
    
    class Settings {
        public:
            Settings () noexcept;
            
        public:
            class Base;
            class Store;
            class Ini; // : Store;
            class Registry; // : Store;

        public:
            
            // load/save without parameters
            //  - loads or saves all currently referenced Settings
            //  - load returns false if at least one setting failed to load
            //    and default value had to be used
            
            bool load () const;
            bool save ();
            
            // erase
            //  - removes all settings
            
            bool erase ();
            
            // initialize_defaults
            //  - calls 'setting.after_load ()' on all settings

            void initialize_defaults ();

        private:
            struct {
                Base * first;
                Base * last;
                // TODO: add "hint" of second-to-last for fast removal on array load
            } settings;
            struct {
                Store * first;
                Store * last;
                mutable Store * active;
            } stores;
        
        private:
            void cleanup () const;
            void cleanup (Store *) const;

            bool load_step (const Base *) const;
            bool save_step (      Base *);
            bool erase_step(      Base *);
            
        public:
            
            // active
            //  - returns non-zero if any operation is currently active,
            //    that is, there is an open store to save/load/erase upon
            
            bool active () const { return this->stores.active != nullptr; };
            
            // load
            //  - 

            bool load (const Base *); // const?
            
            template <typename T>
            bool load (const Base *, T *, std::size_t);
            
            template <typename C>
            bool load (const Base *, std::basic_string <C> *, std::size_t);
            
            // save
            //  - and string specializations

            bool save (Base *);
        
            template <typename T>
            bool save (Base *, T *, std::size_t);

            bool save (Base *, char *, std::size_t);
            bool save (Base *, wchar_t *, std::size_t);
            bool save (Base *, const char *, std::size_t);
            bool save (Base *, const wchar_t *, std::size_t);

            template <typename C>
            bool save (Base *, std::basic_string <C> *, std::size_t);
            
            // erase
            //  - removes nearest removable setting

            bool erase (Base *);
            
        private:
            template <typename T, int>
            struct LoadDispatch;

            template <typename T, bool>
            struct SaveDispatch;

        protected:

            // HexOffset
            //  - helper function for skipping common hexadecimal prefixes
            //  - returns offset at which strtoul(unsigned int, 16) can be correctly used
            //    or 0 if the string does not start with known prefix
            
            static int HexOffset (const wchar_t *);

        private:

            // LoadStr
            
            bool LoadStr (const wchar_t *, const wchar_t *, bool *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *,   signed char *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, unsigned char *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *,   signed short *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, unsigned short *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *,   signed int *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, unsigned int *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *,   signed long *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, unsigned long *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *,   signed long long *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, unsigned long long *, std::size_t, std::size_t);

            bool LoadStr (const wchar_t *, const wchar_t *, float *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, double *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, long double *, std::size_t, std::size_t);

            bool LoadStr (const wchar_t *, const wchar_t *, void *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, char *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, char **, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, wchar_t *, std::size_t, std::size_t);
            bool LoadStr (const wchar_t *, const wchar_t *, wchar_t **, std::size_t, std::size_t);

            // LoadBin

            bool LoadBin (const wchar_t *, const wchar_t *, void *, std::size_t, std::size_t);
            bool LoadBin (const wchar_t *, const wchar_t *, char *, std::size_t, std::size_t);
            bool LoadBin (const wchar_t *, const wchar_t *, wchar_t *, std::size_t, std::size_t);
            
            // LoadInt
            
            bool LoadInt (const wchar_t *, const wchar_t *, void *, std::size_t, std::size_t);
    };
};

#endif
