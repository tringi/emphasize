#ifndef SHADOW_TABCONTROL_HPP
#define SHADOW_TABCONTROL_HPP

/* Emphasize Shadow Controls Library - TabControl
// Shadow_TabControl.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// Description: 
//
// Changelog:
//      04.04.2011 - initial version
*/

#include <windows.h>
#include <commctrl.h>

namespace Shadow {
namespace TabControl {
    
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates the TabControl window
    //  - note: WM_NOTIFY/NM_CUSTOMDRAW/CDDS_PREPAINT should be handled to
    //    custom-erase background of the control
    
    HWND Create (HINSTANCE, HWND, UINT, UINT);

    // WM_COMMAND is sent when current tab changes
    //  - HIWORD (wParam) == new item index!
    //  - not sent when tab moved to different index but is it still the same
    // WM_NOTIFY being sent are:
    //  - NM_CLICK, NM_DBLCLK, NM_RCLICK and NM_RDBLCLK with NMMOUSE
    //     - on NM_CLICK the application can return TRUE to suppress activation
    //     - NMMOUSE.dwItemSpec is tab index
    //     - NMMOUSE.dwItemData is pointer to tab RECT
    //  - NM_CUSTOMDRAW
    //     - handles CDRF_SKIPDEFAULT, CDRF_NOTIFYITEMDRAW, CDRF_NOTIFYPOSTPAINT
    //     - NMCUSTOMDRAW.rc is bounding rectangle of
    //        - the tab (without blank space above or below) for CDDS_ITEM?ERASE
    //        - the text inside the tab for CDDS_ITEM?PAINT
    //     - NMCUSTOMDRAW.dwItemSpec is item index (0 ... 65534)
    //     - NMCUSTOMDRAW.uItemState can be zero, or one or more these flags:
    //        - CDIS_SELECTED, CDIS_HOT, ItemType::SubItem
    //     - NMCUSTOMDRAW.lItemlParam
    //        - on CDDS_ITEMPOSTPAINT is RECT* to full tab rectangle (???)
    //  - NM_CHAR, with NMCHAR (redefined below if missing)
    //  - NM_KEYDOWN, with NMKEY (redefined below if missing)
    //     - allows VK_LEFT and VK_RIGHT to be suppressed

    // TODO:
    //  - pridat Drag and Drop (premistovani zalozek)
    //  - otestovat Inserted, Removed (nebo odebrat, nebudou-li treba)
    //  - sipkove klavesy vlevo a vpravo
    //      - CTRL - Left/Right misto Prev/Next)
    //      - SHIFT - Moving tab or tabpack
    //  - mezernikem prepinat subitemy

    namespace Style {
        static const UINT Auto      = 0x0000u;
        static const UINT Classic   = 0x0001u; // flat, Win95 look
        static const UINT Frame     = 0x0002u; // theme-filled, custom frame
        static const UINT Theme     = 0x0003u; // theme-rendered fully
        static const UINT Custom    = 0x0007u; // custom frame, Vista-like fill
        
        static const UINT Separated = 0x0010u; // last subitems right padded
//        static const UINT Buttons   = 0x0020u; // automatic scroll buttons
        static const UINT RewindBtn = 0x0040u; // display rewind button if shifted
    };

    // Proper sequence for updating data is:
    //  1] Update the data set
    //  2] Send 'Update', 'Inserted', 'Removed' or 'Moved' message
    //  3] Invalidate window for it to repaint

    namespace Message {
        static const UINT Update    = WM_USER;
        static const UINT Inserted  = WM_USER + 0x01; // wParam = index of inserted
        static const UINT Removed   = WM_USER + 0x02; // wParam = index of removed

        static const UINT Left      = WM_USER + 0x0A;
        static const UINT Right     = WM_USER + 0x0B;
        static const UINT Next      = WM_USER + 0x0C;
        static const UINT Previous  = WM_USER + 0x0D;
        
        static const UINT GetCurSel     = WM_USER + 0x10; // returns current tab
        static const UINT GetFullWidth  = WM_USER + 0x11; // returns sum of tab widths
        static const UINT SetActiveDims = WM_USER + 0x14; // 
    };
    namespace Request { 
        static const UINT Size = 1;
        static const UINT Text = 2; // LOWORD (nmhdr.code) == item index
        static const UINT Type = 3; // LOWORD (nmhdr.code) == item index
        
//        static const UINT DragLeft = 11;
//        static const UINT DragRight = 12;
    };
    namespace ItemType {
        static const UINT SubItem   = 0x00004000u;
        static const UINT Separator = 0x00008000u;
    };

    typedef struct tagNMKEY {
        NMHDR   hdr;
        UINT    nVKey;
        UINT    uFlags;
    } NMKEY, * LPNMKEY;
    
    typedef struct tagNMCHAR {
        NMHDR   hdr;
        UINT    ch;
        DWORD   dwItemPrev;
        DWORD   dwItemNext;
    } NMCHAR, * LPNMCHAR;
};
};

#endif
