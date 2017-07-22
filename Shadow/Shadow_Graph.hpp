#ifndef SHADOW_GRAPH_HPP
#define SHADOW_GRAPH_HPP

/* Emphasize Shadow Controls Library - Graph
// Shadow_Graph.hpp
// 
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
// 
// Changelog:
//      06.03.2012 - initial version
*/

#include <windows.h>
#include <commctrl.h>

#include <string>

namespace Shadow {
namespace Graph {
    
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates instance of the Shadow Graph control
    //  - parameters: HINSTANCE - module instance handle
    //                HWND - parent window
    //                UINT - dialog control ID
    //                Interface - mandatory callbacks defined by user
    //  - returns handle to dialog control window or NULL on error
    
    HWND Create (HINSTANCE, HWND, UINT, class Interface &,
                 DWORD style = 0u, DWORD extra = 0u);
    
    // Interface
    //  - 
    
    class Interface {
        public:
            virtual RECT GraphMargin () const;
            
            virtual double GraphStart () const = 0;
            virtual double GraphEnd () const = 0;
            virtual double GraphGridStep (unsigned int level) const = 0;

            virtual unsigned int GraphCount () const = 0;
            virtual COLORREF     GraphColor (unsigned int) const = 0;
            virtual const wchar_t * GraphUnits (unsigned int) const;
            
            virtual std::wstring GraphNote (unsigned int, double, unsigned int) const;
            virtual double       GraphNoteWidth (unsigned int) const;
            
            virtual std::wstring GraphAxisNote (double) const;

            virtual double       GraphMinimum (unsigned int) const = 0;
            virtual double       GraphMaximum (unsigned int) const = 0;
            virtual unsigned int GraphSampleCount (unsigned int) const = 0;
            virtual bool         GraphSampleValid (unsigned int, unsigned int) const;
            virtual bool         GraphSampleBreak (unsigned int, unsigned int) const;
            virtual double       GraphSampleKey   (unsigned int, unsigned int) const = 0;
            virtual double       GraphSampleValue (unsigned int, unsigned int) const = 0;
            virtual double       GraphExtraGridRow (unsigned int, unsigned int) const;
            virtual double       GraphRightGridRow (unsigned int, unsigned int) const;
            virtual bool         GraphValidGridRow (unsigned int, double) const;

        
//        virtual unsigned int Rows () const;
    };
};
};

#endif
