#ifndef WINDOWS_ENVIRONMENTVARIABLES_HPP
#define WINDOWS_ENVIRONMENTVARIABLES_HPP

/* Emphasize Windows Environment Variables helper functions
// Windows_EnvironmentVariables.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      08.05.2011 - initial version
*/

#include <windows.h>

namespace Windows {
    
    // CompareEnvironmentVariable
    //  - returns TRUE if environment variable lpName equals to lpValue
    
    bool CompareEnvironmentVariable (LPCTSTR lpName, LPCTSTR lpValue);
    
    // GetEnvironmentVariableUint
    //  - parses the environment variable as unsigned number and returns the
    //    value or zero on error
    
    UINT GetEnvironmentVariableUint (LPCTSTR lpName);
    
};

#endif
