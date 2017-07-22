#ifndef SHADOW_GRID_HPP
#define SHADOW_GRID_HPP

/* Emphasize Shadow Controls Library - Grid 
// Shadow_Grid.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      10.01.2014 - initial version
*/

#include <windows.h>
#include <commctrl.h>

namespace Shadow {
namespace Grid {
        
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates the Grid window
    
    HWND Create (HINSTANCE, HWND, UINT style, UINT id);
    
    // style
    //  - 
    
    namespace Style {
        static const unsigned int HotTrack = 0x0100;
        static const unsigned int NoHeader = 0x0200;
        static const unsigned int NoDropDowns = 0x0400;
        static const unsigned int WantTab = 0x0800;
        static const unsigned int LiveUpdate = 0x1000;
    };
    
#ifndef HDN_DROPDOWN
#define HDN_DROPDOWN            (HDN_FIRST-18)
#define HDN_ITEMKEYDOWN         (HDN_FIRST-17)
#endif

   
    namespace Request {
        static const unsigned int Rows = 1;
        static const unsigned int Type = 2; // NMRequest, return CellType
        static const unsigned int Value = 3; // NMRequest, return value as string
        static const unsigned int Limit = 4; // NMRequest, return max string length

        static const unsigned int Depth = 6; // NMRequest, ret number of options
        static const unsigned int Option = 7; // NMRequest, i is option index
        
        static const unsigned int Prefetch = 10u; // NMPrefetch
        static const unsigned int Change = 11u; // NMChange
        static const unsigned int Drag = 12u; // NMDrag, ret. 1 to suppress Change
        static const unsigned int Track = 13u; // NMTrack
        static const unsigned int Enter = 14u; // NMTrack
        
        static const unsigned int HeaderCustomDraw = 112u; // forwarded NM_CUSTOMDRAW from header
    };
    namespace Message {
        
        // Message::Insert
        //  - inserts empty item, wParam = width, lParam = header lParam
        //  - title is requested by HDN_GETDISPINFO with only lParam valid
        
        static const unsigned int Insert = WM_USER + 2;
        
        // Message::SetCurrent
        //  - moves focus to: item = wParam, column = lParam, or -1 -1 for none
        
        static const unsigned int SetCurrent = WM_USER + 3;
        
        // Message::SetColumnWidth
        //  - inserts empty item, wParam = width, lParam = index

        static const unsigned int SetColumnWidth = WM_USER + 4;
        
        // Message::GetColumnIndex
        //  - finds header item by lParam and returns the index

//        static const unsigned int GetColumnIndex = WM_USER + 5;
        
        
//        static const unsigned int SetWidth = WM_USER + 10; // ???
//        static const unsigned int SetTextAlign = WM_USER + 11; // ???
        
        static const unsigned int SetSortOrder = WM_USER + 11; // w = index
        static const unsigned int GetSortOrder = WM_USER + 12; // l = +1, 0, -1
    };
    
    enum class CellType : unsigned int {
        Inactive,
        Grayed,
        Checkbox,       // return -1,0,1,2,3,4 (with 0x10 for disabled)
        Text,           // return pointer to text string
//        DecimalNumber,  // TODO: align right, ensure number
//        RealNumber,
//        MonetaryValue,
        MenuItem,       // clickable text rendered somewhat like menu
        Selection,
        MultipleSelection
        // TODO: Date, Time, DateTime, IP?
    };
    
    // TODO: v2 - struct column { int index/ID; int order; int lparam; }
    
    struct NMRequest {
        NMHDR hdr;
        UINT  row;
        UINT  column;
        UINT  i;
    };
    struct NMPrefetch {
        NMHDR hdr;
        struct {
            UINT  first;
            UINT  last;
        } row,
          column;
    };
    
    // NMChange
    //  - send with Request::Change code as a notification of user making change
    //  - for CellType::Selection the 'value' string contains a number, 
    //    a zero-based index of selected option
    //  - for CellType::MultipleSelection the 'value' string contains
    
    struct NMChange {
        NMHDR hdr;
        UINT  row;
        UINT  column;
        const wchar_t * value; // suggested new value
    };
    struct NMDrag {
        NMHDR hdr;
        struct {
            UINT  row;
            UINT  column;
        } source,
          target;
    };
    struct NMTrack {
        NMHDR hdr;
        UINT  row;
        UINT  column;
    };
    
    /*typedef struct tagNMKEY {
        NMHDR   hdr;
        UINT    nVKey;
        UINT    uFlags;
    } NMKEY, * LPNMKEY;*/
};
};

#endif

