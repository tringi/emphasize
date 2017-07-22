#ifndef WINDOWS_GETSYSTEMPARAMETER_HPP
#define WINDOWS_GETSYSTEMPARAMETER_HPP

/* Emphasize Windows system parameters simple access
// Windows_GetSystemParameter.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: Abstracts SystemParametersInfo call and retrieves the requested
//              values already as the documented type or structure.
//
// Changelog:
//      26.12.2011 - initial version
*/

namespace Windows {
    
    // SpiGet
    //  - internal helper, using SPI_GETxxx constants as parameters
    
    template <int> struct SpiGet {};

    // GetSystemParameter
    //  - returns requested system parameter
    //  - on error: - first version returns default-initialized SPI_GET<P>::Type
    //              - second version returns provided 'def' parameter
    //  - example: Windows::GetSystemParameter <SPI_GETWHEELSCROLLLINES> ();
    //  - NOTE: this would be much nicer if return value was second, default
    //          initialized, template parameter, but that is C++11 feature
    
    template <int P>
    typename SpiGet<P>::Type GetSystemParameter ();

    template <int P>
    typename SpiGet<P>::Type GetSystemParameter (typename SpiGet<P>::Type def);

};

#include "Windows_GetSystemParameter.tcc"
#endif
