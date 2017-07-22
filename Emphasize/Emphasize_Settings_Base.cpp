#include "Emphasize_Settings_Base.hpp"

/* Emphasize Settings_Base 
// Emphasize_Settings_Base.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      14.06.2012 - initial version
*/

#include <stdio.h>

Emphasize::Settings::Base::Base (Emphasize::Settings & _settings,
                                 const wchar_t * _name,
                                 const wchar_t * _path) noexcept
    :   settings (_settings),
        next (nullptr),
        name (_name),
        path (_path) {
    
    // Chain this setting to the end of the (forward) list
    //  - simple: point both last's next and then 'last' itself to 'this'
    //  - if 'this' is first there (settings.first is NULL), just set both
    
    if (settings.settings.first) {
        settings.settings.last->next = this;
        settings.settings.last = this;
    } else {
        settings.settings.first = this;
        settings.settings.last = this;
    };

    return;
};

Emphasize::Settings::Base::~Base () noexcept {
    
    // Unlinking this setting from Settings list
    //  - if this is first, point first to our next and we are almost done
    
    if (this->settings.settings.first == this) {
        this->settings.settings.first = this->next;
        
        // If this is also last, just reset them both
        //  - this->next is certainly NULL at this point, but using this->next
        //    instead NULL (nullptr) saves 3 bytes, since it already is in
        //    register (%ecx usually)
        
        if (this->settings.settings.last == this)
            this->settings.settings.last = this->next;
        
    } else {
        
        // Not first
        //  - find element that preceedes 'this' in the list
        
        auto p = this->settings.settings.first;
        while (p->next != this) {
            p = p->next;
        };
        
        // Release 'this' from list
        
        p->next = p->next->next;
        
        // Update settings' last
        //  - if the last (not pointing to next) was actually removed
        
        if (!p->next) {
            this->settings.settings.last = p;
        };
    };
    return;
};

bool Emphasize::Settings::Base::erase () {
    return this->settings.erase (this);
};
