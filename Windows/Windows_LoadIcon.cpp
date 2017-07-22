#include "Windows_LoadIcon.hpp"

/* Windows LoadIcon 
// Windows_LoadIcon.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 2.0
//
// Changelog:
//      05.06.2012 - initial version
//      02.05.2016 - added loading of shell-sized icons
*/

#include "Windows_Symbol.hpp"

#include <VersionHelpers.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdio.h>

HICON Windows::LoadIcon (HINSTANCE hInstance, PCWSTR name) {
    return (HICON)::LoadImage (hInstance, name,
                               IMAGE_ICON, 0,0, LR_DEFAULTCOLOR);
};
HICON Windows::LoadIcon (HINSTANCE hInstance, USHORT name) {
    return (HICON)::LoadImage (hInstance, MAKEINTRESOURCE (name),
                               IMAGE_ICON, 0,0, LR_DEFAULTCOLOR);
};
HICON Windows::LoadIcon (HINSTANCE hInstance, USHORT name, int cx, int cy) {
    return Windows::LoadIcon (hInstance, MAKEINTRESOURCE (name), cx, cy);
};

HICON Windows::LoadIcon (HINSTANCE hInstance, PCWSTR name, int cx, int cy) {
    HICON hIcon;
    HRESULT (WINAPI * LoadIconWithScaleDown) (HINSTANCE, PCWSTR, int, int, HICON *);
    
    if (Windows::Symbol (L"COMCTL32",
                         LoadIconWithScaleDown, "LoadIconWithScaleDown")) {
        if (LoadIconWithScaleDown (hInstance, name, cx, cy, &hIcon) == S_OK)
            return hIcon;
    };
    
    return (HICON)::LoadImage (hInstance, name,
                               IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
};

HICON Windows::LoadSmallIcon (HINSTANCE hInstance, USHORT name) {
    return Windows::LoadSmallIcon (hInstance, MAKEINTRESOURCE (name));
};
HICON Windows::LoadLargeIcon (HINSTANCE hInstance, USHORT name) {
    return Windows::LoadLargeIcon (hInstance, MAKEINTRESOURCE (name));
};

HICON Windows::LoadSmallIcon (HINSTANCE hInstance, PCWSTR name) {
    return Windows::LoadIcon (hInstance, name,
                              GetSystemMetrics (SM_CXSMICON),
                              GetSystemMetrics (SM_CYSMICON));
};
HICON Windows::LoadLargeIcon (HINSTANCE hInstance, PCWSTR name) {
    return Windows::LoadIcon (hInstance, name,
                              GetSystemMetrics (SM_CXICON),
                              GetSystemMetrics (SM_CYICON));
};

namespace {
    SIZE GetShellOrJumboIconSize (UINT type) {
#if WINVER < 0x0600
        HRESULT WINAPI (* SHGetImageList) (int, const GUID &, void**) = NULL; 

        if (Windows::Version < Windows::Version::WindowsVista) {
            Windows::Symbol (L"SHELL32", SHGetImageList, 727);
        } else {
            Windows::Symbol (L"SHELL32", SHGetImageList, "SHGetImageList");
        };

        if (SHGetImageList) {
#endif

        HIMAGELIST list;
        static const GUID guid = { 0x46EB5926L,0x582E,0x4017, {0x9F,0xDF,0xE8,0x99,0x8D,0xAA,0x09,0x50}};
        
        if (SHGetImageList (type, guid, (void **) &list) == S_OK) {
            int cx;
            int cy;
            
            if (ImageList_GetIconSize (list, &cx, &cy)) {
                return { cx, cy };
            };
        };

#if WINVER < 0x0600
        };
#endif

        // on error or missing function

        if (type == 4) {
            // SHIL_JUMBO
            return { 256, 256 };
        } else {
            // SHIL_EXTRALARGE
            if (IsWindowsXPOrGreater ()) {
                return { 48, 48 };
            } else {
                return { GetSystemMetrics (SM_CXICON), GetSystemMetrics (SM_CYICON) };
            };
        };
    };
};

SIZE Windows::GetShellIconSize () {
    return GetShellOrJumboIconSize (2); // SHIL_EXTRALARGE
};
SIZE Windows::GetJumboIconSize () {
    return GetShellOrJumboIconSize (4); // SHIL_JUMBO
};
HICON Windows::LoadShellSizeIcon (HINSTANCE hInstance, USHORT name) {
    return Windows::LoadShellSizeIcon (hInstance, MAKEINTRESOURCE (name));
};
HICON Windows::LoadJumboSizeIcon (HINSTANCE hInstance, USHORT name) {
    return Windows::LoadJumboSizeIcon (hInstance, MAKEINTRESOURCE (name));
};

HICON Windows::LoadShellSizeIcon (HINSTANCE hInstance, PCWSTR name) {
    const auto size = Windows::GetShellIconSize ();
    return Windows::LoadIcon (hInstance, name, size.cx, size.cy);
};
HICON Windows::LoadJumboSizeIcon (HINSTANCE hInstance, PCWSTR name) {
    const auto size = Windows::GetJumboIconSize ();
    return Windows::LoadIcon (hInstance, name, size.cx, size.cy);
};

