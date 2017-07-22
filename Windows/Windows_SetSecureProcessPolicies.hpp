#ifndef WINDOWS_SETSECUREPROCESSPOLICIES_HPP
#define WINDOWS_SETSECUREPROCESSPOLICIES_HPP

/* Windows SetSecureProcessPolicies 
// Windows_SetSecureProcessPolicies.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      12.03.2013 - initial version
//      31.03.2013 - added HeapEnableTerminationOnCorruption
//      03.04.2014 - added activation of low-fragmentation heap
//      24.11.2014 - added RegDisablePredefinedCache(Ex)
*/

namespace Windows {
    
    // SetSecureProcessPolicies
    //  - call following sequence (skips function unsupported by current OS):
    //     1) SetErrorMode (SEM_FAILCRITICALERRORS);
    //     2) SetProcessDEPPolicy (PROCESS_DEP_ENABLE
    //                             | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION);
    //     3) SetDllDirectoryW (L"");
    //     4) SetSearchPathMode (BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE
    //                           | BASE_SEARCH_PATH_PERMANENT);
    //     5) RegDisablePredefinedCache(Ex)
    //     6) HeapSetInformation (HeapCompatibilityInformation) to enable LFH
    //     7) HeapSetInformation (HeapEnableTerminationOnCorruption)
    //  - returns true if all supported (by the current system) policies were
    //    successfully installed, false otherwise (call GetLastError)
    
    bool SetSecureProcessPolicies ();
    
};

#endif

