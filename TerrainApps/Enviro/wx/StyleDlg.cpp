//
// Name:		StyleDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "StyleDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "StyleDlg.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"
#include "vtdata/Features.h"
#include "vtui/wxString2.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StyleDlg
//----------------------------------------------------------------------------

// WDR: event table for StyleDlg

BEGIN_EVENT_TABLE(StyleDlg,AutoDialog)
	EVT_INIT_DIALOG (StyleDlg::OnInitDialog)
	EVT_BUTTON( ID_COLOR, StyleDlg::OnColor )
	EVT_CHECKBOX( ID_GEOMETRY, StyleDlg::OnCheck )
	EVT_CHECKBOX( ID_TEXT_LABELS, StyleDlg::OnCheck )
END_EVENT_TABLE()

StyleDlg::StyleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function StyleDialogFunc for StyleDlg
	StyleDialogFunc( this, TRUE ); 

	m_iTextField = 0;
	m_iColorField = 0;
	m_fLabelHeight = 0.0f;
	m_fLabelSize = 0.0f;

	AddValidator(ID_FEATURE_TYPE, &m_strFeatureType);

	AddValidator(ID_GEOMETRY, &m_bGeometry);

	AddValidator(ID_TEXT_LABELS, &m_bTextLabels);
	AddValidator(ID_TEXT_FIELD, &m_iTextField);
	AddValidator(ID_COLOR_FIELD, &m_iColorField);
	AddNumValidator(ID_LABEL_HEIGHT, &m_fLabelHeight);
	AddNumValidator(ID_LABEL_SIZE, &m_fLabelSize);
}

void StyleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	RefreshFields();
	wxDialog::OnInitDialog(event);
}

void StyleDlg::SetRawLayer(vtStringArray &datapaths, const vtTagArray &Layer)
{
	// for our purposes, we need the actual file location
	m_strFilename = Layer.GetValueString("Filename");

	RGBi label_color = Layer.GetValueRGBi("Color");
	if (!Layer.GetValueInt("TextFieldIndex", m_iTextField))
		m_iTextField = -1;
	if (!Layer.GetValueInt("ColorFieldIndex", m_iColorField))
		m_iColorField = -1;

	m_strResolved = m_strFilename;
	m_strResolved = FindFileOnPaths(datapaths, m_strResolved);
	if (m_strResolved == "")
	{
		vtString path = "PointData/";
		m_strResolved = path + m_strFilename;
		m_strResolved = FindFileOnPaths(datapaths, m_strResolved);
	}
}

void StyleDlg::GetRawLayer(vtTagArray &pLayer)
{
}

void StyleDlg::RefreshFields()
{
	GetTextField()->Clear();
	GetColorField()->Clear();
	GetColorField()->Append(_("(none)"));

	m_type = GetFeatureGeomType(m_strResolved);
	m_strFeatureType = OGRGeometryTypeToName(m_type);

	vtFeatureSetPoint2D dummy;
	if (dummy.LoadFieldInfoFromDBF(m_strResolved))
	{
		int i, num = dummy.GetNumFields();
		for (i = 0; i < num; i++)
		{
			Field *field = dummy.GetField(i);
			wxString2 field_name = field->m_name;
			GetTextField()->Append(field_name);
			GetColorField()->Append(field_name);
		}
		if (num)
		{
			if (m_iTextField < 0)
				m_iTextField = 0;
			if (m_iTextField > num-1)
				m_iTextField = num-1;
			if (m_iColorField < 0)
				m_iColorField = 0;
			if (m_iColorField > num-1)
				m_iColorField = num-1;
		}
	}
}

void StyleDlg::UpdateEnabling()
{
	GetColor()->Enable(m_bGeometry);

	GetTextField()->Enable();
	GetTextField()->Enable();
	GetTextField()->Enable();
	GetTextField()->Enable();
}


// WDR: handler implementations for StyleDlg

void StyleDlg::OnCheck( wxCommandEvent &event )
{
	UpdateEnabling();
}

void StyleDlg::OnColor( wxCommandEvent &event )
{
	
}




