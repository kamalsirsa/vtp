//
// Name: BuildingDlg.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __BuildingDlg_H__
#define __BuildingDlg_H__

#include "vtui_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/Building.h"
#include "vtdata/StructArray.h"

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
	wxTextCtrl* GetFeatures()  { return (wxTextCtrl*) FindWindow( ID_FEATURES ); }
	wxButton* GetLevelCopy()  { return (wxButton*) FindWindow( ID_LEVEL_COPY ); }
	wxButton* GetLevelDel()  { return (wxButton*) FindWindow( ID_LEVEL_DEL ); }
	wxButton* GetLevelDown()  { return (wxButton*) FindWindow( ID_LEVEL_DOWN ); }
	wxButton* GetLevelUp()  { return (wxButton*) FindWindow( ID_LEVEL_UP ); }
	wxStaticBitmap* GetColorBitmap1()  { return (wxStaticBitmap*) FindWindow( ID_COLOR1 ); }
	wxStaticBitmap* GetColorBitmap2()  { return (wxStaticBitmap*) FindWindow( ID_COLOR2 ); }
	wxListBox* GetLevelCtrl()  { return (wxListBox*) FindWindow( ID_LEVEL ); }
	wxListBox* GetEdgeCtrl()  { return (wxListBox*) FindWindow( ID_EDGE ); }
	void Setup(vtStructureArray *pSA, vtBuilding *bld, vtHeightField *pHeightField);
	void EditColor();
	void SetLevel(int i);
	void SetEdge(int i);
	void UpdateSlopes();
	void RefreshLevelsBox();
	void RefreshEdgesBox();
	void HighlightSelectedLevel();
	void HighlightSelectedEdge();
	void UpdateMaterialControl();
	void UpdateColorControl();
	void UpdateFeatures();
	void UpdateFacade();
	void SetupControls();
	void DeleteCurrentLevel();
	void CopyCurrentLevel();
	void SetEdgeFacade();
	void SetupValidators();

	// allow the dialog to control rendering (in case its being used w/3d)
	virtual void EnableRendering(bool bEnable) {}

	// notify subclasses when building is modified
	virtual void Modified() {}

protected:
	// WDR: member variable declarations for BuildingDlg
	vtStructureArray *m_pSA;
	vtBuilding  *m_pBuilding;
	vtLevel  *m_pLevel;
	vtEdge  *m_pEdge;
	vtHeightField *m_pHeightField;

	int  m_iLevel;
	int  m_iEdge;
	int  m_iStories;
	float   m_fStoryHeight;
	wxString2   m_strMaterial;
	wxString2   m_strEdgeSlopes;
	wxString2   m_strFeatures;
	wxString2   m_strFacade;
	wxColour	m_Color;
	int   m_iEdgeSlope;

	wxStaticBitmap  *m_pColorBitmapControl;
	wxListBox   *m_pLevelListBox;
	wxListBox   *m_pEdgeListBox;

	bool m_bSetting;
	bool m_bEdges;

protected:
	// WDR: handler declarations for BuildingDlg
	void OnModifyFacade( wxCommandEvent &event );
	void OnEditHeights( wxCommandEvent &event );
	void OnVertOffset( wxCommandEvent &event );
	void OnFacadeEnter( wxCommandEvent &event );
	void OnFeatDoor( wxCommandEvent &event );
	void OnFeatWindow( wxCommandEvent &event );
	void OnFeatWall( wxCommandEvent &event );
	void OnFeatClear( wxCommandEvent &event );
	void OnEdgeSlope( wxCommandEvent &event );
	void OnLevelDown( wxCommandEvent &event );
	void OnEdges( wxCommandEvent &event );
	void OnLevelDelete( wxCommandEvent &event );
	void OnLevelUp( wxCommandEvent &event );
	void OnLevelCopy( wxCommandEvent &event );
	void OnStoryHeight( wxCommandEvent &event );
	void OnSpinStories( wxSpinEvent &event );
	void OnColor1( wxCommandEvent &event );
	virtual void OnOK( wxCommandEvent &event );
	void OnCloseWindow(wxCloseEvent& event);
	void OnInitDialog(wxInitDialogEvent& event);
	void OnLevel( wxCommandEvent &event );
	void OnEdge( wxCommandEvent &event );
	void OnSetEdgeSlopes( wxCommandEvent &event );
	void OnSetMaterial( wxCommandEvent &event );
	void OnCharHook( wxKeyEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __BuildingDlg_H__

