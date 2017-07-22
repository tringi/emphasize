#ifndef SHADOW_CHART_HPP
#define SHADOW_CHART_HPP

/* Shadow Chart 
// Shadow_Chart.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      26.05.2014 - initial version
*/

#include <windows.h>

#include <vector>
#include <utility>

namespace Shadow {
namespace Chart {
    
    // Initialize
    //  - registers the window class in the provided instance
    
    ATOM Initialize (HINSTANCE);
    
    // Create
    //  - creates instance of the Shadow Chart control
    //  - parameters: HINSTANCE - module instance handle
    //                HWND - parent window
    //                UINT - dialog control ID
    //  - returns handle to dialog control window or NULL on error
    
    HWND Create (HINSTANCE, HWND, UINT id);
    
    // Requests
    
    namespace Request {
        static const auto Layout = 1u; // NMHDR points to Layout below
        static const auto Info = 2u; // NMHDR points to Info below
        static const auto Marker = 3u;  // NMHDR points to Marker below
    };
    
    struct Layout {
        NMHDR nm;
        UINT charts; // number of charts
        RECT margin;
        struct {
            float minimum;
            float maximum;
        } x, y;
    };
    struct Info {
        NMHDR nm;
        UINT chart; // index
        UINT width;
        COLORREF color;
        
        std::vector <std::pair <float, float>> * data;
    };
    struct Marker {
        NMHDR nm;
        UINT index; // incremented for more markers, return false to stop
        
        float y;
        COLORREF color;
        COLORREF textcolor;
        wchar_t text [32];
    };
    
};
};

#endif

