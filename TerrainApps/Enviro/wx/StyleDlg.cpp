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

#include "wx/colordlg.h"

#include "StyleDlg.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"
#include "vtdata/Features.h"
#include "vtui/wxString2.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StyleDlg
//----------------------------------------------------------------------------

// WDR: event table for StyleDlg

BEGIN_EVENT_TABLE(StyleDlg,AutoDialog)
	EVT_INIT_DIALOG (StyleDlg::OnInitDialog)
	EVT_BUTTON( ID_GEOM_COLOR, StyleDlg::OnGeomColor )
	EVT_BUTTON( ID_LABEL_COLOR, StyleDlg::OnLabelColor )
	EVT_CHECKBOX( ID_GEOMETRY, StyleDlg::OnCheck )
	EVT_CHECKBOX( ID_TEXT_LABELS, StyleDlg::OnCheck )
END_EVENT_TABLE()

StyleDlg::StyleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function StyleDialogFunc for StyleDlg
	StyleDialogFunc( this, TRUE ); 

	m_bGeometry = true;
	m_GeomColor.Set(255,255,255);

	m_bTextLabels = false;
	m_LabelColor.Set(255,255,255);
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
	UpdateEnabling();
	UpdateColorButtons();
	wxDialog::OnInitDialog(event);
}

void StyleDlg::SetOptions(vtStringArray &datapaths, const vtTagArray &Layer)
{
	// for our purposes, we need the actual file location
	m_strFilename = Layer.GetValueString("Filename");

	m_bGeometry = Layer.GetValueBool("Geometry");
	if (!Layer.GetValueRGBi("GeomColor", m_GeomColor))
		m_GeomColor.Set(255,255,255);

	m_bTextLabels = Layer.GetValueBool("Labels");
	if (!Layer.GetValueRGBi("LabelColor", m_LabelColor))
		m_LabelColor.Set(255,255,255);

	if (!Layer.GetValueInt("TextFieldIndex", m_iTextField))
		m_iTextField = -1;
	if (!Layer.GetValueInt("ColorFieldIndex", m_iColorField))
		m_iColorField = -1;

	if (!Layer.GetValueFloat("Elevation", m_fLabelHeight))
		m_fLabelHeight= 0;
	if (!Layer.GetValueFloat("LabelSize", m_fLabelSize))
		m_fLabelSize = 20;

	m_strResolved = m_strFilename;
	m_strResolved = FindFileOnPaths(datapaths, m_strResolved);
	if (m_strResolved == "")
	{
		vtString path = "PointData/";
		m_strResolved = path + m_strFilename;
		m_strResolved = FindFileOnPaths(datapaths, m_strResolved);
	}
}

void StyleDlg::GetOptions(vtTagArray &pLayer)
{
	pLayer.SetValueBool("Geometry", m_bGeometry, true);
	if (m_bGeometry)
		pLayer.SetValueRGBi("GeomColor", m_GeomColor, true);
	else
		pLayer.RemoveTag("GeomColor");

	pLayer.SetValueBool("Labels", m_bTextLabels, true);
	if (m_bTextLabels)
	{
		pLayer.SetValueRGBi("LabelColor", m_LabelColor, true);
		pLayer.SetValueBool("TextFieldIndex", m_bTextLabels, true);
		pLayer.SetValueInt("ColorFieldIndex", m_iColorField, true);
		pLayer.SetValueFloat("Elevation", m_fLabelHeight, true);
		pLayer.SetValueFloat("LabelSize", m_fLabelSize, true);
	}
	else
	{
		pLayer.RemoveTag("LabelColor");
		pLayer.RemoveTag("TextFieldIndex");
		pLayer.RemoveTag("ColorFieldIndex");
		pLayer.RemoveTag("Elevation");
		pLayer.RemoveTag("LabelSize");
	}
}

void StyleDlg::RefreshFields()
{
	GetTextField()->Clear();
	GetColorField()->Clear();
	GetColorField()->Append(_("(none)"));

	m_type = GetFeatureGeomType(m_strResolved);
	m_strFeatureType = OGRGeometryTypeToName(m_type);

	if (m_Fields.LoadFieldInfoFromDBF(m_strResolved))
	{
		int i, num = m_Fields.GetNumFields();
		for (i = 0; i < num; i++)
		{
			Field *field = m_Fields.GetField(i);
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
	GetGeomColor()->Enable(m_bGeometry);

	GetLabelColor()->Enable(m_bTextLabels);
	GetTextField()->Enable(m_bTextLabels);
	GetColorField()->Enable(m_bTextLabels && m_Fields.GetNumFields() > 1);
	GetLabelSize()->Enable(m_bTextLabels);
	GetLabelHeight()->Enable(m_bTextLabels);
}

void StyleDlg::UpdateColorButtons()
{
	FillWithColor(GetGeomColor(), m_GeomColor);
	FillWithColor(GetLabelColor(), m_LabelColor);
}

RGBi StyleDlg::AskColor(const RGBi &input)
{
	m_Colour.Set(input.r, input.g, input.b);
	m_ColourData.SetChooseFull(true);
	m_ColourData.SetColour(m_Colour);

	wxColourDialog dlg(this, &m_ColourData);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_ColourData = dlg.GetColourData();
		m_Colour = m_ColourData.GetColour();
		return RGBi(m_Colour.Red(), m_Colour.Green(), m_Colour.Blue());
	}
	else
		return RGBi(-1,-1,-1);
}

// WDR: handler implementations for StyleDlg

void StyleDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void StyleDlg::OnGeomColor( wxCommandEvent &event )
{
	RGBi result = AskColor(m_GeomColor);
	if (result.r == -1)
		return;
	m_GeomColor = result;
	UpdateColorButtons();
}

void StyleDlg::OnLabelColor( wxCommandEvent &event )
{
	RGBi result = AskColor(m_LabelColor);
	if (result.r == -1)
		return;
	m_LabelColor = result;
	UpdateColorButtons();
}

