#ifndef WINDOWS_HANDLEVECTOR_TCC
#define WINDOWS_HANDLEVECTOR_HPP

/* Emphasize Plain containers Library - Vector
// Windows_HandleVector.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      06.06.2011 - initial version
//      31.03.2013 - reimplemented over Windows::Heap::Vector
*/

template <unsigned StockSize, unsigned int NullValue>
bool Windows::HandleVector <StockSize, NullValue> ::Add (HANDLE add) {
    
    auto i = 0u;
    auto n = this->super::Size ();
    
    while (i != n) {
        auto & h = this->super::operator [] (i++);
        if (h == NullValue) {
            h = add;
            return true;
        };
    };
    
    return this->super::Add (add);
};

template <unsigned StockSize, unsigned int NullValue>
unsigned int Windows::HandleVector <StockSize, NullValue> ::Remove (HANDLE rem) {
    
    auto i = 0u;
    auto m = 0u;
    auto n = this->super::Size ();
    
    while (i != n) {
        auto & h = this->super::operator [] (i++);
        if (h == rem) {
            h = NullValue;
            ++m;
        };
    };
    
    return m;
};


template <unsigned StockSize, unsigned int NullValue>
unsigned int Windows::HandleVector <StockSize, NullValue> ::Count () const {
    
    auto i = 0u;
    auto m = 0u;
    auto n = this->super::Size ();
    
    while (i != n) {
        auto h = this->super::operator [] (i++);
        if (h != NullValue) {
            ++m;
        };
    };
    
    return m;
};

#endif
