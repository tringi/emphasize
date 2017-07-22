#ifndef SHADOW_EDITOR_RENDERER_HPP
#define SHADOW_EDITOR_RENDERER_HPP

/* Emphasize Shadow Controls Library - Editor - Internal Renderer class
// Shadow_Editor_Renderer.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      12.02.2012 - initial version
*/

#include "Shadow_Editor.hpp"
#include "Shadow_Editor_Data.hpp"

namespace Shadow {
namespace Editor {
    
    class Renderer {
        private:
            const HWND hWnd;
            const HDC  hDC;
            const RECT rcWindow;
            const RECT rcClip;
            const POINT * sp;
            const Data * data;
            
            unsigned int row;
            unsigned int fraction;
        
        private:
            void Erase ();
            void Selection ();
            void Content ();
            void Row (DWORD, unsigned int, RECT &);
            void Token (DWORD, unsigned int, unsigned int,
                        LPCTSTR &, int, RECT &);
            
            LRESULT Request (UINT);
            LRESULT RowHeight (UINT);
            LRESULT Callback (DWORD, LPCRECT, UINT = 0,
                              UINT = CDIS_DEFAULT, LPARAM = 0);
            LRESULT Next (UINT, LPCTSTR &, int &);
        
        public:
            Renderer (HWND hWnd, HDC hDC, RECT rcWindow, RECT rcClip,
                      unsigned int, unsigned int, const POINT * sp);
        
        private:
            Renderer (const Renderer &); // = delete;
            Renderer & operator = (const Renderer &); // = delete;
    };
};
};

#endif
