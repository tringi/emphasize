#ifndef WINDOWS_SETTIMER_HPP
#define WINDOWS_SETTIMER_HPP

/* Windows SetTimer - abstraction to seamlessly introduce latest features
// Windows_SetTimer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      23.03.2014 - initial version
*/

#include <windows.h>

#ifndef USER_TIMER_MINIMUM
#define USER_TIMER_MINIMUM 0xA
#endif
#ifndef USER_TIMER_MAXIMUM
#define USER_TIMER_MAXIMUM 0x7FFFFFFF
#endif

#ifndef TIMERV_DEFAULT_COALESCING
#define TIMERV_DEFAULT_COALESCING 0
#endif
#ifndef TIMERV_NO_COALESCING
#define TIMERV_NO_COALESCING 0xFFFFFFFF
#endif

namespace Windows {
    
    // SetTimer[AndFire]
    //  - sets coalescable timer where available and normal where not
    /// - the call or WM_TIMER is schedulled immediately for SetTimerAndFire
    //  - parameters: HWND - handle to window owning the timer
    //                UINT_PTR - timer ID (to e.g. override previous settings)
    //                UINT - period of timer (in ms)
    //                TIMERPROC - procedure to call or NULL for WM_TIMER message
    //                          - note: procedure is called by DispatchMessage
    //                ULONG - tolerable delay (for coalescing), 0 means default
    //  - returns: timer ID on success
    //             0 on failure (call GetLastError ())
    
    UINT_PTR SetTimer (HWND, UINT_PTR, UINT period, TIMERPROC procedure = NULL, ULONG tolerance = 0);
    UINT_PTR SetTimerAndFire (HWND, UINT_PTR, UINT period, TIMERPROC procedure = NULL, ULONG tolerance = 0);

    // SetTimer[AndFire] without HWND/ID
    //  - allocated coalescable timer where available and normal where not
    //     - IDs are usually allocated from 32767 down to 256 and recycled
    /// - the call or WM_TIMER is schedulled immediately for SetTimerAndFire
    //  - parameters: TIMERPROC - procedure to call or NULL for WM_TIMER message
    //                          - note: procedure is called by DispatchMessage
    //                UINT - period of timer (in ms)
    //                ULONG - tolerable delay (for coalescing), 0 means default
    //  - returns: new timer ID on success
    //             0 on failure (call GetLastError ())
    
    UINT_PTR SetTimer (UINT period, TIMERPROC procedure, ULONG tolerance = 0);
    UINT_PTR SetTimerAndFire (UINT period, TIMERPROC procedure = NULL, ULONG tolerance = 0);
    
    // SetWaitableTimer
    //  - TODO: more...
    
    BOOL SetWaitableTimer (HANDLE, LONG period, ULONG tolerance = 0);
    
/*    struct DueTime : LARGE_INTEGER {
        DueTime (LONG);
        DueTime (const FILETIME &);
        DueTime (const SYSTEM_TIME &);
    };*/
    
    // ScheduleWaitableTimer ???
/*    BOOL SetWaitableTimerOnce (HANDLE, LONG delay, ULONG tolerance = 0);
    BOOL SetWaitableTimerOnce (HANDLE, FILETIME when, ULONG tolerance = 0);
    BOOL SetWaitableTimerOnce (HANDLE, SYSTEM_TIME when, ULONG tolerance = 0);*/
};

#endif

