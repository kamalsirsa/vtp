//
// Name: DistanceDlg.cpp
//
// Copyright (c) 2002-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DistanceDlg.h"
#include "Helper.h" // for FormatCoord
#include <float.h>	// for FLT_MIN

// WDR: class implementations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

// WDR: event table for DistanceDlg

BEGIN_EVENT_TABLE(DistanceDlg, AutoDialog)
	EVT_INIT_DIALOG (DistanceDlg::OnInitDialog)
	EVT_CHOICE( ID_UNITS1, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS2, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS3, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS4, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS5, DistanceDlg::OnUnits )
END_EVENT_TABLE()

DistanceDlg::DistanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	DistanceDialogFunc( this, TRUE );

	GetUnits2()->Append(_("Meters"));
	GetUnits2()->Append(_("Feet"));
	GetUnits2()->Append(_("US Survey Feet"));

	GetUnits3()->Append(_("Meters"));
	GetUnits3()->Append(_("Feet"));
	GetUnits3()->Append(_("US Survey Feet"));
	GetUnits3()->Append(_("Kilometers"));
	GetUnits3()->Append(_("Miles"));

	GetUnits4()->Append(_("Meters"));
	GetUnits4()->Append(_("Feet"));
	GetUnits4()->Append(_("US Survey Feet"));
	GetUnits4()->Append(_("Kilometers"));
	GetUnits4()->Append(_("Miles"));

	GetUnits5()->Append(_("Meters"));
	GetUnits5()->Append(_("Feet"));
	GetUnits5()->Append(_("US Survey Feet"));

	AddValidator(ID_UNITS1, &m_iUnits1);
	AddValidator(ID_UNITS2, &m_iUnits2);
	AddValidator(ID_UNITS3, &m_iUnits3);
	AddValidator(ID_UNITS4, &m_iUnits4);
	AddValidator(ID_UNITS5, &m_iUnits5);
}

void DistanceDlg::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
	GetMapOffset()->SetValue(_T(""));
	GetMapDist()->SetValue(_T(""));
	GetGeodDist()->SetValue(_T(""));
	UpdateAvailableUnits();
}

void DistanceDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2, bool bUpdate)
{
	m_p1 = p1;
	m_p2 = p2;
	if (bUpdate)
		ShowValues();
}

void DistanceDlg::GetPoints(DPoint2 &p1, DPoint2 &p2)
{
	p1 = m_p1;
	p2 = m_p2;
}

void DistanceDlg::SetGroundAndVertical(float fGround, float fVertical, bool bUpdate)
{
	m_fGround = fGround;
	m_fVertical = fVertical;
	if (bUpdate)
		ShowValues();
}

double GetScaleFromUnits(int units)
{
	switch (units)
	{
	case 3:
		return 1.0 / 1000;	// km
	case 4:
		return 1.0 / (5280 * GetMetersPerUnit(LU_FEET_INT)); // miles
	}
	// otherwise, normal linear units
	return 1.0 / GetMetersPerUnit((LinearUnits)(units+1));
}

void DistanceDlg::ShowValues()
{
	DPoint2 diff_degrees;
	DPoint2 diff_map = m_p2 - m_p1;

	bool bIsGeo = (m_proj.IsGeographic() != FALSE);
	DPoint2 geo1, geo2;
	if (bIsGeo)
	{
		diff_degrees = diff_map;
		geo1 = m_p1;
		geo2 = m_p2;
	}
	else
	{
		// We need degrees, but don't have them, so compute them.
		vtProjection geo;
		CreateSimilarGeographicProjection(m_proj, geo);

		OCT *trans = CreateCoordTransform(&m_proj, &geo);
		if (!trans)
		{
			// This should never happen, unless something is majorly wrong,
			//  like proj.dll is not found.
			GetMapOffset()->SetValue(_("<Projection failure>"));
			return;
		}

		geo1 = m_p1;
		geo2 = m_p2;
		trans->Transform(1, &geo1.x, &geo1.y);
		trans->Transform(1, &geo2.x, &geo2.y);

		diff_degrees = geo2 - geo1;

		delete trans;
	}
	// find geodesic distance
	double geodesic_meters = vtProjection::GeodesicDistance(geo1, geo2);

	LinearUnits lu = m_proj.GetUnits();

	// Map Offset
	wxString str;
	double scale;

	if (m_iUnits1 == 0) // degrees
	{
		str = wxString(FormatCoord(true, diff_degrees.x), wxConvUTF8);
		str += _T(", ");
		str += wxString(FormatCoord(true, diff_degrees.y), wxConvUTF8);
	}
	else
	{
		scale = GetMetersPerUnit(lu) /
			GetMetersPerUnit((LinearUnits)m_iUnits1);
		DPoint2 diff_show = diff_map * scale;
		str = wxString(FormatCoord(bIsGeo, diff_show.x), wxConvUTF8);
		str += _T(", ");
		str += wxString(FormatCoord(bIsGeo, diff_show.y), wxConvUTF8);
	}
	GetMapOffset()->SetValue(str);

	// Map Distance
	if (bIsGeo)
		str = _T("N/A");
	else
	{
		scale = GetMetersPerUnit(lu) /
			GetMetersPerUnit((LinearUnits)(m_iUnits2+1));
		str = wxString(FormatCoord(false, diff_map.Length() * scale), wxConvUTF8);
	}
	GetMapDist()->SetValue(str);

	// Geodesic Distance
	scale = GetScaleFromUnits(m_iUnits3);
	str = wxString(FormatCoord(false, geodesic_meters * scale), wxConvUTF8);
	GetGeodDist()->SetValue(str);

	// Approximate Ground Distance
	bool bShowGround = (m_fGround != FLT_MIN);
	GetGroundDist()->Enable(bShowGround);
	GetUnits4()->Enable(bShowGround);
	if (bShowGround)
	{
		scale = GetScaleFromUnits(m_iUnits4);
		str = wxString(FormatCoord(false, m_fGround * scale), wxConvUTF8);
		GetGroundDist()->SetValue(str);
	}
	else
		GetGroundDist()->SetValue(_T("N/A"));

	// Vertical Difference
	bool bShowVertical = (m_fVertical != FLT_MIN);
	GetVertical()->Enable(bShowVertical);
	GetUnits5()->Enable(bShowVertical);
	if (bShowVertical)
	{
		scale = GetScaleFromUnits(m_iUnits5);
		str = wxString(FormatCoord(false, m_fVertical * scale), wxConvUTF8);
		GetVertical()->SetValue(str);
	}
	else
		GetVertical()->SetValue(_T("N/A"));
}

void DistanceDlg::UpdateAvailableUnits()
{
	bool bIsGeo = (m_proj.IsGeographic() != FALSE);
	GetUnits2()->Enable(!bIsGeo);
	GetMapDist()->Enable(!bIsGeo);

	GetUnits1()->Clear();
	GetUnits1()->Append(_("Degrees"));
	if (!bIsGeo)
	{
		GetUnits1()->Append(_("Meters"));
		GetUnits1()->Append(_("Feet"));
		GetUnits1()->Append(_("US Survey Feet"));
	}

	switch (m_proj.GetUnits())
	{
	case LU_DEGREES:
		m_iUnits1 = 0;
		m_iUnits3 = 0;
		break;
	case LU_METERS:
		m_iUnits1 = 1;
		m_iUnits2 = 0;
		m_iUnits3 = 0;
		break;
	case LU_FEET_INT:
		m_iUnits1 = 2;
		m_iUnits2 = 1;
		m_iUnits3 = 1;
		break;
	case LU_FEET_US:
		m_iUnits1 = 3;
		m_iUnits2 = 2;
		m_iUnits3 = 2;
		break;
	case LU_UNITEDGE:
		// TODO
	case LU_UNKNOWN:
		break;
	}
	m_iUnits4 = m_iUnits3;
	m_iUnits5 = 0;
	TransferDataToWindow();
}

// WDR: handler implementations for DistanceDlg

void DistanceDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateAvailableUnits();
	wxDialog::OnInitDialog(event);
}

void DistanceDlg::OnUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ShowValues();
}

