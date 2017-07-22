#include "Windows_Heap_Vector.hpp"

/* Windows Heap Vector 
// Windows_Heap_Vector.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      30.03.2013 - initial version
*/

#include <windows.h>

void * Windows::Heap::Allocator::Alloc (void * ptr, std::size_t size) {
    return ptr
         ? HeapReAlloc (this->handle, 0, ptr, size)
         : HeapAlloc (this->handle, 0, size);
};
void Windows::Heap::Allocator::Free (void * ptr) {
    HeapFree (this->handle, 0, ptr);
};
std::size_t Windows::Heap::Allocator::Size (void * ptr) {
    return HeapSize (this->handle, 0, ptr);
};

void * Windows::Heap::DefaultAllocator::Alloc (void * ptr, std::size_t size) {
    return ptr
         ? HeapReAlloc (GetProcessHeap (), 0, ptr, size)
         : HeapAlloc (GetProcessHeap (), 0, size);
};
void Windows::Heap::DefaultAllocator::Free (void * ptr) {
    HeapFree (GetProcessHeap (), 0, ptr);
};
std::size_t Windows::Heap::DefaultAllocator::Size (void * ptr) {
    return HeapSize (GetProcessHeap (), 0, ptr);
};
