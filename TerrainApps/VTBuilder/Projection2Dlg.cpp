//
// Name:		Projection2Dlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "Projection2Dlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Projection2Dlg.h"
#include "StatePlaneDlg.h"

//
// Must offset the values we use for Datum because a Choice
// control cannot handle "client data" values less than 0.
//
#define CHOICE_OFFSET 10

// WDR: class implementations

//----------------------------------------------------------------------------
// Projection2Dlg
//----------------------------------------------------------------------------

// WDR: event table for Projection2Dlg

BEGIN_EVENT_TABLE(Projection2Dlg,AutoDialog)
	EVT_CHOICE( ID_PROJ, Projection2Dlg::OnProjChoice )
	EVT_BUTTON( ID_STATEPLANE, Projection2Dlg::OnSetStatePlane )
	EVT_CHOICE( ID_ZONE, Projection2Dlg::OnZone )
	EVT_CHOICE( ID_HORUNITS, Projection2Dlg::OnHorizUnits )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_PROJPARAM, Projection2Dlg::OnItemRightClick )
	EVT_CHOICE( ID_DATUM, Projection2Dlg::OnDatum )
END_EVENT_TABLE()

Projection2Dlg::Projection2Dlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bInitializedUI = false;
	ProjectionDialog2Func( this, TRUE ); 
}

void Projection2Dlg::SetProjection(const vtProjection &proj)
{
	m_proj = proj;

	SetUIFromProjection();
}

void Projection2Dlg::OnInitDialog(wxInitDialogEvent& event)
{
	int i;

	m_pParamCtrl = GetProjparam();
	m_pZoneCtrl = GetZonechoice();
	m_pHorizCtrl = GetHorizchoice();
	m_pDatumCtrl = GetDatumchoice();
	m_pProjCtrl = GetProjchoice();

	AddValidator(ID_PROJ, &m_iProj);
	AddValidator(ID_ZONE, &m_iZone);

	m_pProjCtrl->Append(_T("Geographic"));
	m_pProjCtrl->Append(_T("UTM"));
	m_pProjCtrl->Append(_T("Albers Equal Area Conic"));
	m_pProjCtrl->Append(_T("Lambert Conformal Conic"));
	m_pProjCtrl->Append(_T("Transverse Mercator"));

	// Fill in choices for Datum
	wxString str;
	for (i = NO_DATUM; i <= WGS_84; i++)
	{
		str = wxString::FromAscii(datumToString((DATUM) i));
		m_pDatumCtrl->Append(str, (void *) (i+CHOICE_OFFSET));
	}

	m_pParamCtrl->ClearAll();
	m_pParamCtrl->InsertColumn(0, _T("Attribute"));
	m_pParamCtrl->SetColumnWidth(0, 130);
	m_pParamCtrl->InsertColumn(1, _T("Value"));
	m_pParamCtrl->SetColumnWidth(1, 85);

	m_bInitializedUI = true;

	SetUIFromProjection();

	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

// Re-arrange the UI for a given projection
void Projection2Dlg::SetProjectionUI(ProjType type)
{
	m_eProj = type;
	m_iProj = m_eProj;

	if (m_bInitializedUI)
		UpdateControlStatus();
}

void Projection2Dlg::UpdateControlStatus()
{
	int i, pos = 0;
	int real_zone;
	wxString str;

	m_pZoneCtrl->Clear();
	switch (m_eProj)
	{
	case PT_GEO:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_UTM:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(true);
		real_zone = m_proj.GetUTMZone();
		for (i = -60; i <= -1; i++)
		{
			str.Printf(_T("Zone %d"), i);
			m_pZoneCtrl->Append(str, (void *) (i+100));
			if (i == real_zone)
				m_iZone = pos;
			pos++;
		}
		for (i = 1; i <= 60; i++)
		{
			str.Printf(_T("Zone %d"), i);
			m_pZoneCtrl->Append(str, (void *) (i+100));
			if (i == real_zone)
				m_iZone = pos;
			pos++;
		}
		break;
	case PT_ALBERS:
		m_pParamCtrl->Enable(true);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_LAMBERT:
		m_pParamCtrl->Enable(true);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_TM:
		m_pParamCtrl->Enable(true);
		m_pZoneCtrl->Enable(false);
		break;
	}
	m_iDatum = (int) m_proj.GetDatum();
	str = wxString::FromAscii(datumToString((DATUM)m_iDatum));
	m_pDatumCtrl->SetStringSelection(str);

	// Do horizontal units ("linear units")
	m_pHorizCtrl->Clear();
	if (m_eProj == PT_GEO)
		m_pHorizCtrl->Append(_T("Degrees"), (void *) 0);
	if (m_eProj != PT_GEO)
		m_pHorizCtrl->Append(_T("Meters"), (void *) 1);
	if (m_eProj != PT_GEO && m_eProj != PT_UTM)
	{
		m_pHorizCtrl->Append(_T("Feet (International)"), (void *) 2);
		m_pHorizCtrl->Append(_T("Feet (U.S. Survey)"), (void *) 3);
	}
	// manually transfer value
	for (i = 0; i < m_pHorizCtrl->Number(); i++)
	{
		if ((int) m_pHorizCtrl->GetClientData(i) == m_proj.GetUnits())
			m_pHorizCtrl->SetSelection(i);
	}

	DisplayProjectionSpecificParams();

	TransferDataToWindow();
}

void Projection2Dlg::DisplayProjectionSpecificParams()
{
	m_pParamCtrl->DeleteAllItems();
	switch (m_eProj)
	{
	case PT_GEO:
	case PT_UTM:
		break;
	case PT_ALBERS:
		break;
	case PT_LAMBERT:
		break;
	}
	OGR_SRSNode *root = m_proj.GetRoot();
	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;

	wxString str;
	for (i = 0; i < children; i++)
	{
		node = root->GetChild(i);
		value = node->GetValue();
		if (!strcmp(value, "PARAMETER"))
		{
			par1 = node->GetChild(0);
			value = par1->GetValue();
			str = wxString::FromAscii(value);
			item = m_pParamCtrl->InsertItem(item, str);

			par2 = node->GetChild(1);
			value = par2->GetValue();
			str = wxString::FromAscii(value);
			m_pParamCtrl->SetItem(item, 1, str);
			item++;
		}
	}
}

void Projection2Dlg::SetUIFromProjection()
{
	if (m_proj.IsGeographic())
		SetProjectionUI(PT_GEO);
	else
	{
		const char *proj_string = m_proj.GetProjectionName();

		if (!strcmp(proj_string, SRS_PT_TRANSVERSE_MERCATOR))
		{
			if (m_proj.GetUTMZone() != 0)
				SetProjectionUI(PT_UTM);
			else
				SetProjectionUI(PT_TM);
		}
		if (!strcmp(proj_string, SRS_PT_ALBERS_CONIC_EQUAL_AREA))
		{
			SetProjectionUI(PT_ALBERS);
		}
		if (!strcmp(proj_string, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP))
		{
			SetProjectionUI(PT_LAMBERT);
		}
	}
}

void Projection2Dlg::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}


// WDR: handler implementations for Projection2Dlg

void Projection2Dlg::OnDatum( wxCommandEvent &event )
{
	int sel = event.GetInt();
	m_iDatum = ((int) m_pDatumCtrl->GetClientData(sel)) - CHOICE_OFFSET;
	m_proj.SetDatum((DATUM) m_iDatum);
}

void Projection2Dlg::OnItemRightClick( wxListEvent &event )
{
	int item_clicked = event.GetIndex();

	OGR_SRSNode *root = m_proj.GetRoot();
	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;
	wxString str;

	for (i = 0; i < children; i++)
	{
		node = root->GetChild(i);
		value = node->GetValue();
		if (strcmp(value, "PARAMETER"))
			continue;
		par1 = node->GetChild(0);
		par2 = node->GetChild(1);
		value = par2->GetValue();
		if (item == item_clicked)
		{
			wxString caption = _T("Value for ");
			str = wxString::FromAscii(par1->GetValue());
			caption += str;
			str = wxString::FromAscii(value);
			wxString result = wxGetTextFromUser(caption, _T("Enter new value"),
				str, this);
			if (result != _T(""))
			{
//			  double newval = atof((const char *)result);
				par2->SetValue(result.mb_str());
				DisplayProjectionSpecificParams();
				return;
			}
		}
		item++;
	}
}

void Projection2Dlg::OnHorizUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();

	m_iUnits = m_pHorizCtrl->GetSelection();

	LinearUnits previous = m_proj.GetUnits();
	LinearUnits iUnits = (LinearUnits) (int) m_pHorizCtrl->GetClientData(m_iUnits);

	if (iUnits == LU_METERS)
	{
		m_proj.SetLinearUnits(SRS_UL_METER, 1.0);
	}
	if (iUnits == LU_FEET_INT)
	{
		m_proj.SetLinearUnits(SRS_UL_FOOT, GetMetersPerUnit(iUnits));
	}
	if (iUnits == LU_FEET_US)
	{
		m_proj.SetLinearUnits(SRS_UL_US_FOOT, GetMetersPerUnit(iUnits));
	}

	TransferDataToWindow();
	UpdateControlStatus();
}

void Projection2Dlg::OnZone( wxCommandEvent &event )
{
	TransferDataFromWindow();

	void *vval = m_pZoneCtrl->GetClientData(m_iZone);
	int val = (int) vval - 100;
	m_proj.SetUTMZone(val);

	UpdateControlStatus();
}

void Projection2Dlg::OnSetStatePlane( wxCommandEvent &event )
{
	AskStatePlane();
}

void Projection2Dlg::OnProjChoice( wxCommandEvent &event )
{
	TransferDataFromWindow();

	m_proj.SetGeogCSFromDatum((DATUM) m_iDatum);

	m_eProj = (ProjType) m_iProj;
	switch (m_eProj)
	{
	case PT_GEO:
		// nothing more to do
		break;
	case PT_UTM:
		m_proj.SetUTMZone(1);
		break;
	case PT_ALBERS:
		m_proj.SetACEA( 60.0, 68.0, 59.0, -132.5, 500000, 500000 );
		break;
	case PT_LAMBERT:
		m_proj.SetLAEA( 51, -150, 1000000, 0 );
		break;
	case PT_TM:
		// Put in some default values
		// These are for the OSGB projection, a common case
		m_proj.SetTM(49.0, -2.0, 0.999601272, 400000, -100000);
		break;
	}

	SetProjectionUI( (ProjType) m_iProj );
}

void Projection2Dlg::AskStatePlane()
{
	// Pop up choices for State Plane
	StatePlaneDlg dialog(this, 201, _T("Select State Plane"));
	if (dialog.ShowModal() != wxID_OK)
		return;

	StatePlaneInfo *plane_info = GetStatePlaneTable();
	int selection = dialog.m_iStatePlane;
	int bNAD83 = !dialog.m_bNAD27;
	int usgs_code = plane_info[selection].usgs_code;

	OGRErr result = m_proj.SetStatePlane(usgs_code, bNAD83);
	if (result == OGRERR_FAILURE)
	{
		wxMessageBox(_T("Couldn't set state plane projection.  Perhaps the\n")
			_T("EPSG tables could not be located.  Check that your\n")
			_T("GEOTIFF_CSV environment variable is set."));
	}
	else
	{
		SetUIFromProjection();
	}
}
