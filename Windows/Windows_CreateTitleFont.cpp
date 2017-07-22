#include "Windows_CreateTitleFont.hpp"

/* Windows CreateTitleFont 
// Windows_CreateTitleFont.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      07.06.2012 - initial version
*/

#include "Windows_UxTheme.hpp"
#include <cwchar>
#include <cstdio>

HFONT Windows::CreateTitleFont (HWND hWnd, COLORREF * cr) {
    LOGFONT font;
    HANDLE hTheme = Windows::UxTheme::Open (hWnd, L"TEXTSTYLE");
    
    if (cr) {
        if (hTheme == NULL
            || Windows::UxTheme::GetColor (hTheme, 1,1,0x0EDB, cr) != S_OK) {
            
            // Fallback to normal text color
            //  - if no theme is active
            //  - or if GetThemeColor fails
            
            *cr = GetSysColor (COLOR_WINDOWTEXT);
        };
    };
    
    if (hTheme == NULL
        || Windows::UxTheme::GetFont (hTheme, NULL, 1,0,210, &font) != S_OK) {

        // Fallback to normal text font
        //  - if no theme is active
        //  - or GetThemeFont fails
        
        if (GetObject (GetStockObject (DEFAULT_GUI_FONT), sizeof font, &font)) {

            // Using almost default GUI font
            //  - promoted to Trebuchet MS
            //  - and scaled up by 4pts

            std::wcsncpy (font.lfFaceName, L"Trebuchet MS", LF_FACESIZE);
            if (font.lfHeight > 0)
                font.lfHeight += 5;
            else
                font.lfHeight -= 5;
        };
    };

    if (hTheme)
        Windows::UxTheme::Close (hTheme);
    
    return CreateFontIndirect (&font);
};

