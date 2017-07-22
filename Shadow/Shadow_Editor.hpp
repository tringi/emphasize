#ifndef SHADOW_EDITOR_HPP
#define SHADOW_EDITOR_HPP

/* Emphasize Shadow Controls Library - Editor
// Shadow_Editor.hpp
// 
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// 
// Changelog:
//      06.07.2011 - initial version
*/

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

namespace Shadow {
namespace Editor {
    
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates instance of the Shadow Editor control
    //  - parameters: HINSTANCE - module instance handle
    //                HWND - parent window
    //                UINT - styles to set in addition to WS_CHILD
    //                UINT - dialog control ID
    //  - returns handle to dialog control window or NULL on error
    
    HWND Create (HINSTANCE, HWND, UINT, UINT);
    
    // Style
    //  - additional styles controlling appearance and behavior of the editor
    
    namespace Style {
        static const UINT FractionalScrolling   = 0x0010u;
        static const UINT AlwaysHorzScrollBar   = 0x0020u;
        static const UINT ZerosPrefixIndex      = 0x0100u;
    };
    
    // Messages supported
    //  - list of standard and RichEdit messages supported
    //  - WM_SETFONT - assumes fixed width font
    //  - WM_CUT, WM_COPY, WM_CLEAR and WM_PASTE are supported
    //  - EM_UNDO
    //  - EM_REDO
    
    // Messages
    //  - values above WM_USER+0x30 are reserved for RichEdit EM_ messages!
    
    namespace Message {
        static const UINT Jump              = WM_USER + 0x02;
        static const UINT View              = WM_USER + 0x03;
        static const UINT Map               = WM_USER + 0x04;
        static const UINT Select            = WM_USER + 0x05;
        
        static const UINT SetPadding        = WM_USER + 0x10;
        static const UINT SetNumbersFont    = WM_USER + 0x11;
        static const UINT GetNumbersFont    = WM_USER + 0x12;
        static const UINT SetThumbMinimum   = WM_USER + 0x13; // w: 0 ... 50%
        
    };
    
    // Request/Notify
    //  - following integers are 'nmhdr.code'
    
    namespace Request {
        static const UINT First     = 1; // row index corresponding to pixel offset
        static const UINT Rows      = 2; // number of rows
        static const UINT Height    = 3; // row height (NMValue)
        static const UINT HeightSum = 4; // sum of of all heights (.row ignored)
        static const UINT Length    = 5; // row length
        static const UINT LengthMax = 6; // length of the longest row
        
        static const UINT Token     = 0x0A;
        static const UINT Anchor    = 0x0B; // CTRL+arrows jumpable tokens?
        
        static const UINT Insert    = 0x10; // character or block is written
        static const UINT Delete    = 0x11; // character or block is deleted
        static const UINT NewLine   = 0x12; // row is to be broken in two
        static const UINT UnBreak   = 0x13; // new-line shall be removed
        
        static const UINT Undo      = 0x14;
        static const UINT Redo      = 0x15;
        
        static const UINT CopyBegin  = 0x17; // 
        static const UINT CopyAppend = 0x18; // 
        static const UINT CopyCommit = 0x19; // 
    };
    namespace Notify {
        static const UINT MDown     = 0x20; // middle button down
        static const UINT X1Down    = 0x21;
        static const UINT X2Down    = 0x22;
        static const UINT MClick    = 0x40; // middle button click
        static const UINT X1Click   = 0x41;
        static const UINT X2Click   = 0x42;
        static const UINT MDblClk   = 0x60; // middle button double click
        static const UINT X1DblClk  = 0x61;
        static const UINT X2DblClk  = 0x62;
        
        static const UINT Bump      = 0x80;
    };
    namespace Drawing {
        static const UINT Text      = 0;
        static const UINT Index     = 0x10000000;
    };

    // Hit
    //  - editor hittest values
    //  - OutsideHit (cursor leaving) is sent only in NM_NCHITTEST (mouse move),
    //    otherwise can be used as array size

    enum Hit {
        CaptionHit,
        PrefixHit,
        NumberHit,
        PostfixHit,
        TextHit,
        OutsideHit
    };
    
    // Standard WM_NOTIFY messages sent:
    //  - NM_SETCURSOR with NMMOUSE is sent when mouse is moved around editor
    //     - NMMOUSE.dwHitInfo is value of Hit enum above, determines the cursor
    //     - coordinates are meaningless for OutsideHit
    //     - return nonzero to suppress setting the cursor by the editor
    //  - NM_CHAR and NM_KEYDOWN are sent appropriately
    //     - return nonzero to suppress any default behavior
    //  - NM_CUSTOMDRAW with NMCUSTOMDRAW is sent when drawing
    //     - when text area is about to be redrawn:
    //        - CDDS_PREERASE is sent after erasing background, when selection
    //          is about to be drawn
    //           - NMCUSTOMDRAW.lItemlParam is pointer to 9 POINT structures
    //             which describe up to 8 point of selection polygon frame
    //             with .x member of nineth POINT (p[8].x) being number of
    //             points actually used (typically 0, 4 or 8)
    //        - CDDS_POSTERASE is sent after painting selection if on pre-erase
    //          the CDRF_NOTIFYPOSTPAINT flag was returned
    //        - CDDS_PREPAINT is sent before painting text
    //           - returning CDRF_SKIPDEFAULT will skip painting
    //        - CDDS_ITEMPREPAINT sent before every single ROW is painted
    //          if on pre-paint the CDRF_NOTIFYITEMDRAW was returned
    //           - returning CDRF_SKIPDEFAULT will skip row painting
    //           - NMCUSTOMDRAW.rc is row bounding rectangle
    //           - NMCUSTOMDRAW.dwItemSpec is row index
    //        - CDDS_PREPAINT | CDDS_SUBITEM sent before every single TOKEN is
    //          painted if on item-pre-paint a CDRF_NOTIFYITEMDRAW was returned
    //           - returning CDRF_SKIPDEFAULT will skip token painting
    //           - CDRF_NEWFONT should be returned if handler changed the font
    //           - NMCUSTOMDRAW.rc is token bounding rectangle
    //           - NMCUSTOMDRAW.dwItemSpec is row index
    //           - NMCUSTOMDRAW.lItemlParam is token index
    //        - CDDS_POSTPAINT | CDDS_SUBITEM sent after token is painted
    //          if on item-pre-paint the CDRF_NOTIFYPOSTPAINT was returned
    //           - CDRF_NEWFONT should be returned if handler changed the font
    //        - CDDS_ITEMPOSTPAINT sent after single row is painted
    //          if on pre-item-paint the CDRF_NOTIFYPOSTPAINT is returned
    //        - CDDS_POSTPAINT is sent after painting if on pre-paint
    //          the CDRF_NOTIFYPOSTPAINT flag was returned
    
    //     - when index column is about to be redrawn:
    //        - CDDS_PREPAINT | Drawing::Index sent after erase, before painting
    //           - returning CDRF_SKIPDEFAULT will skip painting
    //        - CDDS_ITEMPREPAINT | Drawing::Index sent before single row index
    //          is painted if on pre-paint the CDRF_NOTIFYITEMDRAW was returned
    //           - returning CDRF_SKIPDEFAULT will skip painting
    //        - CDDS_ITEMPOSTPAINT | Drawing::Index sent after single row index
    //          is painted if on pre-item-paint CDRF_NOTIFYPOSTPAINT is returned
    //        - CDDS_POSTPAINT | Drawing::Index after painting if on pre-paint
    //          the CDRF_NOTIFYPOSTPAINT flag was returned
    
    // Mapping
    //  - lParam of Message::Map
    //  - wParam of Message::Map is row
    //  - if either 'column' or 'token':'offset' pair is zero, it gets updated
    //    based on the other one
    
    struct Mapping {
        unsigned int column; // displayed character offset
        unsigned int token;  // token index
        unsigned int offset; // character offset inside the token
    };

    // Selection
    //  - lParam of Message::Select (NULL means deselect all)

    struct Selection {
        struct {
            unsigned int row;
            unsigned int column;
        } first,
          last;
        
        bool column; // false - classic selection, true - column selection
    };

    // NMValue
    //  - value contains suggested value, change is applied by returning nonzero
    //  - sent for many Request values, see that namespace above
    
    struct NMValue {
        NMHDR hdr;
        UINT  row;
        UINT  value;
    };
    
    // NMFirst
    //  - sent to determine first row to render
    
    struct NMFirst {
        NMHDR hdr;
        UINT  offset;
        UINT  row;
        UINT  fraction;
    };
    
    // NMToken
    //  - requests string pointer and length for a particular token on a row
    //  - sent as a parameter to Request::Token
    //  - return nonzero to indicate success in retrieving the word
    
    struct NMToken {
        NMHDR   hdr;
        UINT    row;
        UINT    index;
        int     length; // set to -1 to indicate NUL-terminated string
        LPCTSTR string; // set this to point to the part of the text
    };
    
    // NMAnchor
    //  - requests user to provide a location of next anchor (for CTRL+arrows)
    //  - row/column on input describe current position
    //     - token is provided as a hint
    //  - change 'row' and/or 'column' and return nonzero to update
    
    struct NMAnchor {
        NMHDR   hdr;
        UINT    token;
        UINT    vk; // one of VK_LEFT, VK_UP, VK_RIGHT or VK_DOWN
        UINT    row;
        UINT    column;
    };
    
    // NMInsert
    //  - requests that application inserts 'length' of characters beginning
    //    at 'string' to be inserted into 'row' before 'column' position
    //  - return nonzero to confirm, zero to cancel
    
    struct NMInsert {
        NMHDR   hdr;
        UINT    row;    // row on which to perform the operation
        UINT    column; // index of relevant position
        UINT    length; // length of string
        LPCTSTR string; // NOT NUL-terminated!
    };

    // NMDelete
    //  - requests that application deletes 'length' of the characters from
    //    'row' beginning with 'column' position
    //  - return nonzero to confirm, zero to cancel

    struct NMDelete {
        NMHDR   hdr;
        UINT    row;    // row on which to perform the operation
        UINT    column; // index of relevant position
        UINT    length; // characters to be deleted, or -1 for whole row
    };
    
    // NMNewLine
    //  - requests that application breaks 'row' on the index of 'column'
    //  - return nonzero to confirm, zero to cancel
    
    struct NMNewLine {
        NMHDR   hdr;
        UINT    row;    // row on which to perform the operation
        UINT    column; // index where to break, may be zero or one after length
    };
    
    // NMUnBreak
    //  - requests that application appends content of the next row (row + 1)
    //    to the end of 'row' and removes that next row
    
    struct NMUnBreak {
        NMHDR   hdr;
        UINT    row;
    };

    // NMCopyAppend
    //  - 

    struct NMCopyAppend {
        NMHDR   hdr;
        UINT    row;
        UINT    column;
        UINT    length; // -1u means until the end
        bool    newline; // terminated by newline
    };
    
    // NMBump
    //  - VK_UP, VK_DOWN, VK_LEFT or VK_RIGHT
    
    struct NMBump {
        NMHDR   hdr;
        UINT    vk;
    };
    
    // NMKEY/NMCHAR
    //  - 

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
