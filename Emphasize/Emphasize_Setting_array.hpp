#ifndef EMPHASIZE_SETTING_ARRAY_HPP
#define EMPHASIZE_SETTING_ARRAY_HPP

/* Emphasize Setting - partial specializations for arrays
// Emphasize_Setting_array.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

#include <array>
#include <deque>
#include <vector>
#include <algorithm>

namespace Emphasize {
    
    // Setting <T[]>
    // Setting <T[N]>
    //  - array of settings, dynamic or fixed to N elements
    //  - usage: Setting<T[N]> option (settings, L"name", { });
    //           Setting<T[N]> option (settings, L"path", L"name", { });
    //            - where T is bool, char, int, long, long long
    //              all signed or unsigned where appropriate
    //               - other types are simply considered binary blobs
    //            - N is size of the array (unsigned integer)
    //               - when ommited, array size will be dynamic and
    //                 limited only by the available memory

    template <typename T, unsigned int N>
    class Setting <T[N]>
        : private Settings::Base {
        
        private:
            mutable std::array <T, N> data;
            
            template <typename F>
            bool for_all_subelements (F) const;
            
        public:
            
            // Setting constructor
            //  - initialized into Settings with name (path) and optional
            //    default values for each array member (excessive are ignored)
            
            Setting (Settings &, const wchar_t *,
                     const std::initializer_list <T> & = {});
            Setting (Settings &, const wchar_t *, const wchar_t *,
                     const std::initializer_list <T> & = {});
            
            // operator []
            //  - exposing the values for direct access
            
            const T & operator [] (unsigned i) const { return this->data [i]; };
                  T & operator [] (unsigned i)       { return this->data [i]; };
            
            // load/save
            //  - instructs parent Settings to load or save this variable
            
            virtual bool load () const;
            virtual bool save ();

            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            
            virtual bool erase ();
            
            // container member functions
            //  - add another from std::array
            
            unsigned int size () const { return N; };
    };
    
    template <typename T>
    class Setting <T[]>
        : private Settings::Base {
        
        private:
            mutable std::vector <T> data; // vector?
            
        public:
            
            // Setting constructor
            //  - initialized into Settings with name (path) and optional
            //    default values (their number indicates initial array size)
            
            Setting (Settings &, const wchar_t *);
            Setting (Settings &, const wchar_t *, std::initializer_list <T>);
            Setting (Settings &, const wchar_t *, const wchar_t *);
            Setting (Settings &, const wchar_t *, const wchar_t *, std::initializer_list <T>);
            
            // operator []
            //  - exposing the values for direct access
            
            const T & operator [] (unsigned i) const { return this->data [i]; };
                  T & operator [] (unsigned i)       { return this->data [i]; };
            
            // load/save
            //  - instructs parent Settings to load or save this variable
            
            virtual bool load () const;
            virtual bool save ();
            
            // erase
            //  - deletes the variable from nearest (as in .save()) storage
            
            virtual bool erase ();
            
            // container member functions
            //  - forwards to the 'data' member
            
            unsigned int size () const { return (unsigned long) this->data.size (); };
            void resize (unsigned int n) const { this->data.resize (n); };
            bool empty () const { return this->data.empty (); };
            
            typename std::vector <T> ::iterator begin () const { return this->data.begin (); };
            typename std::vector <T> ::iterator end () const { return this->data.end (); };
            
            typename std::vector <T> ::iterator erase (unsigned i) const { 
                return this->data.erase (this->data.begin () + i);
            };
            typename std::vector <T> ::iterator erase (const T & v) const {
                return this->erase_found (v);
            };
            typename std::vector <T> ::iterator erase_found (const T & v) const {
                const auto end = this->data.end ();
                const auto found = std::find (this->data.begin (), end, v);
                
                if (found != end)
                    return this->data.erase (found);
                else
                    return end;
            };

            typename std::vector <T> ::iterator find (const T & v) const {
                return std::find (this->data.begin (), this->data.end (), v);
            };
            
            bool contains (const T & v) const {
                const auto end = this->data.end ();
                return std::find (this->data.begin (), end, v) != end;
            };
            void push_back (const T & v) {
                return this->data.push_back (v);
            };
            
            // TODO: insert?
    };
};

#endif

