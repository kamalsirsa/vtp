//
// Name:		LocationDlg.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LocationDlg_H__
#define __LocationDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "LocationDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtlib/core/Location.h"
#include "vtlib/core/AnimPath.h"
#include "vtui/AutoDialog.h"

class vtLocationSaver;

// WDR: class declarations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

class LocationDlg: public AutoDialog
{
public:
	// constructors and destructors
	LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~LocationDlg();

	void OnInitDialog(wxInitDialogEvent &event) {}

	// WDR: method declarations for LocationDlg
	wxBitmapButton* GetReset()  { return (wxBitmapButton*) FindWindow( ID_RESET ); }
	wxCheckBox* GetSmooth()  { return (wxCheckBox*) FindWindow( ID_SMOOTH ); }
	wxButton* GetSaveAnim()  { return (wxButton*) FindWindow( ID_SAVE_ANIM ); }
	wxButton* GetStop()  { return (wxButton*) FindWindow( ID_STOP ); }
	wxButton* GetPlay()  { return (wxButton*) FindWindow( ID_PLAY ); }
	wxListBox* GetAnims()  { return (wxListBox*) FindWindow( ID_ANIMS ); }
	wxButton* GetStoreas()  { return (wxButton*) FindWindow( ID_STOREAS ); }
	wxButton* GetStore()  { return (wxButton*) FindWindow( ID_STORE ); }
	wxButton* GetRecall()  { return (wxButton*) FindWindow( ID_RECALL ); }
	wxListBox* GetLoclist()  { return (wxListBox*) FindWindow( ID_LOCLIST ); }
	wxButton* GetRemove()  { return (wxButton*) FindWindow( ID_REMOVE ); }

	void RefreshList();
	void SetTarget(vtTransform *pTarget, const vtProjection &proj,
				   const vtLocalConversion &conv);
	void SetLocFile(const vtString &fname);
	void RefreshButtons();
	void RecallFrom(const vtString &locname);

	void RefreshAnims();
	void UpdateEnabling();
	void SlidersToValues();
	void ValuesToSliders();

	void SetValues();
	void GetValues();
	void TransferToWindow();

private:
	// WDR: member variable declarations for LocationDlg
	vtLocationSaver *m_pSaver;

	Array<vtAnimPath3d *> m_Anims;
	Array<vtAnimPathEngine *> m_Engines;

	wxButton* m_pStoreAs;
	wxButton* m_pStore;
	wxButton* m_pRecall;
	wxButton* m_pRemove;
	wxListBox* m_pLocList;

	bool m_bSmooth;
	bool m_bPosOnly;
	int m_iAnim;
	float m_fSpeed;
	int m_iSpeed;
	bool m_bSetting;

private:
	// WDR: handler declarations for LocationDlg
	void OnText( wxCommandEvent &event );
	void OnSpeedSlider( wxCommandEvent &event );
	void OnReset( wxCommandEvent &event );
	void OnCheckbox( wxCommandEvent &event );
	void OnAnim( wxCommandEvent &event );
	void OnStop( wxCommandEvent &event );
	void OnPlay( wxCommandEvent &event );
	void OnLoadAnim( wxCommandEvent &event );
	void OnSaveAnim( wxCommandEvent &event );
	void OnRemove( wxCommandEvent &event );
	void OnListDblClick( wxCommandEvent &event );
	void OnLoad( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnStoreAs( wxCommandEvent &event );
	void OnStore( wxCommandEvent &event );
	void OnRecall( wxCommandEvent &event );
	void OnLocList( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LocationDlg_H__

