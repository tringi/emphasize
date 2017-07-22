#ifndef WINDOWS_HEAP_VECTOR_TCC
#define WINDOWS_HEAP_VECTOR_TCC

/* Windows Heap Vector 
// Windows_Heap_Vector.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      30.03.2013 - initial version
*/

#include <cstdio>

template <typename T, unsigned int StockCount, typename Alloc>
Windows::Heap::Vector <T, StockCount, Alloc> ::~Vector () {
    this->Clear ();
    return;
};

template <typename T, unsigned int StockCount, typename Alloc>
void Windows::Heap::Vector <T, StockCount, Alloc> ::Clear () {
    if (this->size > StockCount) {
        this->Alloc::Free (this->vector);
    };
    this->size = 0u;
    return;
};

template <typename T, unsigned int StockCount, typename Alloc>
T * Windows::Heap::Vector <T, StockCount, Alloc> ::Release () {
    if (this->size > StockCount) {
        
        T * rv = this->vector;
        this->vector = NULL;
        this->size = 0u;
        
        return rv;
    } else
        return NULL;
};

template <typename T, unsigned int StockCount, typename Alloc>
bool Windows::Heap::Vector <T, StockCount, Alloc> ::Add (const T & object) {
    if (this->size < this->StockSize) {
        this->stock [this->size] = object;
        ++this->size;
        return true;
        
    } else
    if (this->size < StockCount) {
        this->extra [this->size - this->StockSize] = object;
        ++this->size;
        return true;
        
    } else
    if (this->size == StockCount) {
        
        if (T * memory = reinterpret_cast <T *>
                (this->Alloc::Alloc (NULL, sizeof (T) * 2u * this->ExtraSize))) {

            for (unsigned int i = 0; i != this->size - this->StockSize; ++i) {
                memory [i] = this->extra [i];
            };
            memory [this->size - this->StockSize] = object;
            
            this->vector = memory;
            ++this->size;
            return true;
        } else
            return false;
        
    } else {
        
        const auto vsize = this->Alloc::Size (this->vector);
        const auto depth = vsize / sizeof (T);
        const auto offset = this->size - this->StockSize;
        
        if (offset < depth) {
            this->vector [offset] = object;
            ++this->size;
            return true;
        } else {
            
            // TODO: Windows::Heap::AllocationGrowRate
            
            if (T * memory = reinterpret_cast <T *>
                    (this->Alloc::Alloc (this->vector, 2u * vsize))) {
                
                this->vector = memory;
                return this->Add (object);
            } else
                return false;
        };
    };
};

template <typename T, unsigned int StockCount, typename Alloc>
const T & Windows::Heap::Vector <T, StockCount, Alloc> ::operator [] (unsigned int i) const {
    if (i < this->StockSize)
        return this->stock [i];
    else
    if (this->size <= StockCount)
        return this->extra [i - this->StockSize];
    else
        return this->vector [i - this->StockSize];
};

template <typename T, unsigned int StockCount, typename Alloc>
T & Windows::Heap::Vector <T, StockCount, Alloc> ::operator [] (unsigned int i) {
    if (i < this->StockSize)
        return this->stock [i];
    else
    if (this->size <= StockCount)
        return this->extra [i - this->StockSize];
    else
        return this->vector [i - this->StockSize];
};

#endif

