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

#define PF_LOOP		1
#define PF_INTERP	2
#define PF_CONTIN	4
#define PF_POSONLY	8
#define PF_SPEED	16
#define PF_ALL		(PF_LOOP|PF_INTERP|PF_CONTIN|PF_POSONLY|PF_SPEED)

// WDR: class declarations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

class AnimEntry
{
public:
	AnimEntry() { m_pAnim = NULL; m_pEngine = NULL; }
	~AnimEntry() {
		delete m_pEngine; } // engine owns path
	vtAnimPath *m_pAnim;
	vtAnimPathEngine *m_pEngine;
	vtString m_Name;
};

class LocationDlg: public AutoDialog
{
public:
	// constructors and destructors
	LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~LocationDlg();

	// WDR: method declarations for LocationDlg
	wxTextCtrl* GetSpeed()  { return (wxTextCtrl*) FindWindow( ID_SPEED ); }
	wxTextCtrl* GetRecordSpacing()  { return (wxTextCtrl*) FindWindow( ID_RECORD_SPACING ); }
	wxRadioButton* GetRecordInterval()  { return (wxRadioButton*) FindWindow( ID_RECORD_INTERVAL ); }
	wxRadioButton* GetRecordLinear()  { return (wxRadioButton*) FindWindow( ID_RECORD_LINEAR ); }
	wxCheckBox* GetActive()  { return (wxCheckBox*) FindWindow( ID_ACTIVE ); }
	wxSlider* GetAnimPos()  { return (wxSlider*) FindWindow( ID_ANIM_POS ); }
	wxBitmapButton* GetReset()  { return (wxBitmapButton*) FindWindow( ID_RESET ); }
	wxCheckBox* GetLoop()  { return (wxCheckBox*) FindWindow( ID_LOOP ); }
	wxCheckBox* GetSmooth()  { return (wxCheckBox*) FindWindow( ID_SMOOTH ); }
	wxCheckBox* GetContinuous()  { return (wxCheckBox*) FindWindow( ID_CONTINUOUS ); }
	wxCheckBox* GetPosOnly()  { return (wxCheckBox*) FindWindow( ID_POS_ONLY ); }
	wxButton* GetSaveAnim()  { return (wxButton*) FindWindow( ID_SAVE_ANIM ); }
	wxButton* GetStop()  { return (wxButton*) FindWindow( ID_STOP ); }
	wxButton* GetRecord1()  { return (wxButton*) FindWindow( ID_RECORD1 ); }
	wxButton* GetPlay()  { return (wxButton*) FindWindow( ID_PLAY ); }
	wxListBox* GetAnims()  { return (wxListBox*) FindWindow( ID_ANIMS ); }
	wxButton* GetStoreas()  { return (wxButton*) FindWindow( ID_STOREAS ); }
	wxButton* GetStore()  { return (wxButton*) FindWindow( ID_STORE ); }
	wxButton* GetRecall()  { return (wxButton*) FindWindow( ID_RECALL ); }
	wxListBox* GetLoclist()  { return (wxListBox*) FindWindow( ID_LOCLIST ); }
	wxButton* GetRemove()  { return (wxButton*) FindWindow( ID_REMOVE ); }

	void Update();

	void RefreshList();
	void SetTarget(vtTransform *pTarget, const vtProjection &proj,
				   const vtLocalConversion &conv);
	void SetEngineContainer(vtEngine *pContainer);
	void SetLocFile(const vtString &fname);
	void RefreshButtons();
	void RecallFrom(const vtString &locname);

	void RefreshAnims();
	void RefreshAnimsText();
	void UpdateSlider();
	void UpdateEnabling();
	void SlidersToValues();
	void ValuesToSliders();
	void AppendAnimPath(vtAnimPath *anim, const char *name);

	void SetValues(int which = PF_ALL);
	void GetValues();
	void TransferToWindow();

private:
	// WDR: member variable declarations for LocationDlg
	vtLocationSaver *m_pSaver;

	wxListBox* m_pLocList;

	vtEngine *m_pContainer;
	Array<AnimEntry *> m_Entries;

	vtAnimPath *GetAnim(int i) { return m_Entries.GetAt(i)->m_pAnim; }
	vtAnimPathEngine *GetEngine(int i) { return m_Entries.GetAt(i)->m_pEngine; }

	bool m_bActive;
	bool m_bLoop;
	bool m_bContinuous;
	bool m_bSmooth;
	bool m_bPosOnly;
	int m_iAnim;
	int m_iPos;
	float m_fSpeed;
	int m_iSpeed;
	bool m_bSetting;
	float m_fRecordSpacing;
	bool m_bRecordLinear;
	bool m_bRecordInterval;

private:
	// WDR: handler declarations for LocationDlg
	void OnAnimPosSlider( wxCommandEvent &event );
	void OnActive( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnText( wxCommandEvent &event );
	void OnSpeedSlider( wxCommandEvent &event );
	void OnReset( wxCommandEvent &event );
	void OnCheckbox( wxCommandEvent &event );
	void OnAnim( wxCommandEvent &event );
	void OnStop( wxCommandEvent &event );
	void OnRecord1( wxCommandEvent &event );
	void OnPlay( wxCommandEvent &event );
	void OnLoadAnim( wxCommandEvent &event );
	void OnSaveAnim( wxCommandEvent &event );
	void OnNewAnim( wxCommandEvent &event );
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

