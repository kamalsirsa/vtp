//
// Name:		BuildingDlg.h
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __BuildingDlg_H__
#define __BuildingDlg_H__

#ifdef __GNUG__
	#pragma interface "BuildingDlg.cpp"
#endif

#include "vtui_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Building.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// BuildingDlg
//----------------------------------------------------------------------------

class BuildingDlg: public AutoDialog
{
public:
	// constructors and destructors
	BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for BuildingDlg
	wxButton* GetLevelCopy()  { return (wxButton*) FindWindow( ID_LEVEL_COPY ); }
	wxButton* GetLevelDel()  { return (wxButton*) FindWindow( ID_LEVEL_DEL ); }
	wxButton* GetLevelDown()  { return (wxButton*) FindWindow( ID_LEVEL_DOWN ); }
	wxButton* GetLevelUp()  { return (wxButton*) FindWindow( ID_LEVEL_UP ); }
	wxStaticBitmap* GetColorBitmap()  { return (wxStaticBitmap*) FindWindow( ID_COLOR ); }
	wxListBox* GetLevelCtrl()  { return (wxListBox*) FindWindow( ID_LEVEL ); }
	void Setup(vtBuilding *bld);
	void EditColor();
	void SetLevel(int i);
	void UpdateSlopes();
	void RefreshLevelsBox();
	void HighlightSelectedLevel();
	void UpdateColorControl();

	// allow the dialog to control rendering (in case its being used w/3d)
	virtual void EnableRendering(bool bEnable) {}

protected:
	// WDR: member variable declarations for BuildingDlg
	vtBuilding  *m_pBuilding;
	vtLevel     *m_pLevel;

	int		    m_iLevel;
	int		    m_iStories;
	float	    m_fStoryHeight;
	wxString	m_strMaterial;
	wxString	m_strEdgeSlopes;
	wxColour	m_Color;

	wxStaticBitmap  *m_pColorBitmapControl;
	wxListBox	*m_pLevelListBox;

	bool m_bSetting;

protected:
	// WDR: handler declarations for BuildingDlg
	void OnLevelDown( wxCommandEvent &event );
	void OnLevelDelete( wxCommandEvent &event );
	void OnLevelUp( wxCommandEvent &event );
	void OnLevelCopy( wxCommandEvent &event );
	void OnStoryHeight( wxCommandEvent &event );
	void OnSpinStories( wxCommandEvent &event );
	void OnColor1( wxCommandEvent &event );
	virtual void OnOK( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnLevel( wxCommandEvent &event );
	void OnSetEdgeSlopes( wxCommandEvent &event );
	void OnSetMaterial( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
