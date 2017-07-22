#include "Windows_UxTheme.hpp"

/* Emphasize Windows UxTheme abstraction
// Windows_UxTheme.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      31.03.2011 - initial version
//      26.05.2011 - IsThemeActive removed as redundant
//      28.04.2013 - DwmDefWindowProc not called under Wine
*/

#include "Windows_Symbol.hpp"
#include "Windows_Api.hpp"

#include <windows.h>
#include <shlwapi.h>
#include <uxtheme.h>
#if (_WIN32_WINNT >= 0x0600)
#include <dwmapi.h>
#endif

#include <cstring>
#include <cstdio>

namespace {
    
    // API calls and their default counterparts

    struct WINCOMPATTRDATA {
        DWORD attribute; // the attribute to query, see below
        PVOID pData; // buffer to store the result
        ULONG dataSize; // size of the pData buffer
    };
    struct ACCENTPOLICY {
        DWORD AccentState;
        DWORD AccentFlags;
        DWORD GradientColor;
        DWORD AnimationId;
    };

//#if (_WIN32_WINNT < 0x0A00)
//    HRESULT  WINAPI   SetWindowCompositionAttributeDefault (BOOL *);
    BOOL (WINAPI * SetWindowCompositionAttribute) (HWND, WINCOMPATTRDATA *) = NULL;
//#endif
    
#if (_WIN32_WINNT < 0x0600)
    HRESULT  WINAPI   DwmIsCompositionEnabledDefault (BOOL *);
    HRESULT (WINAPI * DwmIsCompositionEnabled) (BOOL *) = DwmIsCompositionEnabledDefault;
    HRESULT (WINAPI * SetWindowThemeAttribute) (HWND, DWORD, PVOID, DWORD) = NULL;
#endif

#if (_WIN32_WINNT < 0x0501)
    typedef struct _MARGINS {
    	int cxLeftWidth;
    	int cxRightWidth;
    	int cyTopHeight;
    	int cyBottomHeight;
    } MARGINS, *PMARGINS; 
#endif

#if (_WIN32_WINNT < 0x0600)
    typedef struct _DWM_BLURBEHIND {
        DWORD dwFlags;
        BOOL  fEnable;
        HRGN  hRgnBlur;
        BOOL  fTransitionOnMaximized;
    } DWM_BLURBEHIND;
    
    BOOL    (WINAPI * DwmDefWindowProc) (HWND, UINT, WPARAM, LPARAM, LRESULT *) = NULL;
    HRESULT (WINAPI * DwmExtendFrameIntoClientArea) (HWND, const MARGINS *) = NULL;
    HRESULT (WINAPI * SetWindowThemeAttribute) (HWND, DWORD, PVOID, DWORD) = NULL;
    HRESULT (WINAPI * DwmGetColorizationColor) (DWORD *, BOOL *) = NULL;
    HRESULT (WINAPI * DwmEnableBlurBehindWindow) (HWND, const DWM_BLURBEHIND *) = NULL;
#endif
    
#if (_WIN32_WINNT < 0x0501)
    BOOL  WINAPI   IsAppThemedDefault ();
    BOOL (WINAPI * IsAppThemed) () = IsAppThemedDefault;
    
    void (WINAPI * SetThemeAppProperties) (DWORD) = NULL;
    
    HANDLE WINAPI OpenThemeDataDefault (HWND, LPCWSTR);
    HANDLE (WINAPI * OpenThemeData) (HWND, LPCWSTR) = OpenThemeDataDefault;
    HRESULT (WINAPI * CloseThemeData) (HANDLE) = NULL;
    HRESULT (WINAPI * GetThemeColor) (HANDLE, int, int, int, COLORREF *) = NULL;
    HRESULT (WINAPI * GetThemeFont) (HANDLE, HDC, int, int, int, LOGFONTW *) = NULL;
    HRESULT (WINAPI * GetThemeSysFont) (HANDLE, int, LOGFONTW *) = NULL;
    HRESULT (WINAPI * DrawThemeBackground) (HANDLE, HDC, int, int,
                                            const RECT *, const RECT *) = NULL;

    HRESULT  WINAPI   DrawThemeEdgeDefault (HANDLE, HDC, int, int, const RECT *,
                                            UINT, UINT, RECT *);
    HRESULT (WINAPI * DrawThemeEdge) (HANDLE, HDC, int, int, const RECT *,
                                      UINT, UINT, RECT *) = DrawThemeEdgeDefault;
#endif

#if (_WIN32_WINNT < 0x0600)
#ifndef DTT_TEXTCOLOR
    typedef struct _DTTOPTS {
        DWORD dwSize;
        DWORD dwFlags;
        COLORREF crText;
        COLORREF crBorder;
        COLORREF crShadow;
        int iTextShadowType;
        POINT ptShadowOffset;
        int iBorderSize;
        int iFontPropId;
        int iColorPropId;
        int iStateId;
        BOOL fApplyOverlay;
        int iGlowSize;
        void * /*DTT_CALLBACK_PROC*/ pfnDrawTextCallback;
        LPARAM lParam;
    } DTTOPTS, *PDTTOPTS;
    
    #define DTT_TEXTCOLOR       (1UL << 0)      // crText has been specified
    #define DTT_BORDERCOLOR     (1UL << 1)      // crBorder has been specified
    #define DTT_SHADOWCOLOR     (1UL << 2)      // crShadow has been specified
    #define DTT_SHADOWTYPE      (1UL << 3)      // iTextShadowType has been specified
    #define DTT_SHADOWOFFSET    (1UL << 4)      // ptShadowOffset has been specified
    #define DTT_BORDERSIZE      (1UL << 5)      // iBorderSize has been specified
    #define DTT_FONTPROP        (1UL << 6)      // iFontPropId has been specified
    #define DTT_COLORPROP       (1UL << 7)      // iColorPropId has been specified
    #define DTT_STATEID         (1UL << 8)      // IStateId has been specified
    #define DTT_CALCRECT        (1UL << 9)      // Use pRect as and in/out parameter
    #define DTT_APPLYOVERLAY    (1UL << 10)     // fApplyOverlay has been specified
    #define DTT_GLOWSIZE        (1UL << 11)     // iGlowSize has been specified
    #define DTT_CALLBACK        (1UL << 12)     // pfnDrawTextCallback has been specified
    #define DTT_COMPOSITED      (1UL << 13)     // Draws text with antialiased alpha (needs a DIB section)
#endif
    
    enum TEXTSHADOWTYPE {
    	TST_NONE = 0,
    	TST_SINGLE = 1,
    	TST_CONTINUOUS = 2,
    };
    
    HRESULT (WINAPI * DrawThemeTextEx) (HANDLE, HDC, int, int, LPCWSTR, int,
                                        DWORD, LPRECT, const DTTOPTS *) = NULL;


    typedef enum _BP_BUFFERFORMAT {
        BPBF_COMPATIBLEBITMAP,    // Compatible bitmap
        BPBF_DIB,                 // Device-independent bitmap
        BPBF_TOPDOWNDIB,          // Top-down device-independent bitmap
        BPBF_TOPDOWNMONODIB       // Top-down monochrome device-independent bitmap
    } BP_BUFFERFORMAT;

    typedef struct _BP_PAINTPARAMS {
        DWORD                       cbSize;
        DWORD                       dwFlags; // BPPF_ flags
        const RECT *                prcExclude;
        const BLENDFUNCTION *       pBlendFunction;
    } BP_PAINTPARAMS, *PBP_PAINTPARAMS;
    
    HRESULT (WINAPI * BufferedPaintInit) () = NULL;

    HANDLE WINAPI BeginBufferedPaintDefault (HDC, LPCRECT, BP_BUFFERFORMAT,
                                             BP_PAINTPARAMS *, HDC *);
    HRESULT WINAPI EndBufferedPaintDefault (HANDLE, BOOL);
    HRESULT WINAPI BufferedPaintSetAlphaDefault (HANDLE, LPCRECT, BYTE);
    HRESULT WINAPI GetBufferedPaintBitsDefault (HANDLE, RGBQUAD **, int *);

    HANDLE  (WINAPI * BeginBufferedPaintCall) (HDC, LPCRECT, BP_BUFFERFORMAT,
                                               BP_PAINTPARAMS *, HDC *)
                    = BeginBufferedPaintDefault;
    HRESULT (WINAPI * EndBufferedPaintCall) (HANDLE, BOOL)
                    = EndBufferedPaintDefault;
    HRESULT (WINAPI * BufferedPaintSetAlphaCall) (HANDLE, LPCRECT, BYTE)
                    = BufferedPaintSetAlphaDefault;
    HRESULT (WINAPI * GetBufferedPaintBitsCall) (HANDLE, RGBQUAD **, int *)
                    = GetBufferedPaintBitsDefault;
#endif
};

void Windows::UxTheme::Initialize () {
    Symbol (L"USER32", SetWindowCompositionAttribute, "SetWindowCompositionAttribute");
    
    if (HMODULE hUxTheme = LoadLibraryW (L"UXTHEME")) {
#if (_WIN32_WINNT < 0x0600)
        Symbol (hUxTheme, SetWindowThemeAttribute, "SetWindowThemeAttribute");
#endif
#if (_WIN32_WINNT < 0x0501)
        Symbol (hUxTheme, IsAppThemed, "IsAppThemed");
        Symbol (hUxTheme, SetThemeAppProperties, "SetThemeAppProperties");
        
        Symbol (hUxTheme, OpenThemeData, "OpenThemeData");
        Symbol (hUxTheme, CloseThemeData, "CloseThemeData");
        Symbol (hUxTheme, GetThemeColor, "GetThemeColor");
        Symbol (hUxTheme, GetThemeFont, "GetThemeFont");
        Symbol (hUxTheme, GetThemeSysFont, "GetThemeSysFont");
        Symbol (hUxTheme, DrawThemeBackground, "DrawThemeBackground");
        Symbol (hUxTheme, DrawThemeEdge, "DrawThemeEdge");
#endif
#if (_WIN32_WINNT < 0x0600)
        Symbol (hUxTheme, DrawThemeTextEx, "DrawThemeTextEx");
        Symbol (hUxTheme, BufferedPaintInit, "BufferedPaintInit");
        Symbol (hUxTheme, BeginBufferedPaintCall, "BeginBufferedPaint");
        Symbol (hUxTheme, EndBufferedPaintCall, "EndBufferedPaint");
        Symbol (hUxTheme, GetBufferedPaintBitsCall, "GetBufferedPaintBits");
        Symbol (hUxTheme, BufferedPaintSetAlphaCall, "BufferedPaintSetAlpha");
#endif
    };
#if (_WIN32_WINNT < 0x0600)
    if (Api != Api::Wine) {
        if (HMODULE hDwmApi = LoadLibraryW (L"DWMAPI")) {
            Symbol (hDwmApi, DwmDefWindowProc, "DwmDefWindowProc");
            Symbol (hDwmApi, DwmIsCompositionEnabled, "DwmIsCompositionEnabled");
            Symbol (hDwmApi, DwmGetColorizationColor, "DwmGetColorizationColor");
            Symbol (hDwmApi, DwmExtendFrameIntoClientArea, "DwmExtendFrameIntoClientArea");
            Symbol (hDwmApi, DwmEnableBlurBehindWindow, "DwmEnableBlurBehindWindow");
        };
    };
#endif
    
    // on following unlikely errors, revert functionality to basic defaults
    
#if (_WIN32_WINNT < 0x0501)
    if (HMODULE hComCtl = GetModuleHandleW (L"COMCTL32")) {
        BOOL revert = TRUE;
        
        // TODO: is this actually neccessary?
        HRESULT (CALLBACK * ComCtlDllGetVersion) (DLLVERSIONINFO *) = NULL;
        if (Symbol (hComCtl, ComCtlDllGetVersion, "DllGetVersion")) {
            DLLVERSIONINFO vi = { sizeof (DLLVERSIONINFO), 0,0,0,0 };
            if (ComCtlDllGetVersion (&vi) == S_OK) {
                if (vi.dwMajorVersion >= 6u) {
                    revert = FALSE;
                };
            };
        };
        if (revert) {
            IsAppThemed = IsAppThemedDefault;
        };
    };
#endif
#if (_WIN32_WINNT < 0x0600)
    if (BufferedPaintInit) {
        if (BufferedPaintInit () != S_OK) {
            BeginBufferedPaintCall = BeginBufferedPaintDefault;
            EndBufferedPaintCall = EndBufferedPaintDefault;
            GetBufferedPaintBitsCall = GetBufferedPaintBitsDefault;
            BufferedPaintSetAlphaCall = BufferedPaintSetAlphaDefault;
        };
    };
#endif    
    return;
};

Windows::UxTheme::Type Windows::UxTheme::Current () {
    if (IsAppThemed ()) {
        BOOL bDwmEnabled = FALSE;
        if (DwmIsCompositionEnabled (&bDwmEnabled) == S_OK) {
            if (bDwmEnabled) {
                return Windows::UxTheme::Aero;
            };
        };
        return Windows::UxTheme::Lite;
    } else
        return Windows::UxTheme::Classic;
};

LRESULT Windows::UxTheme::DefWindowProc (HWND hWnd, UINT message,
                                         WPARAM wParam, LPARAM lParam,
                                         BOOL * bFromDWM) {
    if (DwmDefWindowProc) {
        LRESULT result = 0;
        if (DwmDefWindowProc (hWnd, message, wParam, lParam, &result)) {
            
            if (bFromDWM) {
                *bFromDWM = TRUE;
            };
            return result;
        };
    };
    
    if (bFromDWM) {
        *bFromDWM = FALSE;
    };
    return ::DefWindowProc (hWnd, message, wParam, lParam);
};

HRESULT Windows::UxTheme::ExtendFrame (HWND hWnd,
                                       UINT left, UINT top,
                                       UINT right, UINT bottom) {
    if (SetWindowCompositionAttribute) {
        ACCENTPOLICY policy = { 0, 0, 0, 0 };
        if (left == -1u && top == -1u && right == -1u && bottom == -1u) {
            policy.AccentState = 3;
        };
        WINCOMPATTRDATA data = { 19, &policy, sizeof policy };
        SetWindowCompositionAttribute (hWnd, &data);
    };

    if (DwmExtendFrameIntoClientArea) {
        MARGINS margins = {
            (int) left,
            (int) right,
            (int) top,
            (int) bottom
        };
        return DwmExtendFrameIntoClientArea (hWnd, &margins);
    } else
        return E_NOINTERFACE;
};

HRESULT Windows::UxTheme::SetAppProperties (DWORD dwFlags) {
#if (_WIN32_WINNT < 0x0501)
    if (SetThemeAppProperties) {
#endif
        SetThemeAppProperties (dwFlags);
        return S_OK;
#if (_WIN32_WINNT < 0x0501)
    } else
        return E_NOINTERFACE;
#endif
};

HRESULT Windows::UxTheme::SetNonClientOptions (HWND hWnd,
                                               DWORD dwFlags, DWORD dwMask) {
    if (SetWindowThemeAttribute) {
        typedef struct WTA_OPTIONS {
            DWORD dwFlags;
            DWORD dwMask;
        } WTA_OPTIONS;
        
        WTA_OPTIONS wto = { dwFlags, dwMask };
#if (_WIN32_WINNT < 0x0600)
        return SetWindowThemeAttribute (hWnd, 1, &wto, sizeof wto);
#else
        return SetWindowThemeAttribute (hWnd, WTA_NONCLIENT, &wto, sizeof wto);
#endif
    } else

        return E_NOINTERFACE;
};

HRESULT Windows::UxTheme::SetWindowBlur (HWND hWnd, BOOL enabled, HRGN hRgn) {
    DWM_BLURBEHIND bb = { 1, enabled, hRgn, FALSE };
    
#if (_WIN32_WINNT < 0x0600)
    if (DwmEnableBlurBehindWindow) {
#endif
        return DwmEnableBlurBehindWindow (hWnd, &bb);
#if (_WIN32_WINNT < 0x0600)
    } else
        return E_NOINTERFACE;
#endif
};

COLORREF Windows::UxTheme::GetColorization (int * mode) {
    DWORD color;
    BOOL opaque;
    
    if (DwmGetColorizationColor
            && SUCCEEDED (DwmGetColorizationColor (&color, &opaque))) {
        
        //  opaque  | mode
        // (0)FALSE | (1)TRANSPARENT
        // (1)TRUE  | (2)OPAQUE
        
        if (mode) {
            *mode = opaque + 1;
        };
        
        // DWORD clr: 0xAARRGGBB
        // COLORREF: 0x00BBGGRR 

        return RGB (GetBValue (color), GetGValue (color), GetRValue (color));
    } else {
        if (mode)
            *mode = OPAQUE;
        
        return GetSysColor (COLOR_3DFACE);
    };
};

HANDLE Windows::UxTheme::Open (HWND hWnd, LPCWSTR szClasses) {
    return OpenThemeData (hWnd, szClasses);
};
HRESULT Windows::UxTheme::Close (HANDLE hTheme) {
    return CloseThemeData (hTheme);
};
HRESULT Windows::UxTheme::GetColor (HANDLE hTheme,
                                    int iPartId, int iStateId, int iPropId,
                                    COLORREF * color) {
    return GetThemeColor (hTheme, iPartId, iStateId, iPropId, color);
};
HRESULT Windows::UxTheme::GetFont (HANDLE hTheme, HDC hDC,
                                   int iPartId, int iStateId, int iPropId,
                                   LOGFONTW * plf) {
    return GetThemeFont (hTheme, hDC, iPartId, iStateId, iPropId, plf);
};

HRESULT Windows::UxTheme::GetSysFont (HANDLE hTheme, int id, LOGFONTW * plf) {
#if (_WIN32_WINNT < 0x0501)
    if (!GetThemeSysFont) {
        if (id == 806) { // TMT_ICONTITLEFONT
            
            if (SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof *plf, plf, 0u)) {
                return S_OK;
            } else {
                return GetLastError ();
            };
            
        } else {
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof ncm;
            
            if (SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0u)) {
                switch (id) {
                    case 801: *plf = ncm.lfCaptionFont; break;
                    case 802: *plf = ncm.lfSmCaptionFont; break;
                    case 803: *plf = ncm.lfMenuFont; break;
                    case 804: *plf = ncm.lfStatusFont; break;
                    case 805: *plf = ncm.lfMessageFont; break;
                    
                    default:
                        return E_INVALIDARG;
                };
                
                return S_OK;
            } else {
                return GetLastError ();
            };
        };
    } else
#endif
        return GetThemeSysFont (hTheme, id, plf);
};


HRESULT Windows::UxTheme::DrawBackground (HANDLE hTheme, HDC hDC,
                                          int iPartId, int iStateId,
                                          const RECT * r, const RECT * clip) {
    return DrawThemeBackground (hTheme, hDC, iPartId, iStateId, r, clip);
};
HRESULT Windows::UxTheme::DrawEdge (HANDLE hTheme, HDC hDC, 
                                    int iPartId, int iStateId,
                                    const RECT * r, UINT edge, UINT flags,
                                    RECT * adjusted) {
    return DrawThemeEdge (hTheme, hDC, iPartId, iStateId,
                          r, edge, flags, adjusted);
};
HRESULT Windows::UxTheme::DrawText (HANDLE hTheme, HDC hDC, 
                                    LPCWSTR szString, int iLength,
                                    DWORD dwDTFormat, LPRECT rc,
                                    UINT glow) {
    
    if (DrawThemeTextEx && hTheme) {
        
        HRESULT result = -1;
        if (HDC hMemoryDC = CreateCompatibleDC (hDC)) {
        
            BITMAPINFO info;
            std::memset (&info, 0, sizeof info);
            
            info.bmiHeader.biSize = sizeof info;
            info.bmiHeader.biWidth = 2 * glow + rc->right - rc->left;
            info.bmiHeader.biHeight = -(LONG) (2 * glow + rc->bottom - rc->top);
            info.bmiHeader.biPlanes = 1;
            info.bmiHeader.biBitCount = 32;
            info.bmiHeader.biCompression = BI_RGB;
            
            if (HBITMAP dib = CreateDIBSection (hDC, &info, DIB_RGB_COLORS,
                                                NULL, NULL, 0u)) {

                HFONT hFont = (HFONT) SelectObject (hDC, GetStockObject (DEFAULT_GUI_FONT));
                
                SelectObject (hMemoryDC, dib);
                SelectObject (hMemoryDC, hFont);

                RECT bound = { (LONG) glow, (LONG) glow,
                               (LONG) glow + rc->right - rc->left,
                               (LONG) glow + rc->bottom - rc->top };
                
                if (GetBkMode (hDC) == TRANSPARENT) {
                    BitBlt (hMemoryDC,
                            0, 0, bound.right + glow, bound.bottom + glow,
                            hDC, rc->left - glow, rc->top - glow, SRCCOPY);
                };

                DTTOPTS options;
                options.dwSize = sizeof options;
                options.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
                options.crText = GetTextColor (hDC);
                
                if (glow) {
                    options.dwFlags |= DTT_GLOWSIZE;
                    options.iGlowSize = glow;
                };
                if (dwDTFormat & DT_CALCRECT) {
                    options.dwFlags |= DTT_CALCRECT;
                };

                result = DrawThemeTextEx (hTheme, hMemoryDC, 0, 0,
                                          szString, iLength, dwDTFormat,
                                          &bound, &options);
                
                if (result == S_OK) {
                    if (dwDTFormat & DT_CALCRECT) {
                        rc->right = rc->left + bound.right;
                        rc->bottom = rc->top + bound.bottom;
                    };
                    
                    BitBlt (hDC, rc->left - glow, rc->top - glow,
                            bound.right + glow, bound.bottom + glow,
                            hMemoryDC, 0, 0, SRCCOPY);
                };
                DeleteObject (dib);
                SelectObject (hDC, hFont);
            };
            DeleteDC (hMemoryDC);
        };
        
        if (result != -1)
            return result;
        else
            return GetLastError ();
        
    } else {
        
        dwDTFormat &= ~DT_MODIFYSTRING;
        if (DrawTextEx (hDC, const_cast <LPWSTR> (szString), iLength, 
                        rc, dwDTFormat, NULL))
            return S_OK;
        else
            return GetLastError ();
    };
};

#if (_WIN32_WINNT < 0x0600)
HANDLE Windows::UxTheme::BeginBufferedPaint (HDC hDC, LPCRECT rc,
                                             HDC * hOut, BOOL inverted) {
    return BeginBufferedPaintCall (hDC, rc,
                                   inverted ? BPBF_DIB : BPBF_TOPDOWNDIB,
                                   NULL, hOut);
};
HRESULT Windows::UxTheme::EndBufferedPaint (HANDLE handle, BOOL flag) {
    return EndBufferedPaintCall (handle, flag);
};
HRESULT Windows::UxTheme::GetBufferedPaintBits (HANDLE h, RGBQUAD ** p, int * w) {
    return GetBufferedPaintBitsCall (h, p, w);
};

HRESULT Windows::UxTheme::BufferedPaintSetAlpha (HANDLE handle,
                                                 LPCRECT r, BYTE alpha) {
    return BufferedPaintSetAlphaCall (handle, r, alpha);
};
#endif

// Default functions
//  - fall-back implementations when the call is not supported by the system

namespace {
    HRESULT WINAPI DwmIsCompositionEnabledDefault (BOOL * pfEnabled) {
        if (pfEnabled) {
            *pfEnabled = FALSE;
        };
        return S_OK;
    };

#if (_WIN32_WINNT < 0x0501)
    BOOL WINAPI IsAppThemedDefault () {
        return FALSE;
    };

    HANDLE WINAPI OpenThemeDataDefault (HWND, LPCWSTR) {
        return NULL;
    };
#endif

    HANDLE WINAPI BeginBufferedPaintDefault (HDC hDC, LPCRECT, BP_BUFFERFORMAT,
                                             BP_PAINTPARAMS *, HDC * hOut) {
        *hOut = hDC;
        // TODO: Implement own simple buffering for 2k and XP?
        return INVALID_HANDLE_VALUE;
    };
    HRESULT WINAPI EndBufferedPaintDefault (HANDLE, BOOL) {
        return S_OK;
    };
    HRESULT WINAPI BufferedPaintSetAlphaDefault (HANDLE h, LPCRECT, BYTE a) {
        if (h == INVALID_HANDLE_VALUE && a == 255)
            return S_OK;
        else
            return E_NOINTERFACE;
    };
    HRESULT WINAPI GetBufferedPaintBitsDefault (HANDLE, RGBQUAD **, int *) {
        return E_NOINTERFACE;
    };

#if (_WIN32_WINNT < 0x0501)
    HRESULT WINAPI DrawThemeEdgeDefault (HANDLE, HDC hDC, int, int,
                                         const RECT * _r, UINT edge, UINT flags,
                                         RECT * adjusted) {
        RECT r = *_r;
        if (::DrawEdge (hDC, &r, edge, flags)) {
            if (adjusted && (flags & BF_ADJUST)) {
                *adjusted = r;
            };
            return S_OK;
        } else
            return GetLastError ();
    };
#endif
};

