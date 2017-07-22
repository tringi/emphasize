#ifndef WINDOWS_HANDLEVECTOR_HPP
#define WINDOWS_HANDLEVECTOR_HPP

/* Emphasize Windows library - Handle Vector
// Windows_HandleVector.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
// Description: Simple handle vector, designed for small amounts of data.
//              Initially doesn't allocate any memory and keeps 'stock_size'
//              of handles in 'stock'.
//
// Changelog:
//      06.06.2011 - initial version
//      31.03.2013 - reimplemented over Windows::Heap::Vector
*/

#include <windows.h>
#include "Windows_Heap_Vector.hpp"

namespace Windows {
    
    // HandleVector
    //  - StockSize
    //  - NullValue (usually 0 or INVALID_HANDLE_VALUE)
    
    template <unsigned StockSize = 1u, unsigned NullValue = 0u>
    class HandleVector
        :   private Windows::Heap::Vector <HANDLE, StockSize> {
        
        private:
            typedef Windows::Heap::Vector <HANDLE, StockSize> super;
            
        public:
            HandleVector () {};
            
            bool         Add (HANDLE);
            unsigned int Remove (HANDLE);
            unsigned int Count () const;
            
            using super::Clear;
            using super::Release;
            
            // Enumerate
            //  - calls 'callback' with 'p' subsequently for every handle in
            //    the vector; when some 'callback' returns true, the function
            //    returns true, otherwise false is returned
            //  - implemented here to allow quick glances of the implementation
            
            inline
            bool Enumerate (bool (* callback)(HANDLE, void *), void * p) const {
                auto i = 0u;
                auto n = this->super::Size ();
                
                while (i != n) {
                    auto h = this->super::operator [] (i++);
                    if (h != NullValue) {
                        if (callback (h, p))
                            return true;
                    };
                };
                
                return false;
            };
            
        private:
            HandleVector (const HandleVector &) = delete;
            HandleVector & operator = (const HandleVector &) = delete;
    };
};

#include "Windows_HandleVector.tcc"
#endif
