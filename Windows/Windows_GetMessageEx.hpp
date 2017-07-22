#ifndef WINDOWS_GETMESSAGEEX_HPP
#define WINDOWS_GETMESSAGEEX_HPP

/* Emphasize Windows routine for alertable wait on messages
// Windows_GetMessageEx.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
// Description: 
//
// Changelog:
//      18.06.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // GetMessageEx
    //  - alertable version of the GetMessage system function
    //  - parameters: - the same as for the GetMessage function
    //                - when last BOOL is TRUE, the call is alertable
    //                  otherwise the function behaves just like GetMessage
    //  - returns: - positive number (msg->message) when message is retrieved
    //             - 0 when WM_QUIT is retrieved or on error
    //                - call GetLastError, ERROR_SUCCESS means WM_QUIT
    
    int GetMessageEx (LPMSG, HWND, UINT, UINT, BOOL);
};

#endif
