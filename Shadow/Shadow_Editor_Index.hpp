#ifndef SHADOW_EDITOR_INDEX_HPP
#define SHADOW_EDITOR_INDEX_HPP

/* Emphasize Shadow Controls Library - Editor Index
// Shadow_Editor_Index.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      04.02.2012 - initial version
*/

#include "Shadow_Editor.hpp"

namespace Shadow {
namespace Editor {
namespace Index {

    // Initialize
    //  - registers the window class in the provided instance

    ATOM Initialize (HINSTANCE);

    // Create
    //  - creates instance of the Shadow Editor Index control
    //  - parameters: HINSTANCE - module instance handle
    //                HWND - parent window
    //                UINT - styles to set in addition to WS_CHILD
    //                UINT - dialog control ID
    //  - returns handle to dialog control window or NULL on error

    HWND Create (HINSTANCE, HWND, UINT, UINT);

    // Style
    //  - additional styles controlling appearance and behavior of the index

    namespace Style {
        static const UINT ZerosPrefix = 0x0100u;
    };

    namespace Message {
        static const UINT SetOffset = WM_USER + 0; // wP - row, lP - fraction
        static const UINT SetPadding = WM_USER + 1; // wP - left, lP - right
        static const UINT SetDefaultRowHeight = WM_USER + 2;
        static const UINT SetNumbersCount = WM_USER + 3;
    };

};
};
};

#endif
