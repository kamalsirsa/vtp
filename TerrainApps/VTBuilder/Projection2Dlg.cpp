//
// Name:		Projection2Dlg.cpp
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Projection2Dlg.h"
#include "StatePlaneDlg.h"
#include "Helper.h"
#include "vtui/wxString2.h"
#include "vtdata/vtLog.h"

//
// Must offset the values we use for Datum because a Choice
// control cannot handle "client data" values less than 0.
//
#define CHOICE_OFFSET 10

// WDR: class implementations

//---------------------------------------------------------------------------
// Projection2Dlg
//---------------------------------------------------------------------------

// WDR: event table for Projection2Dlg

BEGIN_EVENT_TABLE(Projection2Dlg, AutoDialog)
	EVT_INIT_DIALOG (Projection2Dlg::OnInitDialog)
	EVT_CHOICE( ID_PROJ, Projection2Dlg::OnProjChoice )
	EVT_BUTTON( ID_STATEPLANE, Projection2Dlg::OnSetStatePlane )
	EVT_CHOICE( ID_ZONE, Projection2Dlg::OnZone )
	EVT_CHOICE( ID_HORUNITS, Projection2Dlg::OnHorizUnits )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_PROJPARAM, Projection2Dlg::OnItemRightClick )
	EVT_CHOICE( ID_DATUM, Projection2Dlg::OnDatum )
	EVT_CHECKBOX( ID_SHOW_ALL_DATUMS, Projection2Dlg::OnShowAllDatums )
	EVT_BUTTON( ID_PROJ_LOAD, Projection2Dlg::OnProjLoad )
	EVT_BUTTON( ID_PROJ_SAVE, Projection2Dlg::OnProjSave )
END_EVENT_TABLE()

Projection2Dlg::Projection2Dlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	m_bInitializedUI = false;
	m_bShowAllDatums = false;

	m_GeoRefPoint.Set(0,0);

	ProjectionDialog2Func( this, TRUE ); 

	m_pParamCtrl = GetProjparam();
	m_pZoneCtrl = GetZonechoice();
	m_pHorizCtrl = GetHorizchoice();
	m_pDatumCtrl = GetDatumchoice();
	m_pProjCtrl = GetProjchoice();

	AddValidator(ID_PROJ, &m_iProj);
	AddValidator(ID_ZONE, &m_iZone);
	AddValidator(ID_SHOW_ALL_DATUMS, &m_bShowAllDatums);

	m_pProjCtrl->Append(_("Albers Equal Area Conic"));
	m_pProjCtrl->Append(_("Geographic"));
	m_pProjCtrl->Append(_("Lambert Azimuthal Equal-Area"));
	m_pProjCtrl->Append(_("Lambert Conformal Conic"));
	m_pProjCtrl->Append(_("New Zealand Map Grid"));
	m_pProjCtrl->Append(_("Oblique Stereographic"));
	m_pProjCtrl->Append(_("Polar Stereographic"));
	m_pProjCtrl->Append(_("Sinusoidal"));
	m_pProjCtrl->Append(_("Stereographic"));
	m_pProjCtrl->Append(_("Transverse Mercator"));
	m_pProjCtrl->Append(_("UTM"));
	m_pProjCtrl->Append(_("Dymaxion"));

	// Fill in choices for Datum
	RefreshDatums();

	m_pParamCtrl->ClearAll();
	m_pParamCtrl->InsertColumn(0, _("Attribute"));
	m_pParamCtrl->SetColumnWidth(0, 130);
	m_pParamCtrl->InsertColumn(1, _("Value"));
	m_pParamCtrl->SetColumnWidth(1, 85);

	m_bInitializedUI = true;
}

void Projection2Dlg::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
	SetUIFromProjection();
}

void Projection2Dlg::OnInitDialog(wxInitDialogEvent& event)
{
	SetUIFromProjection();
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

void Projection2Dlg::RefreshDatums()
{
	m_pDatumCtrl->Clear();

	wxString2 str;
	m_pDatumCtrl->Append(_("Unknown"), (void *) (-1+CHOICE_OFFSET));
	for (unsigned int i = 0; i < g_EPSGDatums.GetSize(); i++)
	{
		int code = g_EPSGDatums[i].iCode;
		str = g_EPSGDatums[i].szName;
		if (!m_bShowAllDatums)
		{
			if (!g_EPSGDatums[i].bCommon)
				continue;
		}
		m_pDatumCtrl->Append(str, (void *) (code+CHOICE_OFFSET));
	}
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
	wxString2 str;

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
	case PT_LCC:
	case PT_LAEA:
	case PT_NZMG:
	case PT_TM:
	case PT_SINUS:
	case PT_STEREO:
	case PT_OS:
	case PT_PS:
		m_pParamCtrl->Enable(true);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_DYMAX:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(false);
		break;
	}
	m_iDatum = m_proj.GetDatum();
	UpdateDatumStatus();

	// Do horizontal units ("linear units")
	m_pHorizCtrl->Clear();
	if (m_eProj == PT_GEO)
		m_pHorizCtrl->Append(_("Degrees"), (void *) LU_DEGREES);
	if (m_eProj != PT_GEO && m_eProj != PT_DYMAX)
		m_pHorizCtrl->Append(_("Meters"), (void *) LU_METERS);
	if (m_eProj != PT_GEO && m_eProj != PT_UTM && m_eProj != PT_DYMAX)
	{
		m_pHorizCtrl->Append(_("Feet (International)"), (void *) LU_FEET_INT);
		m_pHorizCtrl->Append(_("Feet (U.S. Survey)"), (void *) LU_FEET_US);
	}
	if (m_eProj == PT_DYMAX)
		m_pHorizCtrl->Append(_("Unit Edges"), (void *) LU_UNITEDGE);
	// manually transfer value
	for (i = 0; i < m_pHorizCtrl->GetCount(); i++)
	{
		if ((int) m_pHorizCtrl->GetClientData(i) == m_proj.GetUnits())
			m_pHorizCtrl->SetSelection(i);
	}

	DisplayProjectionSpecificParams();

	TransferDataToWindow();
}

void Projection2Dlg::UpdateDatumStatus()
{
	bool bIsCommon = false;
	for (unsigned int i = 0; i < g_EPSGDatums.GetSize(); i++)
	{
		if (g_EPSGDatums[i].iCode == m_iDatum)
			bIsCommon = g_EPSGDatums[i].bCommon;
	}
	// If we've got a rare datum, but the UI is set to show the short list,
	//  change it to show the full list instead.
	if (!bIsCommon && !m_bShowAllDatums)
	{
		m_bShowAllDatums = true;
		RefreshDatums();
		TransferDataToWindow();
	}
	wxString2 str = DatumToString(m_iDatum);
	m_pDatumCtrl->SetStringSelection(str);
}

void Projection2Dlg::DisplayProjectionSpecificParams()
{
	m_pParamCtrl->DeleteAllItems();

	OGR_SRSNode *root = m_proj.GetRoot();
	if (!root)
	{
		m_pParamCtrl->InsertItem(0, _("(Invalid projection)"));
		return;	 // bogus projection
	}

	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;

	wxString2 str;
	for (i = 0; i < children; i++)
	{
		node = root->GetChild(i);
		value = node->GetValue();
		if (!strcmp(value, "PARAMETER"))
		{
			par1 = node->GetChild(0);
			value = par1->GetValue();
			str = value;
			item = m_pParamCtrl->InsertItem(item, str);

			par2 = node->GetChild(1);
			value = par2->GetValue();
			str = value;
			m_pParamCtrl->SetItem(item, 1, str);
			item++;
		}
	}
}

void Projection2Dlg::SetUIFromProjection()
{
	if (m_proj.IsDymaxion())
		SetProjectionUI(PT_DYMAX);
	else if (m_proj.IsGeographic())
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

		// Supposedly, Gauss-Kruger is just a form of Transverse Mercator
		else if (!strcmp(proj_string, "Gauss_Kruger"))
			SetProjectionUI(PT_TM);

		else if (!strcmp(proj_string, SRS_PT_ALBERS_CONIC_EQUAL_AREA))
			SetProjectionUI(PT_ALBERS);

		else if (!strcmp(proj_string, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP))
			SetProjectionUI(PT_LCC);

		else if (!strcmp(proj_string, SRS_PT_LAMBERT_AZIMUTHAL_EQUAL_AREA))
			SetProjectionUI(PT_LAEA);

		else if (!strcmp(proj_string, SRS_PT_NEW_ZEALAND_MAP_GRID))
			SetProjectionUI(PT_NZMG);

		else if (!strcmp(proj_string, SRS_PT_SINUSOIDAL))
			SetProjectionUI(PT_SINUS);

		else if (!strcmp(proj_string, SRS_PT_STEREOGRAPHIC))
			SetProjectionUI(PT_STEREO);

		else if (!strcmp(proj_string, SRS_PT_OBLIQUE_STEREOGRAPHIC))
			SetProjectionUI(PT_OS);

		else if (!strcmp(proj_string, SRS_PT_POLAR_STEREOGRAPHIC))
			SetProjectionUI(PT_PS);

		// I've seen a .prj file for Stereo70 which refers to the projection
		//  as "Double_Stereographic", but this is unknown by OGR.  We do know
		//  about Oblique Stereographic, which is what i believe is meant.
		else if (!strcmp(proj_string, "Double_Stereographic"))
		{
		    OGR_SRSNode *node = m_proj.GetAttrNode("PROJECTION");
			node = node->GetChild(0);
			node->SetValue(SRS_PT_OBLIQUE_STEREOGRAPHIC);
			SetProjectionUI(PT_OS);
		}
		else
		{
			wxString2 str = _T("Unknown projection: ");
			str += proj_string;
			wxMessageBox(str);
		}
	}
}

void Projection2Dlg::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}


// WDR: handler implementations for Projection2Dlg

void Projection2Dlg::OnProjSave( wxCommandEvent &event )
{
	wxFileDialog saveFile(NULL, _("Save Projection to File"), _T(""), _T(""),
		_("Projection Files (*.prj)|*.prj|"), wxSAVE | wxOVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString2 strPathName = saveFile.GetPath();
	m_proj.WriteProjFile(strPathName.mb_str());
}

void Projection2Dlg::OnProjLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Projection from File"), _T(""), _T(""),
		_("Projection Files (*.prj)|*.prj|"), wxOPEN);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	wxString2 strPathName = loadFile.GetPath();
	m_proj.ReadProjFile(strPathName.mb_str());
	SetUIFromProjection();
}

void Projection2Dlg::OnDatum( wxCommandEvent &event )
{
	// operate on a copy for safety
	vtProjection copy = m_proj;
	int sel = event.GetInt();
	int datum_new = ((int) m_pDatumCtrl->GetClientData(sel)) - CHOICE_OFFSET;
	OGRErr err = copy.SetDatum(datum_new);
	if (err == OGRERR_NONE)
	{
		// succeeded
		m_proj = copy;
		m_iDatum = datum_new;
	}
	else
	{
		// Failed.  The OGR error message has already gone to the debug log
		//  via CPL.  We just need to inform the user via the GUI.
		wxMessageBox(_("Couldn't set that Datum.  Perhaps the EPSG\n tables could not be located.  Check that your\n GEOTIFF_CSV environment variable is set."));
		SetUIFromProjection();
	}
}

void Projection2Dlg::OnItemRightClick( wxListEvent &event )
{
	int item_clicked = event.GetIndex();

	OGR_SRSNode *root = m_proj.GetRoot();
	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;
	wxString2 str;

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
			wxString caption = _("Value for ");
			str = par1->GetValue();
			caption += str;
			str = value;
			wxString2 result = wxGetTextFromUser(caption, _("Enter new value"),
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

	// Even lightweight tasks can runs into trouble with the Locale ./, issue.
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	m_proj.SetGeogCSFromDatum(m_iDatum);

	m_eProj = (ProjType) m_iProj;

	if (m_eProj != PT_DYMAX)
		m_proj.SetDymaxion(false);

	switch (m_eProj)
	{
	case PT_GEO:
		// nothing more to do
		break;
	case PT_UTM:
		// To be polite, suggest a UTM zone based roughly on where the user
		//  might have some data.
		m_iZone = GuessZoneFromGeo(m_GeoRefPoint);
		m_proj.SetUTMZone(m_iZone);
		break;
	case PT_ALBERS:
		// Put in some default values
		m_proj.SetACEA( 60.0, 68.0, 59.0, -132.5, 500000, 500000 );
		break;
	case PT_LAEA:
		// Put in some default values
		m_proj.SetLAEA( 51, -150, 1000000, 0 );
		break;
	case PT_LCC:
		// Put in some default values
		m_proj.SetLCC( 10, 20, 0, 15, 0, 0 );
		break;
	case PT_NZMG:
		// Put in some default values
		m_proj.SetNZMG( 41, 173, 2510000, 6023150 );
		break;
	case PT_TM:
		// Put in some default values
		// These are for the OSGB projection, a common case
		m_proj.SetTM(49.0, -2.0, 0.999601272, 400000, -100000);
		break;
	case PT_SINUS:
		// Put in some default values
		m_proj.SetSinusoidal(0, 0, 0); // dfCenterLong, dfFalseEasting, dfFalseNorthing
		break;
	case PT_STEREO:
		// Put in some default values
		m_proj.SetStereographic( 0.0, 0.0, 1.0, 0.0, 0.0);
		break;
	case PT_OS:
		// Put in some default values
		// These are for Stereo70 (Romania)
		m_proj.SetOS(45.0, 25.0, 0.999750,500000, 500000);
		break;
	case PT_PS:
		// Put in some default values
		// These are for the IBCAO polar bathymetry
		m_proj.SetPS(90.0, 0.0, 1.0, 0.0, 0.0);
		break;
	case PT_DYMAX:
		m_proj.SetDymaxion(true);
		break;
	}

	SetProjectionUI( (ProjType) m_iProj );
}

void Projection2Dlg::OnShowAllDatums( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshDatums();
	UpdateDatumStatus();
}

void Projection2Dlg::AskStatePlane()
{
	// Pop up choices for State Plane
	StatePlaneDlg dialog(this, 201, _("Select State Plane"));
	if (dialog.ShowModal() != wxID_OK)
		return;

	StatePlaneInfo *plane_info = GetStatePlaneTable();
	int selection = dialog.m_iStatePlane;
	int bNAD83 = !dialog.m_bNAD27;
	int usgs_code = plane_info[selection].usgs_code;

	OGRErr result;
	if (dialog.m_bFeet)
		result = m_proj.SetStatePlane(usgs_code, bNAD83, SRS_UL_FOOT, GetMetersPerUnit(LU_FEET_INT));
	else if (dialog.m_bFeetUS)
		result = m_proj.SetStatePlane(usgs_code, bNAD83, SRS_UL_US_FOOT, GetMetersPerUnit(LU_FEET_US));
	else
		result = m_proj.SetStatePlane(usgs_code, bNAD83);

	if (result == OGRERR_FAILURE)
	{
		wxMessageBox(_("Couldn't set state plane projection.  Perhaps the\n EPSG tables could not be located.  Check that your\n GEOTIFF_CSV environment variable is set."));
	}
	else
	{
		SetUIFromProjection();
	}
}

