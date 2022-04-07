#include "Windows_QueryDynamicMemoryMaximum.hpp"
#include <perflib.h>

// HvDmvcsGuid
//  - "Hyper-V Dynamic Memory Integration Service"
//
static const GUID HvDmvcsGuid = { 0x66f19dff, 0xa4dd, 0x4802, { 0x8f, 0xbb, 0x29, 0xe6, 0xa5, 0x4a, 0xf9, 0xda } };

DWORD Windows::QueryDynamicMemoryMaximum (std::uint64_t * value) {
    DWORD error = ERROR_SUCCESS;
    HANDLE query = NULL;

    if ((error = PerfOpenQueryHandle (NULL, &query)) == ERROR_SUCCESS) {

        PERF_COUNTER_IDENTIFIER counter = {};
        counter.CounterSetGuid = HvDmvcsGuid;
        counter.Size = sizeof counter;
        counter.CounterId = 1; // ID 1 == "Maximum Memory, Mbytes"

        if ((error = PerfAddCounters (query, &counter, sizeof counter)) == ERROR_SUCCESS) {

            struct {
                PERF_DATA_HEADER    header;
                PERF_COUNTER_HEADER counter;
                PERF_COUNTER_DATA   counterdata;
                ULONGLONG           value;
            } data = {};

            DWORD n;
            if ((error = PerfQueryCounterData (query, &data.header, sizeof data, &n)) == ERROR_SUCCESS) {

                switch (data.counter.dwType) {
                    default:
                        error = ERROR_INVALID_DATA;
                        break;
                    case PERF_ERROR_RETURN:
                        error = data.counter.dwStatus;
                        break;

                    case PERF_SINGLE_COUNTER:
                        switch (data.header.dwNumCounters) {
                            default:
                                error = ERROR_MORE_DATA;
                                break;
                            case 0:
                                error = ERROR_NO_DATA;
                                break;
                            
                            case 1:
                                if (n >= (sizeof data - sizeof (DWORD))) { // if published as single DWORD instead of ULONGLONG
                                    if (value) {
                                        *value = data.value * 1048576uLL;
                                    }
                                }
                        }
                }
            }
        }
        PerfCloseQueryHandle (query);
    }
    return error;
}
