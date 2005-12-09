//
// Name:		Projection2Dlg.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __Projection2Dlg_H__
#define __Projection2Dlg_H__

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Projections.h"

enum ProjType
{
	PT_ALBERS,
	PT_GEO,
	PT_HOM,		// Hotine Oblique Mercator
	PT_LAEA,	// Lambert Azimuthal Equal-Area
	PT_LCC,		// Lambert Conformal Conic
	PT_NZMG,
	PT_OS,
	PT_PS,
	PT_SINUS,
	PT_STEREO,
	PT_TM,
	PT_UTM,
	PT_DYMAX
};

// WDR: class declarations

//----------------------------------------------------------------------------
// Projection2Dlg
//----------------------------------------------------------------------------

class Projection2Dlg: public AutoDialog
{
public:
	// constructors and destructors
	Projection2Dlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void SetProjection(const vtProjection &proj);
	void GetProjection(vtProjection &proj);
	void SetGeoRefPoint(const DPoint2 &p) { m_GeoRefPoint = p; }

protected:
	// WDR: method declarations for Projection2Dlg
	wxListCtrl* GetProjparam()  { return (wxListCtrl*) FindWindow( ID_PROJPARAM ); }
	wxChoice* GetHorizchoice()  { return (wxChoice*) FindWindow( ID_HORUNITS ); }
	wxChoice* GetZonechoice()  { return (wxChoice*) FindWindow( ID_ZONE ); }
	wxChoice* GetDatumchoice()  { return (wxChoice*) FindWindow( ID_DATUM ); }
	wxChoice* GetProjchoice()  { return (wxChoice*) FindWindow( ID_PROJ ); }

	void SetUIFromProjection();
	void SetProjectionUI(ProjType type);
	void UpdateControlStatus();
	void DisplayProjectionSpecificParams();
	void AskStatePlane();
	void RefreshDatums();
	void UpdateDatumStatus();

private:
	// WDR: member variable declarations for Projection2Dlg
	wxListCtrl  *m_pParamCtrl;
	wxChoice	*m_pZoneCtrl;
	wxChoice	*m_pHorizCtrl;
	wxChoice	*m_pDatumCtrl;
	wxChoice	*m_pProjCtrl;

	vtProjection	m_proj;
	ProjType		m_eProj;
	int		m_iDatum;
	int		m_iZone;
	int		m_iProj;
	int		m_iUnits;
	bool	m_bShowAllDatums;
	bool	m_bInitializedUI;
	DPoint2 m_GeoRefPoint;

private:
	// WDR: handler declarations for Projection2Dlg
	void OnProjSave( wxCommandEvent &event );
	void OnProjLoad( wxCommandEvent &event );
	void OnDatum( wxCommandEvent &event );
	void OnItemRightClick( wxListEvent &event );
	void OnHorizUnits( wxCommandEvent &event );
	void OnZone( wxCommandEvent &event );
	void OnSetStatePlane( wxCommandEvent &event );
	void OnProjChoice( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnShowAllDatums( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};


#endif

