#include "Windows_SetTimer.hpp"

/* Windows SetTimer - abstraction to seamlessly introduce latest features
// Windows_SetTimer.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      23.03.2014 - initial version
*/

#include "Windows_Symbol.hpp"

namespace {
    UINT_PTR (WINAPI * ImpSetCoalescableTimer) (HWND, UINT_PTR, UINT, TIMERPROC, ULONG) = NULL;
    UINT_PTR  WINAPI   ImpSetCoalescableTimerDefault (HWND hWnd, UINT_PTR nIDEvent, UINT uElapse,
                                                      TIMERPROC lpTimerFunc, ULONG uToleranceDelay) {
        
        // uElapse limited to bounds
        //  - Windows XP does this automatically, but we need to ensure
        //    bounding values for following check
        //  - this also fixes behavior for Windows 2000
        
        if (uElapse < USER_TIMER_MINIMUM) uElapse = USER_TIMER_MINIMUM;
        if (uElapse > USER_TIMER_MAXIMUM) uElapse = USER_TIMER_MAXIMUM;
        
        // ensure validity of uToleranceDelay and uElapse
        //  - this is, per docs, how SetCoalescableTimer validates
        //  - checking here in order to unify behavior for invalid values
        
        if (uToleranceDelay > 0x7FFFFFF5 /* maximum documented */
            && uToleranceDelay != 0xFFFFFFFF /* TIMERV_NO_COALESCING */) {
            
            SetLastError (ERROR_INVALID_PARAMETER);
            return 0;
        };
        
        if ((uElapse + uToleranceDelay) > 0x7FFFFFFF /* USER_TIMER_MAXIMUM */) {
            SetLastError (ERROR_INVALID_PARAMETER);
            return 0;
        };
        
        return ::SetTimer (hWnd, nIDEvent, uElapse, lpTimerFunc);
    };

    typedef struct _REASON_CONTEXT {
        ULONG Version;
        DWORD Flags;
        union {
            struct {
                HMODULE LocalizedReasonModule;
                ULONG LocalizedReasonId;
                ULONG ReasonStringCount;
                LPWSTR *ReasonStrings;
            } Detailed;
            LPWSTR SimpleReasonString;
        } Reason;
    } REASON_CONTEXT, *PREASON_CONTEXT;

    typedef enum _POWER_REQUEST_TYPE {
        PowerRequestDisplayRequired,
        PowerRequestSystemRequired,
        PowerRequestAwayModeRequired,
        PowerRequestExecutionRequired
    } POWER_REQUEST_TYPE, *PPOWER_REQUEST_TYPE;

    #ifndef POWER_REQUEST_CONTEXT_VERSION
    #define POWER_REQUEST_CONTEXT_VERSION          0
    #endif
    #ifndef POWER_REQUEST_CONTEXT_SIMPLE_STRING
    #define POWER_REQUEST_CONTEXT_SIMPLE_STRING    0x00000001
    #define POWER_REQUEST_CONTEXT_DETAILED_STRING  0x00000002
    #endif

    BOOL (WINAPI * ImpSetWaitableTimerEx) (HANDLE, const LARGE_INTEGER *, LONG, PTIMERAPCROUTINE, LPVOID, PREASON_CONTEXT, ULONG) = NULL;
    BOOL  WINAPI   ImpSetWaitableTimerExDefault (HANDLE hTimer, const LARGE_INTEGER * lpDueTime, LONG lPeriod,
                                                 PTIMERAPCROUTINE pfnCompletionRoutine, LPVOID lpArgToCompletionRoutine,
                                                 PREASON_CONTEXT WakeContext, ULONG TolerableDelay) {
        
        WakeContext = WakeContext;
        TolerableDelay = TolerableDelay;
        
        return ::SetWaitableTimer (hTimer, lpDueTime, lPeriod,
                                   pfnCompletionRoutine, lpArgToCompletionRoutine,
                                   FALSE);
    };

    BOOL WINAPI CallSetWaitableTimerEx (HANDLE hTimer, const LARGE_INTEGER * lpDueTime, LONG lPeriod,
                                        PTIMERAPCROUTINE pfnCompletionRoutine, LPVOID lpArgToCompletionRoutine,
                                        PREASON_CONTEXT WakeContext, ULONG TolerableDelay) {
        if (!ImpSetWaitableTimerEx) {
            if (!Windows::Symbol (L"KERNEL32", ImpSetWaitableTimerEx, "SetWaitableTimerEx")) {
                ImpSetWaitableTimerEx = ImpSetWaitableTimerExDefault;
            };
        };
        return ImpSetWaitableTimerEx (hTimer, lpDueTime, lPeriod,
                                      pfnCompletionRoutine, lpArgToCompletionRoutine,
                                      WakeContext, TolerableDelay);
    };
};

// SetTimer

UINT_PTR Windows::SetTimer (HWND hWnd, UINT_PTR nIDEvent, UINT uElapse,
                            TIMERPROC lpTimerFunc, ULONG uToleranceDelay) {
    if (!ImpSetCoalescableTimer) {
        if (!Windows::Symbol (L"USER32", ImpSetCoalescableTimer, "SetCoalescableTimer")) {
            ImpSetCoalescableTimer = ImpSetCoalescableTimerDefault;
        };
    };
    return ImpSetCoalescableTimer (hWnd, nIDEvent, uElapse, lpTimerFunc, uToleranceDelay);
};

UINT_PTR Windows::SetTimerAndFire (HWND hWnd, UINT_PTR nIDEvent, UINT uElapse,
                                   TIMERPROC lpTimerFunc, ULONG uToleranceDelay) {
    UINT_PTR id = Windows::SetTimer (hWnd, nIDEvent, uElapse, lpTimerFunc, uToleranceDelay);
    if (id) {
        PostMessage (hWnd, WM_TIMER, (WPARAM) id, (LPARAM) lpTimerFunc);
    };
    return id;
};

UINT_PTR Windows::SetTimer (UINT uElapse, TIMERPROC lpTimerFunc, ULONG uToleranceDelay) {
    return Windows::SetTimer (NULL, 0, uElapse, lpTimerFunc, uToleranceDelay);
};

UINT_PTR Windows::SetTimerAndFire (UINT uElapse, TIMERPROC lpTimerFunc, ULONG uToleranceDelay) {
    return Windows::SetTimerAndFire (NULL, 0, uElapse, lpTimerFunc, uToleranceDelay);
};

// SetWaitableTimer

BOOL Windows::SetWaitableTimer (HANDLE handle, LONG period, ULONG tolerance) {
    LARGE_INTEGER due;
    
    due.QuadPart = UInt32x32To64 (10000, period);
    due.QuadPart *= -1;
    
/*    REASON_CONTEXT reason;
    reason.Version = POWER_REQUEST_CONTEXT_VERSION;
    reason.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    reason.Reason.SimpleReasonString = const_cast <LPWSTR> (L"generic");// */
    
    return CallSetWaitableTimerEx (handle, &due, period,
                                   NULL, NULL, NULL, tolerance);
};
