#ifndef WINDOWS_QUERYDYNAMICMEMORYMAXIMUM_HPP
#define WINDOWS_QUERYDYNAMICMEMORYMAXIMUM_HPP

#include <windows.h>
#include <cstddef>
#include <cstdint>

namespace Windows {

    // Windows::QueryDynamicMemoryMaximum
    //  - uses Vista+ perfromance counters API to retrieve the value
    //     - "Hyper-V Dynamic Memory Integration Service" / "Maximum Memory, MBytes"
    //     - the value is acquired from root partition through VMBus in Kernel mode by DMVSC.sys
    //       driver, and published only as a performance counter and WMI value (slow to query)
    //     - if Dynamic Memory is not enabled, the value equals to assigned RAM
    //  - ON SUCCESS: writes dynamic memory maximum into 'result' (in bytes)
    //     - 'result' can be nullptr if the actual value is not needed
    //     - result value is always in multiple of 2 MBs
    //        - Hyper-V Dynamic Memory has granularity of 2 MB
    //        - raw value stored is in MBs
    //  - returns: ERROR_SUCCESS on success, otherwise error code from the earliest failing API
    //     - common errors:
    //        - ERROR_NO_DATA - incompatible protocol or not a Virtual Machine, but driver is running
    //        - ERROR_ACCESS_DENIED - returns Performance Counters API inside Nano Server container
    //
    DWORD QueryDynamicMemoryMaximum (std::uint64_t * result);
}

#endif
