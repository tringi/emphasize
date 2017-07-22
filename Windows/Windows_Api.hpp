#ifndef WINDOWS_API_HPP
#define WINDOWS_API_HPP

/* Windows Api 
// Windows_Api.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      27.04.2013 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // Api
    //  - 
    
    extern class Api {
        public:
            explicit Api ();
        
        public:
            enum KnownApis {
                Native,     // 32b API on 32b Windows or 64b API on 64b Windows
                WOW64,      // 32b API on 64b Windows
                    WOW = WOW64,
                Wine,       // Wine Linux Layer
                ReactOS,    // ReactOS Layer
                
                NumberOfKnownApis
            };
        
        private:
            const KnownApis value;
        
        public:
            bool operator == (KnownApis x) const { return this->value == x; };
            bool operator != (KnownApis x) const { return this->value != x; };
        
    } Api;

};

#endif

