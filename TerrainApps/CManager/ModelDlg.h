/////////////////////////////////////////////////////////////////////////////
// Name:        ModelDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __ModelDlg_H__
#define __ModelDlg_H__

#ifdef __GNUG__
    #pragma interface "ModelDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "cmanager_wdr.h"
#include "AutoPanel.h"

class vtModel;

// WDR: class declarations

//----------------------------------------------------------------------------
// ModelDlg
//----------------------------------------------------------------------------

class ModelDlg: public AutoPanel
{
public:
    // constructors and destructors
    ModelDlg( wxWindow *parent, wxWindowID id = -1,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER );
    
    // WDR: method declarations for ModelDlg
    void SetCurrentModel(vtModel *mod);
	void SetModelStatus(const char *string);
	void UpdateFromControls();

private:
    // WDR: member variable declarations for ModelDlg
    wxString	m_strFilename;
    float		m_fDistance;
    float		m_fScale;
	wxString	m_strStatus;
	vtModel		*m_pCurrentModel;
	bool		m_bUpdating;

private:
    // WDR: handler declarations for ModelDlg
    void OnTextScale( wxCommandEvent &event );
    void OnTextVisible( wxCommandEvent &event );
    void OnTextFilename( wxCommandEvent &event );
    void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};




#endif
