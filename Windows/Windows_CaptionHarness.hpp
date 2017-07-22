#ifndef WINDOWS_CAPTIONHARNESS_HPP
#define WINDOWS_CAPTIONHARNESS_HPP

/* Emphasize Windows Client area to Title bar extension
// Windows_CaptionHarness.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
// Description: Facility (message processing) for windows that have
//              the client area extended into their title bar.
//
// Changelog:
//      23.06.2011 - initial version
//      09.10.2013 - extended of four new types of glass frame - 2.0
*/

#include <windows.h>

namespace Windows {
namespace CaptionHarness {
    
    // Type
    //  - 
    
    enum Type {
        Basic,          // extended glassy borders
        FullGlass,      // whole window is glass, has title and icon
        JustGlass,      // whole window is glass, no title/icon, full client area
        ClientTitle,    // extended borders, no title/icon, full client area
    };
    
    // Settings
    //  - extended caption and borders settings
    //  - the window should be redrawn when metrics in this structure change
    
    struct Settings {
        Type type;
        RECT margins; // extension into client area (top must include caption)
        UINT split;   // where to split left/right border for extended caption
        bool menu;    // set true to enable window menu on caption icon
    };
    
    // IsProcessibleMesssage (UINT)
    //  - returns false for messages that ProcessMessage ignores
    //  - TODO: consider moving implementation here so it can be inlined
    
    bool IsProcessibleMesssage (UINT);
    
    // ProcessMessage
    //  - processes window messages for caption and border extension
    //  - returns: true - if the message was processed and the window procedure
    //                    should return the value stored in the LRESULT pointer
    //             false - returned when not handled
    //                   - typically DwmDefWindowProc/DefWindowProc gets called
    
    bool ProcessMessage (HWND, UINT, WPARAM, LPARAM,
                         const Settings &, LRESULT *);
    
};
};

#endif
