/////////////////////////////////////////////////////////////////////////////
// Name:	ModelDlg.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __ModelDlg_H__
#define __ModelDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "ModelDlg.cpp"
#endif

#include "cmanager_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"

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
	wxString2	m_strFilename;
	float		m_fDistance;
	float		m_fScale;
	wxString2	m_strStatus;
	vtModel		*m_pCurrentModel;
	bool		m_bUpdating;

private:
	// WDR: handler declarations for ModelDlg
	void OnTextScale( wxCommandEvent &event );
	void OnTextVisible( wxCommandEvent &event );
	void OnTextFilename( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif

