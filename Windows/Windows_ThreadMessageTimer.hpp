#ifndef WINDOWS_THREADMESSAGETIMER_HPP
#define WINDOWS_THREADMESSAGETIMER_HPP

/* Emphasize Windows thread message timer callbacks
// Windows_ThreadMessageTimer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
// Description: The following call:
//              SetTimer (NULL, 0, T, Windows::ThreadMessageTimer <WM_USER>);
//              will set timer (in T milliseconds period) that sends the
//              WM_USER (or any) message.
//
// Changelog:
//      04.08.2011 - initial version
//      19.09.2012 - extended of LParam template argument
*/

#include <windows.h>

namespace Windows {
    
    // ThreadMessageTimer
    //  - 
    
    template <unsigned int MSG, LONG_PTR LParam = 0>
    void CALLBACK ThreadMessageTimer (HWND, UINT, UINT_PTR id, DWORD) {
        PostThreadMessage (GetCurrentThreadId (), MSG, id, LParam);
        return;
    };
    
    // OneShotThreadMessageTimer
    //  - 
    
    template <unsigned int MSG, LONG_PTR LParam = 0>
    void CALLBACK OneShotThreadMessageTimer (HWND, UINT, UINT_PTR id, DWORD) {
        PostThreadMessage (GetCurrentThreadId (), MSG, id, LParam);
        KillTimer (NULL, id);
        return;
    };    
};

#endif
