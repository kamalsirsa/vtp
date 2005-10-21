//
// LODDlg.h
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LODDlg_H__
#define __LODDlg_H__

#include "enviro_wdr.h"
class EnviroFrame;

// WDR: class declarations

//----------------------------------------------------------------------------
// LODDlg
//----------------------------------------------------------------------------

class LODDlg: public wxDialog
{
public:
	// constructors and destructors
	LODDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for LODDlg
	wxTextCtrl* GetCacheUsed()  { return (wxTextCtrl*) FindWindow( ID_CACHE_USED ); }
	wxTextCtrl* GetTileHits()  { return (wxTextCtrl*) FindWindow( ID_TILE_HITS ); }
	wxTextCtrl* GetTileLoads()  { return (wxTextCtrl*) FindWindow( ID_TILE_LOADS ); }
	wxSpinCtrl* GetTarget()  { return (wxSpinCtrl*) FindWindow( ID_TARGET ); }
	wxTextCtrl* GetCurrent()  { return (wxTextCtrl*) FindWindow( ID_CURRENT ); }
	wxPanel* GetPanel1()  { return (wxPanel*) FindWindow( ID_PANEL1 ); }

	void Refresh(float res0, float res, float res1, int target, int count,
		int cache_size, int cache_used, int disk_loads, int cache_hits);
	void DrawChart(float res0, float res, float res1, int target, int count);
	void SetFrame(EnviroFrame *pFrame) { m_pFrame = pFrame; }

private:
	// WDR: member variable declarations for LODDlg
	int m_iTarget;
	int m_iCacheSize, m_iCacheUsed, m_iDiskLoads, m_iCacheHits;
	EnviroFrame *m_pFrame;
	
private:
	// WDR: handler declarations for LODDlg
	void OnSpinTargetUp( wxSpinEvent &event );
	void OnSpinTargetDown( wxSpinEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LODDlg_H__
