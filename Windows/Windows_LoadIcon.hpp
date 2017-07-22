#ifndef WINDOWS_LOADICON_HPP
#define WINDOWS_LOADICON_HPP

/* Windows LoadIcon 
// Windows_LoadIcon.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      05.06.2012 - initial version
//      02.05.2016 - added loading of shell-sized icons
*/

#include <windows.h>

namespace Windows {
    
    // LoadIcon
    //  - simplest LoadIcon loads the icon resource as-is without rescalling
    //  - when dimensions are provided, icons are scaled apropiately to OS
    
    HICON LoadIcon (HINSTANCE, PCWSTR);
    HICON LoadIcon (HINSTANCE, USHORT);
    
    HICON LoadIcon (HINSTANCE, PCWSTR, int, int);
    HICON LoadIcon (HINSTANCE, USHORT, int, int);
    
    // Load[Small/Large]Icon
    //  - simple wrappers with GetSystemMetrics (SM_C[X/Y](SM)ICON) calls
    
    HICON LoadSmallIcon (HINSTANCE, PCWSTR);
    HICON LoadSmallIcon (HINSTANCE, USHORT);
    HICON LoadLargeIcon (HINSTANCE, PCWSTR);
    HICON LoadLargeIcon (HINSTANCE, USHORT);
    
    // Get[Shell/Jumbo]IconSize
    //  - returns size of shell and jumbo icons, typically 48×48 adnd 256×256
    
    SIZE GetShellIconSize ();
    SIZE GetJumboIconSize ();
    
    // Load[Shell/Jumbo]SizeIcon
    //  - simple wrappers to LoadIcon passing shell or jumbo icon size
    
    HICON LoadShellSizeIcon (HINSTANCE, PCWSTR);
    HICON LoadShellSizeIcon (HINSTANCE, USHORT);
    HICON LoadJumboSizeIcon (HINSTANCE, PCWSTR);
    HICON LoadJumboSizeIcon (HINSTANCE, USHORT);
};

#endif

