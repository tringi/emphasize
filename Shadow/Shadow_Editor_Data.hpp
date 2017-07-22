#ifndef SHADOW_EDITOR_DATA_HPP
#define SHADOW_EDITOR_DATA_HPP

/* Emphasize Shadow Controls Library - Editor - Internal Data structure
// Shadow_Editor_Data.hpp
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
    class Data {
        public:
            HFONT   hFont;
            SIZE    character;
    
            struct {
                unsigned int top;
                unsigned int left;
            } margin;
    
            struct {
                struct {
                    unsigned int left;
                    unsigned int right;
                } padding;
    
                unsigned int minimum;
                unsigned int width;
            } index;
    
            struct Caret {
                unsigned int    row;
                unsigned int    column;
                unsigned short  width;
                unsigned short  height;
                unsigned int    token;
                unsigned int    offset;
            } caret;
    
            struct Selection {
                unsigned int    row;
                unsigned int    column;
    
                enum Type {
                    None = 0,
                    Normal,
                    Column
                } type;
            } selection;
            
            struct Thumb {
                unsigned char   minimum; // percent of window height (0...50)
            } thumb;
    
            bool        recompute;
            bool        reposition; // call UpdateCaret after OnPaint
            bool        vanished;
            bool        overwritting;
            bool        autoscrollbar;
    
            signed char wheelV;
            signed char wheelH;
            
            unsigned char bold_caret; // width when overwritting
            
        
        public:
            static LONG_PTR New ();
            static void Delete (HWND);
        
        public:
            static const unsigned int MagicIndexBorder = 2u;
            static inline Data * Ref (HWND hWnd) {
                return reinterpret_cast <Data *> (GetWindowLongPtr (hWnd, 0));
            };
    };
};
};

#endif
