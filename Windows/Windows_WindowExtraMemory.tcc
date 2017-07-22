#ifndef WINDOWS_WINDOWEXTRAMEMORY_TCC
#define WINDOWS_WINDOWEXTRAMEMORY_TCC

/* Emphasize Windows window extra memory accessor templates
// Windows_WindowExtraMemory.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      07.04.2011 - initial version
*/

#include <windows.h>
#include <algorithm>
#include <cstring>
#include <limits>

// Windows::SetWindowExtraImplementation
//  - the following classes are implementation detail and may change

namespace Windows {
    template <typename T, typename D, typename S, bool, bool>
    struct SetWindowExtraImplementation;

    template <typename T, typename D, typename S>
    struct SetWindowExtraImplementation <T, D, S, true, false>;

    template <typename T, typename D, typename S>
    struct Windows::SetWindowExtraImplementation <T, D, S, false, true>;
};

// Windows::GetWindowExtraSize
//  - size of D increased of (sizeof(LONG_PTR)-1) because we need to be able
//    to access the last member variable even if it is a single byte

template <typename D>
UINT Windows::GetWindowExtraSize () {
    return sizeof (D) + sizeof (LONG_PTR) - 1u;

// WAS: size of D rounded to the nearest larger multiple of sizeof (LONG_PTR)
//    return  (sizeof (D) - 1u) + sizeof (LONG_PTR)
//         - ((sizeof (D) - 1u) % sizeof (LONG_PTR));
};

// Windows::GetWindowExtra
// Windows::SetWindowExtra
//  - implementation of the public interface

template <typename T, typename D>
T Windows::GetWindowExtra (HWND hWnd, T D::* Offset) {
    return (T) GetWindowLongPtr (hWnd, (int) (&(((D *) NULL)->*Offset)));
};

template <typename T, typename D, typename S>
T Windows::SetWindowExtra (HWND hWnd, T D::* Offset, S value) {
    return Windows::SetWindowExtraImplementation <T, D, S,
                                                  sizeof (T) == sizeof (LONG_PTR),
                                                  sizeof (T) <= sizeof (LONG_PTR)>
                        () (hWnd, Offset, value);
};

// Windows::SetWindowExtraImplementation
//  - opening the namespace is required by C++ standard

namespace Windows {

    // Windows::SetWindowExtraImplementation <true, true> specialization
    //  - ideal path, where size of written type is equal to the number of bytes
    //    that SetWindowLongPtr function writes
    
    template <typename T, typename D, typename S>
    struct SetWindowExtraImplementation <T, D, S, true, true> {
        T operator () (HWND hWnd, T D::* Offset, S value) {
            return (T) SetWindowLongPtr (hWnd,
                                         (int) (&(((D *) NULL)->*Offset)),
                                         (LONG_PTR) (T) value);
        };
    };
    
    // Windows::SetWindowExtraImplementation <true, false> specialization
    //  - other typical path, where size of written type is less than the
    //    number of bytes that SetWindowLongPtr function writes
    //  - reading the value first, zeroing lower sizeof(T) bytes, adding
    //    the value and setting it back
    
    template <typename T, typename D, typename S>
    struct SetWindowExtraImplementation <T, D, S, false, true> {
        T operator () (HWND hWnd, T D::* Offset, S value) {
            LONG_PTR v = GetWindowLongPtr (hWnd, (int) (&(((D *) NULL)->*Offset)));
            LONG_PTR w = 0;
            
            std::memcpy (&w, &value, std::min (sizeof (S), sizeof (T)));
            
            v >>= std::numeric_limits <unsigned char> ::digits * sizeof (T);
            v <<= std::numeric_limits <unsigned char> ::digits * sizeof (T);

            return (T) SetWindowLongPtr (hWnd,
                                         (int) (&(((D *) NULL)->*Offset)),
                                         v | w);
        };
    };

    // Windows::SetWindowExtraImplementation <false, false> specialization
    //  - error path, where size of written type is larger than the number
    //    of bytes that SetWindowLongPtr function can write
    
    template <typename T, typename D, typename S, bool, bool>
    struct SetWindowExtraImplementation {
        private:
            T operator () (HWND, T D::*, S);
    };
};

#endif
