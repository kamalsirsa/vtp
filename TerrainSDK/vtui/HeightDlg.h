/////////////////////////////////////////////////////////////////////////////
// Name:        HeightDlg.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __HeightDlg_H__
#define __HeightDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma interface "HeightDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "vtui_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/Building.h"
#include "vtdata/HeightField.h"

#define BASELINE_COL 0
#define ACTIVE_COL 1
#define STOREYS_COL 2
#define HEIGHT_COL 3

enum
{
    SCALE_EVENLY = 0,
    SCALE_FROM_BOTTOM,
    SCALE_FROM_TOP
};


class CHeightGrid;

// WDR: class declarations

//---------------------------------------------------------------------------
// CHeightDialog
//---------------------------------------------------------------------------

class CHeightDialog: public AutoDialog
{
public:
    // constructors and destructors
    CHeightDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );

    void Setup(vtBuilding * const pBuilding, vtHeightField *pHeightField);
    void OnLeftClickGrid( wxGridEvent &event );
    void OnGridEditorHidden( wxGridEvent &event );

    // WDR: method declarations for CHeightDialog
    wxTextCtrl* GetBaselineoffset()  { return (wxTextCtrl*) FindWindow( ID_BASELINEOFFSET ); }
    wxRadioBox* GetScaleradiobox()  { return (wxRadioBox*) FindWindow( ID_SCALERADIOBOX ); }
    CHeightGrid* GetHeightgrid()  { return (CHeightGrid*) FindWindow( ID_HEIGHTGRID ); }

protected:
    // WDR: member variable declarations for CHeightDialog

protected:
    // WDR: handler declarations for CHeightDialog
    void OnBaselineOffset( wxCommandEvent &event );
    void OnOK( wxCommandEvent &event );
    void OnRecalculateHeights( wxCommandEvent &event );
    void OnInitDialog(wxInitDialogEvent& event);

private:
    bool m_bGridModified;
    CHeightGrid *m_pHeightGrid;
    wxRadioBox *m_pScaleradiobox;
    vtBuilding *m_pBuilding;
    vtHeightField *m_pHeightField;
    wxTextCtrl* m_pBaselineOffset;
    float m_fBaselineOffset;
    int m_BottomRow;
    int m_NumLevels;
    double m_dBaseLine;
    DECLARE_EVENT_TABLE()
};




#endif
