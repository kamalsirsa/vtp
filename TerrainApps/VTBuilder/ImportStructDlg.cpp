//
// Name: ImportStructDlg.cpp
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ImportStructDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
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
	EVT_RADIOBUTTON( ID_TYPE_CENTER, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_FOOTPRINT, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_LINEAR, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_INSTANCE, ImportStructDlg::OnRadio )
	EVT_CHOICE( ID_CHOICE_HEIGHT_FIELD, ImportStructDlg::OnChoiceHeightField )
	EVT_CHOICE( ID_CHOICE_HEIGHT_TYPE, ImportStructDlg::OnChoiceHeightType )
	EVT_CHOICE( ID_CHOICE_FILE_FIELD, ImportStructDlg::OnChoiceFileField )
END_EVENT_TABLE()

ImportStructDlg::ImportStructDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	ImportStructFunc( this, TRUE );

	m_iType = 0;
	m_iHeightType = 0;
	m_opt.bFlip = false;
	m_opt.bInsideOnly = false;

	AddValidator(ID_FLIP, &m_opt.bFlip);
	AddValidator(ID_INSIDE_AREA, &m_opt.bInsideOnly);
	AddValidator(ID_CHOICE_HEIGHT_TYPE, &m_iHeightType);
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

void ImportStructDlg::OnChoiceHeightType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_opt.m_HeightType = (StructImportOptions::HeightType) m_iHeightType;
}

void ImportStructDlg::OnChoiceHeightField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString2 str = GetChoiceHeightField()->GetStringSelection();
	m_opt.m_strFieldNameHeight = str.mb_str();
}

void ImportStructDlg::OnRadio( wxCommandEvent &event )
{
	if (GetRadio(ID_TYPE_CENTER)) m_iType = 0;
	if (GetRadio(ID_TYPE_FOOTPRINT)) m_iType = 1;
	if (GetRadio(ID_TYPE_LINEAR)) m_iType = 2;
	if (GetRadio(ID_TYPE_INSTANCE)) m_iType = 3;
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
		m_iType = 0;
	}
	else
	if (GetTypeFootprint()->IsEnabled())
	{
		GetTypeFootprint()->SetValue(true);
		m_iType = 1;
	}
	else
	if (GetTypeLinear()->IsEnabled())
	{
		GetTypeLinear()->SetValue(true);
		m_iType = 2;
	}
	UpdateEnables();

	GetChoiceHeightField()->Append(_("(none)"));
	GetChoiceFileField()->Append(_("(none)"));

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
		}
	}
	GetChoiceFileField()->SetSelection(0);
	GetChoiceHeightField()->SetSelection(0);

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

	GetFlip()->Enable(m_iType == 1);
	GetChoiceHeightField()->Enable(m_iType == 1);
	GetChoiceHeightType()->Enable(m_iType == 1);

	GetChoiceFileField()->Enable(m_iType == 3);
}

