#include "Shadow_Editor_Renderer.hpp"

/* Emphasize Shadow Controls Library - Editor - Internal Renderer class
// Shadow_Editor_Renderer.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      12.02.2012 - initial version
*/

Shadow::Editor::Renderer::Renderer (HWND _hWnd, HDC _hDC,
                                    RECT _rcWindow, RECT _rcClip,
                                    unsigned int _row, unsigned int _fraction,
                                    const POINT * _sp)
    :   hWnd (_hWnd),
        hDC (_hDC),
        rcWindow (_rcWindow),
        rcClip (_rcClip),
        sp (_sp),
        data (Data::Ref (hWnd)),
        row (_row),
        fraction (_fraction) {
    
    SetBkMode (this->hDC, TRANSPARENT);
    SelectObject (this->hDC, GetStockObject (DC_PEN));
    SelectObject (this->hDC, GetStockObject (DC_BRUSH));
    
    this->Erase ();
    this->Selection ();
    this->Content ();
    return;
};

void Shadow::Editor::Renderer::Erase () {
    FillRect (this->hDC, &this->rcClip, (HBRUSH)
              SendMessage (GetParent (this->hWnd), WM_CTLCOLOREDIT,
                           (WPARAM) this->hDC, (LPARAM) this->hWnd));
    return;
};

void Shadow::Editor::Renderer::Selection () {
    POINT polygon [9];
    LONG & points = polygon[8].x;

    points = 4u;

    // Rendering selection
    //  - passing selection frame in CDDS_PREERASE to parent window
    //  - so it can alter rendering to something as per Emphasize specs

    if (data->selection.type) {

        const unsigned int h0 = this->RowHeight (data->selection.row);
        const unsigned int h1 = this->RowHeight (data->caret.row);
        const unsigned int lm = data->character.cx
                              * this->Request (Request::LengthMax);

        switch (data->selection.type) {
            case Data::Selection::None:
                break;
            case Data::Selection::Normal:
                if (data->selection.row == data->caret.row) {
                    polygon[0].x = sp[0].x; polygon[0].y = sp[0].y;
                    polygon[1].x = sp[1].x; polygon[1].y = sp[0].y;
                    polygon[2].x = sp[1].x; polygon[2].y = sp[0].y + h0;
                    polygon[3].x = sp[0].x; polygon[3].y = sp[0].y + h0;
                } else {
                    polygon[0].x = sp[0].x; polygon[0].y = sp[0].y;
                    polygon[1].x =      lm; polygon[1].y = sp[0].y;
                    polygon[2].x =      lm; polygon[2].y = sp[1].y;
                    polygon[3].x = sp[1].x; polygon[3].y = sp[1].y;
                    polygon[4].x = sp[1].x; polygon[4].y = sp[1].y + h1;
                    polygon[5].x =       0; polygon[5].y = sp[1].y + h1;
                    polygon[6].x =       0; polygon[6].y = sp[0].y + h0;
                    polygon[7].x = sp[0].x; polygon[7].y = sp[0].y + h0;
                    points = 8u;
                };
                break;
            case Data::Selection::Column:
                polygon[0].x = sp[0].x; polygon[0].y = sp[0].y;
                polygon[1].x = sp[1].x; polygon[1].y = sp[0].y;
                polygon[2].x = sp[1].x; polygon[2].y = sp[1].y;
                polygon[3].x = sp[0].x; polygon[3].y = sp[1].y;

                if (data->caret.row < data->selection.row) {
                    polygon[0].y += h0;
                    polygon[1].y += h0;
                } else {
                    polygon[2].y += h1;
                    polygon[3].y += h1;
                };
                break;
        };

        for (int i = 0; i < points; ++i) {
            polygon[i].x += data->margin.left - GetScrollPos (hWnd, SB_HORZ) + 1;
            polygon[i].y += data->margin.top;
        };

    } else {
        points = 0u;
    };

    COLORREF crHi = GetSysColor (COLOR_HIGHLIGHT);
    COLORREF crWn = GetSysColor (COLOR_WINDOW);
    COLORREF crBg = RGB ((GetRValue (crHi) + GetRValue (crWn)) / 2u,
                         (GetGValue (crHi) + GetGValue (crWn)) / 2u,
                         (GetBValue (crHi) + GetBValue (crWn)) / 2u);

    SetDCPenColor (hDC, crHi);
    SetDCBrushColor (hDC, crBg);

    DWORD cdn = this->Callback (CDDS_PREERASE, &rcWindow, 0, CDIS_DEFAULT,
                                reinterpret_cast <LPARAM> (polygon));
    if (!(cdn & CDRF_SKIPDEFAULT) && points > 0) {
        Polygon (hDC, polygon, points);
    };
    if (cdn & CDRF_NOTIFYPOSTPAINT)
        this->Callback (CDDS_POSTERASE, &rcWindow);

    return;
};

void Shadow::Editor::Renderer::Row (DWORD cdnPrePaint,
                                    unsigned int height,
                                    RECT & rr) {

    DWORD cdn = 0;
    if (cdnPrePaint & CDRF_NOTIFYITEMDRAW)
        cdn = this->Callback (CDDS_ITEMPREPAINT, &rr, row);

    if (!(cdn & CDRF_SKIPDEFAULT)) {

        // render each token

        RECT rword = rr;
        rword.left -= GetScrollPos (hWnd, SB_HORZ);

        SelectObject (hDC, data->hFont);
        int length = 0;
        LPCTSTR string = NULL;

        int i = 0;
        while (rword.left < rcWindow.right
                && this->Next (i, string, length)) {

            if (DrawText (hDC, string, length, &rword, DT_CALCRECT |
                          DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP)) {

                RECT rtemp;
                if (IntersectRect (&rtemp, &rword, &rcClip)) {
                    this->Token (cdn, height, i, string, length, rword);
                };

                rword.left += rword.right - rword.left;
            };

            ++i;
        };
    };

    if (cdn & CDRF_NOTIFYPOSTPAINT)
        this->Callback (CDDS_ITEMPOSTPAINT, &rr, row);

    return;
};

void Shadow::Editor::Renderer::Token (DWORD cdn,
                                      unsigned int height, unsigned int i,
                                      LPCTSTR & string, int length,
                                      RECT & rToken) {
    DWORD cdnsub = 0;
    DWORD cdnsub2 = 0;

    UINT state = CDIS_DEFAULT;

    // TODO: set CDIS_FOCUS on caret in this token, CDIS_HOT on mouse, CDIS_SELECTED if in selection
    // TODO: the same flags for whole row

    if (cdn & CDRF_NOTIFYITEMDRAW) {
        cdnsub = this->Callback (CDDS_PREPAINT | CDDS_SUBITEM,
                                 &rToken, row, state, i);
    };
    if (!(cdnsub & CDRF_SKIPDEFAULT)) {
        rToken.bottom = rToken.top + height;
        DrawText (hDC, string, length, &rToken,
                  DT_SINGLELINE | DT_NOPREFIX);
    };
    if (cdnsub & CDRF_NOTIFYPOSTPAINT) {
        cdnsub2 = this->Callback (CDDS_POSTPAINT | CDDS_SUBITEM,
                                  &rToken, row, state, i);
    };

    if ((cdnsub | cdnsub2) & CDRF_NEWFONT)
        SelectObject (hDC, data->hFont);

    return;
};

void Shadow::Editor::Renderer::Content () {
    
    DWORD cdn = this->Callback (CDDS_PREPAINT, &rcWindow);
    if (!(cdn & CDRF_SKIPDEFAULT)) {

        RECT rr = {
            (long) data->margin.left + 1,
            (long) data->margin.top,
            (long) rcWindow.right,
            (long) data->margin.top + (long) data->character.cy
        };

        rr.top -= this->fraction;
        rr.bottom -= this->fraction;

        // Rendering rows
        //  - start with first visible and continue until first row that would
        //    not be visible or until we run out of rows

        const unsigned int rows = this->Request (Request::Rows);
        while ((this->row < rows) && (rr.top < rcWindow.bottom)) {

            const unsigned int height = this->RowHeight (row);
            if (height) {

                RECT rtemp;
                if (IntersectRect (&rtemp, &rr, &rcClip)) {
                    this->Row (cdn, height, rr);
                };

                rr.top += height;
                rr.bottom += height;
            };
            ++this->row;
        };
    };

    if (cdn & CDRF_NOTIFYPOSTPAINT)
        this->Callback (CDDS_POSTPAINT, &rcWindow);
    
    return;
};

LRESULT Shadow::Editor::Renderer::Next (UINT i,
                                        LPCTSTR & string, int & length) {
    NMToken nm = {
        {   this->hWnd, (UINT) GetDlgCtrlID (this->hWnd),
            Shadow::Editor::Request::Token },
        this->row, i, 0, NULL
    };
    
    LRESULT result = SendMessage (GetParent (this->hWnd), WM_NOTIFY,
                                  nm.hdr.idFrom, (LPARAM) &nm);
    if (result) {
        string = nm.string;
        length = nm.length;
    };
    return result;
};
LRESULT Shadow::Editor::Renderer::Callback (DWORD stage, LPCRECT rc, UINT item,
                                            UINT state, LPARAM lParam) {
    NMCUSTOMDRAW nmCustomDraw = {
        { this->hWnd, (UINT) GetDlgCtrlID (this->hWnd), (UINT) NM_CUSTOMDRAW },
        stage, this->hDC, *rc, item, state, lParam
    };
    return SendMessage (GetParent (this->hWnd), WM_NOTIFY,
                        nmCustomDraw.hdr.idFrom, (LPARAM) &nmCustomDraw);
};
LRESULT Shadow::Editor::Renderer::Request (UINT code) {
    NMHDR nm = {
        this->hWnd, (UINT) GetDlgCtrlID (this->hWnd), code
    };
    return SendMessage (GetParent (this->hWnd), WM_NOTIFY,
                        nm.idFrom, (LPARAM) &nm);
};
LRESULT Shadow::Editor::Renderer::RowHeight (UINT row) {
    NMValue nm = {
        {   this->hWnd, (UINT) GetDlgCtrlID (this->hWnd),
            Shadow::Editor::Request::Height },
        row, (UINT) this->data->character.cy
    };
    if (SendMessage (GetParent (hWnd), WM_NOTIFY,
                     nm.hdr.idFrom, (LPARAM) &nm)) {
        return nm.value;
    } else
        return this->data->character.cy;
};
