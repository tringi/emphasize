#ifndef SHADOW_SPLITTER_HPP
#define SHADOW_SPLITTER_HPP

/* Emphasize Shadow Controls Library - Splitter bar
// Shadow_Splitter.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      19.08.2011 - initial version
*/

#include <windows.h>
#include <commctrl.h>

namespace Shadow {
namespace Splitter {
    
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates the Splitter control as a child of HINSTANCE/HWND
    //  - first UINT parameter is style, WS_CHILD is already included
    //  - second UINT parameter is Dialog Control Identifier
    
    HWND Create (HINSTANCE, HWND, UINT, UINT);
    
    // Style
    //  - window styles for the control

    namespace Style {
        static const UINT Horizontal    = 0x0001u;
        static const UINT Vertical      = 0x0002u;
    };
    
    // Request == nmhdr.code
    
    namespace Request {
        
        // Moving
        //  - the control is about to be moved
        //  - pointer to NMHDR is actually pointer to NMMoving structure
        
        static const UINT Moving    = 1;
        
        // Bounds
        //  - request for lower and upper bounds for the movement
        //  - pointer to NMHDR is actually pointer to NMBounds structure
        //  - returning 0 means that no bounds are enforced
        
        static const UINT Bounds    = 2;
        
        // Home
        //  - request for a home position
        //  - pointer to NMHDR is actually pointer to NMHome structure
        //  - failing to set HMHome::home will result in home to be 0,0
        
        static const UINT Home      = 3;
        
        // Away
        //  - request for a away position
        //  - pointer to NMHDR is actually pointer to NMAway structure
        //  - failing to set HMAway::away will result in no animation
        
        static const UINT Away      = 4;
        
        // Cursor
        //  - request for a cursor to set for the window
        //  - return 0 for control to use default cursor
        
        static const UINT Cursor    = 5;
    };
    
    // NM_CUSTOMDRAW
    //  - following codes are sent
    //  - CDDS_PREERASE - before any drawing
    //                  - returning CDRF_SKIPDEFAULT will skip WM_CTLCOLORSTATIC
    //                    callback from being sent
    //                  - returning CDRF_NOTIFYPOSTERASE will get CDDS_POSTERASE
    //  - CDDS_POSTERASE - after erasing background, if requested
    //  - CDDS_PREPAINT - befor grip is drawn
    //                  - returning CDRF_SKIPDEFAULT will not draw grip & focus
    //                  - returning CDRF_SKIPPOSTPAINT will not draw focus
    //                  - returning CDRF_NOTIFYPOSTPAINT will get CDDS_POSTPAINT
    //  - CDDS_POSTPAINT - after all painting
    
    // NM_CLICK
    //  - on click, the controls send WM_NOTIFY::NM_CLICK
    //  - returning non-zero will suppress capturing/focusing for movement
    
    // NM_KEYDOWN/NMKEY
    //  - on key press, the controls send WM_NOTIFY::NM_KEYDOWN
    //  - returning non-zero will suppress default movement
    //  - NMKEY provided for compatibility with incomplete MinGW commctrl.h
    //  - NMKEY taken from Windows SDK documentation
    
    typedef struct tagNMKEY {
        NMHDR   hdr;
        UINT    nVKey;
        UINT    uFlags;
    } NMKEY, * LPNMKEY;
    
    // NMMoving
    //  - previous - previous window position
    //  - offset - offset in which the user actually draggs the splitter
    //  - drag - drag client coordinated in the control
    //  - target - proposed new coordinates (x,y,cx,cy) that user can modify
    
    struct NMMoving {
        NMHDR   hdr;
        POINT   previous;
        POINT   offset;
        POINT   drag;
        RECT    target;
    };
    
    // NMHome/NMAway
    //  - the "home" and "away" positions
    //  - user is required to fill the appropriate members
    //  - not applicable members are ignored
    
    struct NMHome {
        NMHDR   hdr;
        POINT   home;
    };
    struct NMAway {
        NMHDR   hdr;
        POINT   away;
    };

    // NMBounds
    //  - bounds - user is required to fill the appropriate members
    //           - not applicable members are ignored
    
    struct NMBounds {
        NMHDR   hdr;
        RECT    bounds;
    };
    
};
};

#endif
