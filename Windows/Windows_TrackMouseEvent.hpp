#ifndef WINDOWS_TRACKMOUSEEVENT_HPP
#define WINDOWS_TRACKMOUSEEVENT_HPP

/* Emphasize Windows TrackMouseEvent wrapper
// Windows_TrackMouseEvent.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      13.12.2011 - initial version
*/

#include <windows.h>

namespace Windows {

    // TrackMouseEvent
    //  - only simplifies call to global TrackMouseEvent Windows API function
    //  - parameters: HWND - handle to window to receive the appropriate message
    //                DWORD - events: TME_HOVER - to receive WM_MOUSEHOVER
    //                                TME_LEAVE - to receive WM_MOUSELEAVE
    //                      - can be combined with TME_NONCLIENT or TME_CANCEL
    //                DWORD - timeout for hover message

    bool TrackMouseEvent (HWND, DWORD, DWORD = HOVER_DEFAULT);
};

#endif
