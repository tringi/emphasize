#ifndef WINDOWS_UXTHEME_HPP
#define WINDOWS_UXTHEME_HPP

/* Emphasize Windows UxTheme abstraction
// Windows_UxTheme.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: Dynamic abstraction of Windows Theme engines. 
//
// Changelog:
//      31.03.2011 - initial version
*/

#include <windows.h>

namespace Windows {
namespace UxTheme {
    
    // Initialize
    //  - attempts to dynamically load the theme libraries
    //  - until called, this unit will behave as if Classic theme was active
    //  - InitCommonControls needs to be called before to get correct behavior
    
    void Initialize ();
    
    // Type
    //  - Window Manager (renderer) types
    
    enum Type {
        Classic,    // user32:  Windows 2000
        Lite,       // uxtheme: Windows XP Luna, Vista/7 Aero Lite
        Aero,       // ux+dwm:  Windows Vista/7 Aero
//        Metro,      // TODO: Windows 8 Immersive UI
//        Types
    };
    
    // Current
    //  - returns current UxTheme type for the process
    //  - note that even when Lite theme is available, the Windows will return
    //    Classic until the application creates any User Interface elements
    
    Type Current ();
    
    // DefWindowProc
    //  - abstracts calls to DwmDefWindowProc and DefWindowProc for custom
    //    framed windows
    //  - if not NULL, the BOOL* parameter is set to TRUE when DwmDefWindowProc
    //    processed this (typically HITTEST message) otherwise is set to FALSE
    
    LRESULT DefWindowProc (HWND, UINT, WPARAM, LPARAM, BOOL * = NULL);
    
    // ExtendFrame
    //  - abstracts DwmExtendFrameIntoClientArea
    //    and SetWindowCompositionAttribute (ACCENT_ENABLE_BLURBEHIND...)
    //    for full windows extension (-1,-1,-1,-1)
    //  - returns E_NOINTERFACE if this functionality is not available
    
    HRESULT ExtendFrame (HWND, UINT left, UINT top, UINT right, UINT bottom);
    
    // SetNonClientOptions
    //  - abstracts SetWindowThemeAttribute with WTA_NONCLIENT / WTA_OPTIONS
    //  - returns E_NOINTERFACE if this functionality is not available
    
    HRESULT SetNonClientOptions (HWND, DWORD dwFlags, DWORD dwMask);
    
    // SetAppProperties
    //  - abstracts SetThemeAppProperties call
    //  - returns S_OK or E_NOINTERFACE if this functionality is not available
    
    HRESULT SetAppProperties (DWORD dwFlags);
    
    // SetWindowBlur
    //  - abstracts DwmEnableBlurBehindWindow
    //  - returns S_OK or E_NOINTERFACE if this functionality is not available
    
    HRESULT SetWindowBlur (HWND, BOOL, HRGN = NULL);
    
    // GetColorization
    //  - abstracts call to DwmGetColorizationColor
    //  - returns - composition colorization color
    //            - when int* is not NULL, is set to OPAQUE or TRANSPARENT
    //  - returns GetSysColor(COLOR_3DFACE) and OPAQUE when unavailable
    
    COLORREF GetColorization (int * = NULL);
    
    // Open(ThemeData)
    // Close(ThemeData)
    //  - opens theme data if any, returns NULL on error/disabled/win2000
    
    HANDLE Open (HWND, LPCWSTR = L"WINDOW");
    HRESULT Close (HANDLE);
    
    // Get[...]
    // Draw[...]
    //  - dynamicaly loaded rendering functions
    //  - will crash on error/disabled/win2000
    //     - do not call if Open returned NULL or Current returns Classic
    //  - for parameters consult Microsoft Windows SDK
    //  - functions are added as needed
    
    HRESULT GetColor (HANDLE, int, int, int, COLORREF *);
    HRESULT GetFont (HANDLE, HDC, int, int, int, LOGFONTW *);
HRESULT DrawBackground (HANDLE, HDC, int, int, const RECT *, const RECT * = NULL);

    // GetSysFont
    //  - dynamically loaded on XP+, theme HANDLE can be NULL

    HRESULT GetSysFont (HANDLE, int, LOGFONTW *);
    
    // DrawEdge
    //  - calls DrawThemeEdge or DrawEdge appropriately
    
    HRESULT DrawEdge (HANDLE, HDC, int, int, const RECT *, UINT, UINT, RECT * = NULL);
    
    // DrawText
    //  - calls DrawThemeTextEx (with solid alpha on Aero) or DrawTextEx
    //  - glow is supported only on Vista and above (through DrawThemeTextEx)
    
    HRESULT DrawText (HANDLE, HDC, LPCWSTR, int, DWORD, LPRECT,
                      UINT glow = 0);
    
    
    // BeginBufferedPaint
    // EndBufferedPaint
    //  - if Vista/UxTheme equivalents are available, forwards otherwise
    //    just copied input HDC to output HDC*
    
    HANDLE BeginBufferedPaint (HDC, LPCRECT, HDC *, BOOL invert = FALSE);
    HRESULT EndBufferedPaint (HANDLE, BOOL = TRUE);
    HRESULT GetBufferedPaintBits (HANDLE, RGBQUAD **, int *);
    
    // BufferedPaintSetAlpha
    //  - if Vista/UxTheme equivalent is available, forwards, otherwise NO-OP
    
    HRESULT BufferedPaintSetAlpha (HANDLE, LPCRECT, BYTE);
    
};    
};

#endif
