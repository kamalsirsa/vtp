//
// Name: DistanceDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "DistanceDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wxString2.h"
#include "DistanceDlg.h"
#include "Helper.h"	// for FormatCoord

// WDR: class implementations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

// WDR: event table for DistanceDlg

BEGIN_EVENT_TABLE(DistanceDlg,AutoDialog)
	EVT_CHOICE( ID_UNITS1, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS2, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS3, DistanceDlg::OnUnits )
END_EVENT_TABLE()

DistanceDlg::DistanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_pProj = NULL;
	DistanceDialogFunc( this, TRUE ); 
}

void DistanceDlg::SetProjection(vtProjection *proj)
{
	m_pProj = proj;
	GetMapOffset()->SetValue(_T(""));
	GetMapDist()->SetValue(_T(""));
	GetGeodDist()->SetValue(_T(""));
	SetAvailableUnits();
}

void DistanceDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2)
{
	m_p1 = p1;
	m_p2 = p2;
	ShowValues();
}

void DistanceDlg::ShowValues()
{
	DPoint2 diff_degrees;
	DPoint2 diff_map = m_p2 - m_p1;

	bool bIsGeo = (m_pProj->IsGeographic() != FALSE);
	DPoint2 geo1, geo2;
	if (bIsGeo)
	{
		diff_degrees = diff_map;
		geo1 = m_p1;
		geo2 = m_p2;
	}
	else
	{
		vtProjection geo;
		CreateSimilarGeographicProjection(*m_pProj, geo);

		OCT *trans = OGRCreateCoordinateTransformation(m_pProj, &geo);

		geo1 = m_p1;
		geo2 = m_p2;
		trans->Transform(1, &geo1.x, &geo1.y);
		trans->Transform(1, &geo2.x, &geo2.y);

		diff_degrees = geo2 - geo1;

		delete trans;
	}
	// find geodesic distance
	double geodesic_meters = m_pProj->GeodesicDistance(geo1, geo2);

	LinearUnits lu = m_pProj->GetUnits();

	// Map Offset
	wxString2 str;
	double scale;

	if (m_iUnits1 == 0)	// degrees
	{
		str.Printf(_T("%s, %s"),
			(const char *) FormatCoord(true, diff_degrees.x),
			(const char *) FormatCoord(true, diff_degrees.y));
	}
	else
	{
		scale = GetMetersPerUnit(lu) /
			GetMetersPerUnit((LinearUnits)m_iUnits1);
		DPoint2 diff_show = diff_map * scale;
		str.Printf(_T("%s, %s"),
			(const char *) FormatCoord(bIsGeo, diff_show.x),
			(const char *) FormatCoord(bIsGeo, diff_show.y));
	}
	GetMapOffset()->SetValue(str);

	// Map Distance
	if (bIsGeo)
		str = _T("N/A");
	else
	{
		scale = GetMetersPerUnit(lu) /
			GetMetersPerUnit((LinearUnits)(m_iUnits2+1));
		str = FormatCoord(false, diff_map.Length() * scale);
	}
	GetMapDist()->SetValue(str);

	// Geodesic Distance
	switch (m_iUnits3)
	{
	case 0:
	case 1:
	case 2:
		scale = 1.0 / GetMetersPerUnit((LinearUnits)(m_iUnits3+1));
		break;
	case 3:
		scale = 1.0 / 1000;
		break;
	case 4:
		scale = 1.0 / (5280 * GetMetersPerUnit(LU_FEET_INT));
		break;
	}
	str = FormatCoord(false, geodesic_meters * scale);
	GetGeodDist()->SetValue(str);
}

void DistanceDlg::SetAvailableUnits()
{
	bool bIsGeo = (m_pProj->IsGeographic() != FALSE);
	GetUnits2()->Enable(!bIsGeo);
	GetMapDist()->Enable(!bIsGeo);

	GetUnits1()->Clear();
	GetUnits1()->Append(_T("Degrees"));
	if (!bIsGeo)
	{
		GetUnits1()->Append(_T("Meters"));
		GetUnits1()->Append(_T("Feet"));
		GetUnits1()->Append(_T("US Survey Feet"));
	}

	switch (m_pProj->GetUnits())
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
	}
	TransferDataToWindow();
}

// WDR: handler implementations for DistanceDlg

void DistanceDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetUnits2()->Clear();
	GetUnits2()->Append(_T("Meters"));
	GetUnits2()->Append(_T("Feet"));
	GetUnits2()->Append(_T("US Survey Feet"));

	GetUnits3()->Clear();
	GetUnits3()->Append(_T("Meters"));
	GetUnits3()->Append(_T("Feet"));
	GetUnits3()->Append(_T("US Survey Feet"));
	GetUnits3()->Append(_T("Kilometers"));
	GetUnits3()->Append(_T("Miles"));

	AddValidator(ID_UNITS1, &m_iUnits1);
	AddValidator(ID_UNITS2, &m_iUnits2);
	AddValidator(ID_UNITS3, &m_iUnits3);

	SetAvailableUnits();

	wxDialog::OnInitDialog(event);
}

void DistanceDlg::OnUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ShowValues();
}

