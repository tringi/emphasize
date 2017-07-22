#include "Shadow_Editor_Data.hpp"

/* Emphasize Shadow Controls Library - Editor - Internal Data structure
// Shadow_Editor_Data.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      04.02.2012 - initial version
*/

LONG_PTR Shadow::Editor::Data::New () {
    Data * const data = static_cast <Data *>
                (HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, sizeof (Data)));

    data->hFont = (HFONT) GetStockObject (SYSTEM_FIXED_FONT);
    data->index.minimum = 4u;
    data->character.cx = 8u;
    data->character.cy = 12u;

    data->recompute = true;
    return reinterpret_cast <LONG_PTR> (data);
};

void Shadow::Editor::Data::Delete (HWND hWnd) {
    HeapFree (GetProcessHeap (), 0, Data::Ref (hWnd));
    return;
};
