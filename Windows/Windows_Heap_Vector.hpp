#ifndef WINDOWS_HEAP_VECTOR_HPP
#define WINDOWS_HEAP_VECTOR_HPP

/* Windows Heap Vector 
// Windows_Heap_Vector.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      30.03.2013 - initial version
*/

#include <cstddef>

namespace Windows {
namespace Heap {
    
    // Allocator
    // DefaultAllocator
    //  - policies for following Vector class
    //  - Allocator allows to specify own heap handle, DefaultAllocator uses
    //    process default heap
    //  - TODO: - move to separated units (now implemented in .cpp)
    //          - more overhaul (especially type-safety) is required
    
    class Allocator {
        private:
            void * handle;
        public:
            inline constexpr Allocator (void * h) : handle (h) {};
            
            void * Alloc (void *, std::size_t);    // allocate or reallocate
            void Free (void *);                     // release
            std::size_t Size (void *);             // returns size
    };
    class DefaultAllocator {
        public:
            inline constexpr DefaultAllocator () {};
            inline constexpr DefaultAllocator (void *) {};
            
            void * Alloc (void *, std::size_t);
            void Free (void *);
            std::size_t Size (void *);
    };
    
    // Heap::Vector
    //  - used as storage when usually only StockCount or less item is needed
    //    and overcoming this limit is not seen as performance critical
    //  - StockCount - static number of T objects
    //               - no memory is allocated unless count exceeds this number
    //  - Alloc - any class that conforms to 'Allocator' interface defined above
    //          - derived from to exploit empty base class optimization
    
    template <typename T,
              unsigned StockCount,
              typename Alloc = DefaultAllocator>
    class Vector
        : private Alloc {
        
        public:
            typedef T                   ItemType;
            static const unsigned int   ExtraSize = (sizeof (T *) / sizeof (T)) ? (sizeof (T *) / sizeof (T)) : 1;
            static const unsigned int   StockSize = (ExtraSize < StockCount) ? StockCount - ExtraSize : 0;
            
        private:
            unsigned int size;
            T            stock [StockSize];
            union {
                T *     vector;
                T       extra [ExtraSize];
            };
        
        private:
            Vector (const Vector &) = delete;
            Vector & operator = (const Vector &) = delete;
            
        public:
            Vector (void * heap = nullptr)
#ifdef _MSC_VER
                : Alloc::Alloc (heap), size (0u) {};
#else
                : Alloc (heap), size (0u) {};
#endif
            ~Vector ();
            
            // Add
            //  - copies T into the datavector
            //  - returns: true when success
            //             false on error (out of memory), see GetLastError ();
            
            bool Add (const T &);
            
            // Size
            //  - returns number of T objects in the vector
            
            unsigned int Size () const { return size; };
            
            // operator[]
            
            const T & operator [] (unsigned int i) const;
                  T & operator [] (unsigned int i);
            
            // Clear
            //  - empty the container and release excess memory (if any)
            
            void Clear ();
            
            // Release
            //  - returns data vector pointer (if any)and forgets all data
            //  - useful when exiting application, to skip useless deallocating
            
            T * Release ();
            
            // TODO: begin and end!
    };
};
};

#include "Windows_Heap_Vector.tcc"
#endif

