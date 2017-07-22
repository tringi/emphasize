#include "Windows_BlankWndClass.hpp"

/* Emphasize Windows blank window class
// Windows_BlankWndClass.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.0
//
// Changelog:
//      01.08.2011 - initial version
*/

namespace {
    LRESULT CALLBACK Procedure (HWND, UINT, WPARAM, LPARAM);
};

const LPCTSTR Windows::BlankClass::Name = TEXT ("BLANK");

ATOM Windows::BlankClass::Initialize (HMODULE hInstance) {
    static WNDCLASS wc = {
        0, Procedure, 0, 0, NULL,
        NULL, NULL, NULL, NULL, Name
    };
    
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    return RegisterClass (&wc);
};

namespace {
    LRESULT CALLBACK Procedure (HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam) {
        
        if (message == WM_NCCREATE) {
                
            // Windows SDK: WM_NCCREATE
            //  - not the first message, but WM_GETMINMAXINFO can be ignored
            //  - the first call simply replaces the window procedure with
            //    the pointer provided as a LPVOID parameter of CreateWindowEx
            //    and after that it is obviously not called again
            
            CREATESTRUCT * cs = reinterpret_cast <CREATESTRUCT *> (lParam);
            LPVOID         lp = cs->lpCreateParams;
            WNDPROC        procedure = reinterpret_cast <WNDPROC> (lp);
            LONG_PTR       longptr   = reinterpret_cast <LONG_PTR> (lp);
            
            if (procedure) {
                SetLastError (0);
                SetWindowLongPtr (hWnd, GWLP_WNDPROC, longptr);
                
                if (GetLastError () == ERROR_SUCCESS) {
                    return CallWindowProc (procedure, hWnd,
                                           message, wParam, lParam);
                } else
                    return 0;
            };
        };
        return DefWindowProc (hWnd, message, wParam, lParam);
    };
};

