#include "Windows_TrackMouseEvent.hpp"

/* Emphasize Windows TrackMouseEvent wrapper
// Windows_TrackMouseEvent.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      13.12.2011 - initial version
*/

bool Windows::TrackMouseEvent (HWND hWnd, DWORD dwFlags, DWORD dwHoverTime) {
    TRACKMOUSEEVENT tme = {
        sizeof (TRACKMOUSEEVENT),
        dwFlags, hWnd, dwHoverTime
    };
    return ::TrackMouseEvent (&tme);
};
