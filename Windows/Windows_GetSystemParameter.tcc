#ifndef WINDOWS_GETSYSTEMPARAMETER_TCC
#define WINDOWS_GETSYSTEMPARAMETER_TCC

/* Emphasize Windows system parameters simple access
// Windows_GetSystemParameter.tcc
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      26.12.2011 - initial version
*/

#include <windows.h>
#include "../Windows/Windows_W2kCompatibility.hpp"

namespace Windows {
    
    // SpiGet mapping table
    //  - mapping SPI_GET constants to 
    
    #define ENTRY(code,p,type)              \
        template <>                         \
        struct SpiGet <code> {              \
            typedef type        Type;       \
            static const UINT   Param = p;  \
            static void Init (Type &) {};   \
        }

    ENTRY (SPI_GETCLIENTAREAANIMATION,  0u, BOOL);
    ENTRY (SPI_GETSHOWSOUNDS,           0u, BOOL);

    ENTRY (SPI_GETMENUFADE,             0u, BOOL);
    ENTRY (SPI_GETMENUANIMATION,        0u, BOOL);
    ENTRY (SPI_GETMENUDROPALIGNMENT,    0u, BOOL);
    ENTRY (SPI_GETCOMBOBOXANIMATION,    0u, BOOL);
    ENTRY (SPI_GETSELECTIONFADE,        0u, BOOL);
    
    ENTRY (SPI_GETFONTSMOOTHING,        0u, BOOL);
    ENTRY (SPI_GETMOUSEVANISH,          0u, BOOL);
    
    ENTRY (SPI_GETMOUSEHOVERHEIGHT,     0u, UINT);
    ENTRY (SPI_GETMOUSEHOVERWIDTH,      0u, UINT);
    ENTRY (SPI_GETMOUSEHOVERTIME,       0u, UINT);
    ENTRY (SPI_GETMOUSESPEED,           0u, UINT);

    ENTRY (SPI_GETWHEELSCROLLCHARS,     0u, UINT);
    ENTRY (SPI_GETWHEELSCROLLLINES,     0u, UINT);

    #undef ENTRY
};

// Windows::GetSystemParameter ()
//  - simple forwarder to call with default parameter

template <int P>
    typename Windows::SpiGet<P>::Type
Windows::GetSystemParameter () {
    return Windows::GetSystemParameter <P> (typename SpiGet<P>::Type ());
};

// Windows::GetSystemParameter (Type)
//  - the main function

template <int P>
    typename Windows::SpiGet<P>::Type
Windows::GetSystemParameter (typename Windows::SpiGet<P>::Type default_) {

    // result
    //  - the variable/structure, default-initialized at first
    //  - then the Init function is called upon it, usually NO-OP

    typename
    SpiGet<P>::Type result = typename SpiGet<P>::Type ();
    SpiGet<P>::Init (result);
    
    // SystemParametersInfo
    //  - passing correct parameters this should get correct result
    //  - on error (usually unsupported by current OS version) returns default
    
    if (SystemParametersInfo (P, SpiGet<P>::Param, &result, 0))
        return result;
    else
        return default_;
};


#endif
