//
// Name: TileDlg.h
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TileDlg_H__
#define __TileDlg_H__

#include "VTBuilder_wdr.h"
#include "vtdata/ElevationGrid.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "Layer.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// TileDlg
//----------------------------------------------------------------------------

class TileDlg: public AutoDialog
{
public:
	// constructors and destructors
	TileDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TileDlg
	wxChoice* GetChoiceLod0Size()  { return (wxChoice*) FindWindow( ID_CHOICE_LOD0_SIZE ); }

	void SetTilingOptions(TilingOptions &opt);
	void GetTilingOptions(TilingOptions &opt) const;
	void SetArea(const DRECT &area);
	void UpdateInfo();
	void SetElevation(bool bElev);
	void SetView(BuilderView *pView) { m_pView = pView; }

	double m_fEstX;
	double m_fEstY;

private:
	// WDR: member variable declarations for TileDlg
	wxString2 m_strToFile;
	int m_iColumns;
	int m_iRows;
	int m_iLODChoice;
	int m_iLOD0Size;
	int m_iNumLODs;
	bool m_bElev;

	int m_iTotalX;
	int m_iTotalY;
	double m_fAreaX;
	double m_fAreaY;
	double m_fCurX;
	double m_fCurY;

	DRECT m_area;
	bool m_bSetting;
	BuilderView *m_pView;

private:
	// WDR: handler declarations for TileDlg
	void OnLODSize( wxCommandEvent &event );
	void OnSize( wxCommandEvent &event );
	void OnDotDotDot( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TileDlg_H__

