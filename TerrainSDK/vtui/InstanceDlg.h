//
// InstanceDlg.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __InstanceDlg_H__
#define __InstanceDlg_H__

#include "vtui_UI.h"
#include "AutoDialog.h"
#include "vtdata/Projections.h"
#include "vtdata/Content.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// InstanceDlg
//----------------------------------------------------------------------------

class InstanceDlg: public InstanceDlgBase
{
public:
	// constructors and destructors
	InstanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void ClearContent();
	void AddContent(vtContentManager *mng);

	void SetProjection(const vtProjection &proj) { m_proj = proj; }
	void SetLocation(const DPoint2 &pos);
//	wxString GetPath() { return GetModelFile()->GetValue(); }
	vtTagArray *GetTagArray();

	// WDR: method declarations for InstanceDlg
	wxTextCtrl* GetLocation()  { return (wxTextCtrl*) FindWindow( ID_LOCATION ); }
	wxButton* GetBrowseModelFile()  { return (wxButton*) FindWindow( ID_BROWSE_MODEL_FILE ); }
	wxTextCtrl* GetModelFile()  { return (wxTextCtrl*) FindWindow( ID_MODEL_FILE ); }
	wxChoice* GetChoiceItem()  { return (wxChoice*) FindWindow( ID_CHOICE_ITEM ); }
	wxChoice* GetChoiceType()  { return (wxChoice*) FindWindow( ID_CHOICE_TYPE ); }
	wxChoice* GetChoiceFile()  { return (wxChoice*) FindWindow( ID_CHOICE_FILE ); }
	wxRadioButton* GetRadioModel()  { return (wxRadioButton*) FindWindow( ID_RADIO_MODEL ); }
	wxRadioButton* GetRadioContent()  { return (wxRadioButton*) FindWindow( ID_RADIO_CONTENT ); }

private:
	// WDR: member variable declarations for InstanceDlg
	DPoint2			m_pos;
	vtProjection	m_proj;
	bool			m_bContent;
	int				m_iManager;
	int				m_iItem;
	vtTagArray		m_dummy;

private:
	// WDR: handler declarations for InstanceDlg
	void OnLocationText( wxCommandEvent &event );
	void OnBrowseModelFile( wxCommandEvent &event );
	void OnChoice( wxCommandEvent &event );
	void OnChoiceItem( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

	void UpdateLoc();
	void UpdateEnabling();
	void UpdateContentItems();

	std::vector<vtContentManager*> m_contents;
	vtContentManager *Current();

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __InstanceDlg_H__

