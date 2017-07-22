#include "Windows_GetMessageEx.hpp"

/* Emphasize Windows routine for alertable wait on messages
// Windows_GetMessageEx.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      18.06.2011 - initial version
//      15.03.2012 - fixed returning 0 on WM_NULL, returning WM_TESTING instead
*/

#include "Windows_Version.hpp"

namespace {
    DWORD mwmo = 0u;
    DWORD mwmoInit ();
};

int Windows::GetMessageEx (LPMSG msg, HWND hWnd,
                           UINT wFilterLow, UINT wFilterHigh, BOOL alertable) {
    // peek first
    //  - when the wait call bellow is satisfied by alertable APC, the message
    //    queue is no longer signalled and won't return more messages, this
    //    check foreruns such state
    
    if (PeekMessage (msg, hWnd, wFilterLow, wFilterHigh, PM_REMOVE)) {
        if (msg->message == WM_QUIT) {
            SetLastError (ERROR_SUCCESS);
            return 0;
        } else
        if (msg->message == WM_NULL)
            return 0x0040; // WM_TESTING
        else
            return msg->message;
    };
    
    // initialize OS version dependent flags
    //  - doing it here delays the initialization to the moment when there
    //    are no more immediate messages present in the queue
    
    if (mwmo == 0u) {
        mwmo = mwmoInit ();
    };
    
    // optionally alertable wait
    //  - calling MsgWaitForMultipleObjectsEx with no objects causes
    //    messages to be signalled with return code WAIT_OBJECT_0
    
    while (true)
    switch (MsgWaitForMultipleObjectsEx (0u, NULL, INFINITE, mwmo,
                                         alertable ? MWMO_ALERTABLE : 0u)) {
        case WAIT_OBJECT_0:
            if (PeekMessage (msg, hWnd, wFilterLow, wFilterHigh, PM_REMOVE)) {
                if (msg->message == WM_QUIT) {
                    SetLastError (ERROR_SUCCESS);
                    return 0;
                } else
                if (msg->message == WM_NULL)
                    return 0x0040; // WM_TESTING
                else
                    return msg->message;
            };
            // at this point some new message arrived, but did not conform to
            // the filter, so just continue and wait for more messages
            continue;
        
        case WAIT_IO_COMPLETION:
            // APC alertable callback delivered, continue wait
            continue;
        
        default:
            SetLastError (E_UNEXPECTED);
            
        case WAIT_FAILED:
            return 0;
    };
};

namespace {
    DWORD mwmoInit () {
        if (Windows::Version > Windows::Version::Windows2000)
            return 0x04FFu; // QS_ALLINPUT | QS_RAWINPUT
        else
            return 0x00FFu; // QS_ALLINPUT for Win2k
    };
};
