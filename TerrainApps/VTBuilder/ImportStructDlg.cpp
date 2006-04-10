//
// Name: ImportStructDlg.cpp
//
// Copyright (c) 2003-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImportStructDlg.h"
#include "StructLayer.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportStructDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportStructDlg

BEGIN_EVENT_TABLE(ImportStructDlg, AutoDialog)
	EVT_INIT_DIALOG (ImportStructDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_TYPE_LINEAR, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_INSTANCE, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_CENTER, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_FOOTPRINT, ImportStructDlg::OnRadio )

	EVT_RADIOBUTTON( ID_RADIO_ROOF_DEFAULT, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ROOF_SINGLE, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ROOF_FIELD, ImportStructDlg::OnRadio )

	EVT_CHOICE( ID_CHOICE_FILE_FIELD, ImportStructDlg::OnChoiceFileField )
	EVT_CHOICE( ID_CHOICE_HEIGHT_FIELD, ImportStructDlg::OnChoiceHeightField )
	EVT_CHOICE( ID_CHOICE_HEIGHT_TYPE, ImportStructDlg::OnChoiceHeightType )
	EVT_CHOICE( ID_CHOICE_ROOF_TYPE, ImportStructDlg::OnChoiceRoofType )
	EVT_CHOICE( ID_CHOICE_ROOF_FIELD, ImportStructDlg::OnChoiceRoofField )
END_EVENT_TABLE()

ImportStructDlg::ImportStructDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	ImportStructFunc( this, TRUE );

	m_iType = 0;
	m_iHeightType = 0;
	m_iRoofType = 0;
	m_opt.bInsideOnly = false;

	AddValidator(ID_INSIDE_AREA, &m_opt.bInsideOnly);
	AddValidator(ID_CHOICE_HEIGHT_TYPE, &m_iHeightType);
	AddValidator(ID_CHOICE_ROOF_TYPE, &m_iRoofType);
}

bool ImportStructDlg::GetRadio(int id)
{
	wxRadioButton *button = (wxRadioButton*) FindWindow(id);
	if (!button)
		return false;
	return button->GetValue();
}


// WDR: handler implementations for ImportStructDlg

void ImportStructDlg::OnChoiceFileField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString2 str = GetChoiceFileField()->GetStringSelection();
	m_opt.m_strFieldNameFile = str.mb_str();
}

void ImportStructDlg::OnChoiceHeightField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString2 str = GetChoiceHeightField()->GetStringSelection();
	m_opt.m_strFieldNameHeight = str.mb_str();
}

void ImportStructDlg::OnChoiceHeightType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_opt.m_HeightType = (StructImportOptions::HeightType) m_iHeightType;
}

void ImportStructDlg::OnChoiceRoofType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_opt.m_eRoofType = (RoofType) m_iRoofType;
}

void ImportStructDlg::OnChoiceRoofField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString2 str = GetChoiceRoofField()->GetStringSelection();
	m_opt.m_strFieldNameRoof = str.mb_str();
}

void ImportStructDlg::OnRadio( wxCommandEvent &event )
{
	if (GetRadio(ID_TYPE_LINEAR)) m_iType = 0;
	if (GetRadio(ID_TYPE_INSTANCE)) m_iType = 1;
	if (GetRadio(ID_TYPE_CENTER)) m_iType = 2;
	if (GetRadio(ID_TYPE_FOOTPRINT)) m_iType = 3;

	if (GetRadio(ID_RADIO_ROOF_DEFAULT)) m_iRoofMode = 0;
	if (GetRadio(ID_RADIO_ROOF_SINGLE)) m_iRoofMode = 1;
	if (GetRadio(ID_RADIO_ROOF_FIELD)) m_iRoofMode = 2;

	UpdateEnables();
}

void ImportStructDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_nShapeType = GetSHPType(m_filename.mb_str());
	UpdateEnables();

	// Select one of the radio buttons, whichever is enabled
	if (GetTypeCenter()->IsEnabled())
	{
		GetTypeCenter()->SetValue(true);
		m_iType = 2;
	}
	else
	if (GetTypeFootprint()->IsEnabled())
	{
		GetTypeFootprint()->SetValue(true);
		m_iType = 3;
	}
	else
	if (GetTypeLinear()->IsEnabled())
	{
		GetTypeLinear()->SetValue(true);
		m_iType = 0;
	}
	UpdateEnables();

	GetChoiceHeightField()->Append(_("(none)"));
	GetChoiceFileField()->Append(_("(none)"));
	GetChoiceRoofField()->Append(_("(none)"));

	// Open DBF File
	DBFHandle db = DBFOpen(m_filename.mb_str(), "rb");
	if (db != NULL)
	{
		// Fill the DBF field names into the "Use Field" controls
		int *pnWidth = 0, *pnDecimals = 0;
		char pszFieldName[20];
		int iFields = DBFGetFieldCount(db);
		wxString2 str;
		int i;
		for (i = 0; i < iFields; i++)
		{
			DBFFieldType fieldtype = DBFGetFieldInfo(db, i,
				pszFieldName, pnWidth, pnDecimals );
			str = pszFieldName;

			if (fieldtype == FTString)
				GetChoiceFileField()->Append(str);
			if (fieldtype == FTInteger || fieldtype == FTDouble)
				GetChoiceHeightField()->Append(str);
			if (fieldtype == FTString)
				GetChoiceRoofField()->Append(str);
		}
	}
	GetChoiceFileField()->SetSelection(0);
	GetChoiceHeightField()->SetSelection(0);
	GetChoiceRoofField()->SetSelection(0);

	TransferDataToWindow();
}

void ImportStructDlg::UpdateEnables()
{
	GetTypeCenter()->Enable(m_nShapeType == SHPT_POINT);

	GetTypeFootprint()->Enable(m_nShapeType == SHPT_POLYGON ||
		m_nShapeType == SHPT_POLYGONZ || m_nShapeType == SHPT_ARC);

	GetTypeLinear()->Enable(m_nShapeType == SHPT_ARC ||
		m_nShapeType == SHPT_POLYGON);

	GetTypeInstance()->Enable(m_nShapeType == SHPT_POINT ||
		m_nShapeType == SHPT_POINTZ);

	GetChoiceHeightField()->Enable(m_iType == 3);
	GetChoiceHeightType()->Enable(m_iType == 3);

	GetChoiceFileField()->Enable(m_iType == 1);

	GetRadioRoofDefault()->Enable(m_iType == 3);
	GetRadioRoofSingle()->Enable(m_iType == 3);
	GetRadioRoofField()->Enable(m_iType == 3);

	GetChoiceRoofType()->Enable(m_iType == 3 && m_iRoofMode == 1);
	GetChoiceRoofField()->Enable(m_iType == 3 && m_iRoofMode == 2);

	if (m_iType != 3 || m_iRoofMode != 1)
		m_opt.m_eRoofType = ROOF_UNKNOWN;

	if (m_iType != 3 || m_iRoofMode != 2)
		m_opt.m_strFieldNameRoof = "";

	if (m_iType == 0) m_opt.type = ST_LINEAR;
	if (m_iType == 1) m_opt.type = ST_INSTANCE;
	if (m_iType == 2) m_opt.type = ST_BUILDING;
	if (m_iType == 3) m_opt.type = ST_BUILDING;
}

