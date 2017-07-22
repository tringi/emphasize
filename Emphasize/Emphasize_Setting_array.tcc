#ifndef EMPHASIZE_SETTING_ARRAY_TCC
#define EMPHASIZE_SETTING_ARRAY_TCC

/* Emphasize Setting - partial specializations for arrays
// Emphasize_Setting_array.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.08.2012 - initial version
*/

namespace ext {
    
    template <typename T>
    void default_construct (T & value) {
        value = T ();
        return;
    };
    
    // fill_extend
    //  - fills [TB;TE) range with data from [SB;SE) range, extending with last
    //    element of source if target range is longer, stopping at TE if target
    //    range is shorter than source
    
    template <typename TargetIterator,
              typename SourceIterator>
    TargetIterator fill_extend (TargetIterator tb, TargetIterator te,
                                SourceIterator sb, SourceIterator se) {
        if (sb != se) {
            for (; (sb != se) && (tb != te); ++sb, ++tb) {
                *tb = *sb;
            };
            for (--sb; tb != te; ++tb) {
                *tb = *sb;
            };
        } else {
            for (; tb != te; ++tb) {
                default_construct (*tb);
            };
        };
        return tb;
    };
};

template <typename T, unsigned int N>
Emphasize::Setting <T[N]> ::Setting (Emphasize::Settings & settings,
                                     const wchar_t * name,
                                     const std::initializer_list <T> & defaults)
    :   Emphasize::Settings::Base (settings, name) {
    
    ext::fill_extend (&this->data [0], &this->data [N],
                      defaults.begin (), defaults.end ());
    return;
};
template <typename T, unsigned int N>
Emphasize::Setting <T[N]> ::Setting (Emphasize::Settings & settings,
                                     const wchar_t * path, const wchar_t * name,
                                     const std::initializer_list <T> & defaults)
    :   Emphasize::Settings::Base (settings, name, path) {
    
    ext::fill_extend (&this->data [0], &this->data [N],
                      defaults.begin (), defaults.end ());
    return;
};

template <typename T, unsigned int N>
template <typename F>
bool Emphasize::Setting <T[N]> ::for_all_subelements (F operation) const {
    
    // buffer for sub-element names
    //  - using G++ extension
    
    bool ok = true;
    auto nn = std::wcslen (this->name) + 16;
    auto ename = new wchar_t [nn];
    
    // build names for all sub-elements
    
    for (auto i = 0u; i < N; ++i) {
        _snwprintf (ename, nn, L"%s#%u", this->name, i);
        
        // and save everyone
        //  - remember to fail if any save operation failed
        //  - on success, copy data back (relevant on .load() operation only)
        
        Emphasize::Setting <T> subelement (this->settings, this->path,
                                           ename, this->data [i]);
        if ((subelement.*operation) ()) {
            this->data [i] = subelement;
        } else {
            ok = false;
        };
    };
    
    delete [] ename;
    return ok;
};

template <typename T, unsigned int N>
bool Emphasize::Setting <T[N]> ::load () const {
    if (this->settings.active ()) {
        return this->for_all_subelements (&Emphasize::Setting <T> ::load);
    } else
        return this->settings.load (this);
};
template <typename T, unsigned int N>
bool Emphasize::Setting <T[N]> ::save () {
    if (this->settings.active ()) {
        return this->for_all_subelements (&Emphasize::Setting <T> ::save);
    } else
        return this->settings.save (this);
};
template <typename T, unsigned int N>
bool Emphasize::Setting <T[N]> ::erase () {
    if (this->settings.active ()) {
        return this->for_all_subelements (&Emphasize::Setting <T> ::erase);
    } else
        return this->settings.erase (this);
};


template <typename T>
Emphasize::Setting <T[]> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * name)
    :   Emphasize::Settings::Base (settings, name) {};

template <typename T>
Emphasize::Setting <T[]> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * name,
                                    std::initializer_list <T> defaults)
    :   Emphasize::Settings::Base (settings, name),
        data (defaults) {};

template <typename T>
Emphasize::Setting <T[]> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * path, const wchar_t * name)
    :   Emphasize::Settings::Base (settings, name, path) {};

template <typename T>
Emphasize::Setting <T[]> ::Setting (Emphasize::Settings & settings,
                                    const wchar_t * path, const wchar_t * name,
                                    std::initializer_list <T> defaults)
    :   Emphasize::Settings::Base (settings, name, path),
        data (defaults) {};

template <typename T>
bool Emphasize::Setting <T[]> ::load () const {
    
    if (this->settings.active ()) {
        auto i = 0u;
        auto nn = std::wcslen (this->name) + 16;
        auto ename = new wchar_t [nn];
        
        // remove anything already held
        //  - because if load find nothing then it is an empty array
        
        this->data.clear ();
        
        while (true) {
            _snwprintf (ename, nn, L"%s#%u", this->name, i);
            
            // attempt to load sequentially all elements
            //  - the point at which load fails is considered the end of the array
            
            Emphasize::Setting <T> element (this->settings, this->path, ename);
            if (element.load ()) {
                this->data.push_back (element);
                ++i;
            } else
                break;
        };
        
        delete [] ename;
        return true;
    } else
        return this->settings.load (this);
};

template <typename T>
bool Emphasize::Setting <T[]> ::save () {
    
    if (this->settings.active ()) {
        auto i = 0u;
        auto nn = std::wcslen (this->name) + 16;
        auto ename = new wchar_t [nn];

        // save all array elements
        //  - returning early on failure to avert potential data loss
        
        for (; i != this->data.size (); ++i) {
            _snwprintf (ename, nn, L"%s#%u", this->name, i);
    
            if (!Emphasize::Setting <T> (this->settings,
                                         this->path, ename,
                                         this->data [i]).save ())
                return false;
        };
        
        // erase all remains from previous state
        //  - simply continue erasing until the operation fails
        //    (i.e. there are no more elements to be deleted)
        
        do {
            _snwprintf (ename, nn, L"%s#%u", this->name, i++);
            
        } while (Emphasize::Setting <T>
                 (this->settings, this->path, ename).erase ());
        
        delete [] ename;
        return true;
    } else
        return this->settings.save (this);
};

template <typename T>
bool Emphasize::Setting <T[]> ::erase () {

    if (this->settings.active ()) {
        auto i = 0u;
        auto nn = std::wcslen (this->name) + 16;
        auto ename = new wchar_t [nn];
        
        // simple erase loop
        //  - erasing all elements until the operation fails
        //    (i.e. there are no more elements to be deleted)

        do {
            _snwprintf (ename, nn, L"%s#%u", this->name, i++);
            
        } while (Emphasize::Setting <T>
                 (this->settings, this->path, ename).erase ());
        
        delete [] ename;
        return true;
    } else
        return this->settings.erase (this);
};

#endif
