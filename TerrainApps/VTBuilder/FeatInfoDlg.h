//
// Name: FeatInfoDlg.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FeatInfoDlg_H__
#define __FeatInfoDlg_H__

#include "VTBuilder_wdr.h"
#include "RawLayer.h"
#include "vtui/AutoDialog.h"

class vtFeatures;
class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// FeatInfoDlg
//----------------------------------------------------------------------------

class FeatInfoDlg: public AutoDialog
{
public:
	// constructors and destructors
	FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetView(BuilderView *pView) { m_pView = pView; }
	void SetFeatureSet(vtFeatureSet *pFeatures);
	void SetLayer(vtLayer *pLayer) { m_pLayer = pLayer; }
	vtLayer *GetLayer() { return m_pLayer; }
	void Clear();
	void ShowSelected();
	void ShowPicked();
	void ShowAll();
	void ShowFeature(int iFeat);
	void UpdateFeatureText(int iItem, int iFeat);
	void RefreshItems();
	bool EditValue(int iFeature, int iColumn);

	// WDR: method declarations for FeatInfoDlg
	wxStaticText* GetTextVertical()  { return (wxStaticText*) FindWindow( ID_TEXT_VERTICAL ); }
	wxButton* GetDelHigh()  { return (wxButton*) FindWindow( ID_DEL_HIGH ); }
	wxChoice* GetChoiceVertical()  { return (wxChoice*) FindWindow( ID_CHOICE_VERTICAL ); }
	wxChoice* GetChoiceShow()  { return (wxChoice*) FindWindow( ID_CHOICE_SHOW ); }
	wxListCtrl* GetList()  { return (wxListCtrl*) FindWindow( ID_LIST ); }
	
private:
	// WDR: member variable declarations for FeatInfoDlg
	BuilderView *m_pView;
	vtLayer *m_pLayer;
	vtFeatureSet *m_pFeatures;
	int m_iShow;
	int m_iVUnits;
	int	m_iCoordColumns;

	bool m_bGeo;	// true if coordinates are geographic (lon-lat)

private:
	// WDR: handler declarations for FeatInfoDlg
	void OnItemRightClick( wxListEvent &event );
	void OnLeftDClick( wxMouseEvent &event );
	void OnDeleteHighlighted( wxCommandEvent &event );
	void OnChoiceVertical( wxCommandEvent &event );
	void OnChoiceShow( wxCommandEvent &event );
	void OnListRightClick( wxListEvent &event );
	void OnItemSelected( wxListEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
