#ifndef WINDOWS_RESETCURRENTDIRECTORY_HPP
#define WINDOWS_RESETCURRENTDIRECTORY_HPP

/* Emphasize Windows current directory service helper
// Windows_ResetCurrentDirectory.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      29.07.2011 - initial version
*/

namespace Windows {
    
    // ResetCurrentDirectory
    //  - sets current directory to directory from where the application
    //    is running
    
    bool ResetCurrentDirectory ();
};

#endif
