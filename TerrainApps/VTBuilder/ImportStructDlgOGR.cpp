//
// Name: ImportStructDlgOGR.cpp
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ImportStructDlgOGR.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "ImportStructDlgOGR.h"
#include "StructLayer.h"

#include "ogrsf_frmts.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportStructDlgOGR
//----------------------------------------------------------------------------

// WDR: event table for ImportStructDlgOGR

BEGIN_EVENT_TABLE(ImportStructDlgOGR, AutoDialog)
	EVT_INIT_DIALOG (ImportStructDlgOGR::OnInitDialog)
	EVT_RADIOBUTTON( ID_TYPE_BUILDING, ImportStructDlgOGR::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_LINEAR, ImportStructDlgOGR::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_INSTANCE, ImportStructDlgOGR::OnRadio )
	EVT_CHOICE( ID_CHOICE_HEIGHT_FIELD, ImportStructDlgOGR::OnChoiceHeightField )
	EVT_CHOICE( ID_CHOICE_FILE_FIELD, ImportStructDlgOGR::OnChoiceFileField )
	EVT_CHOICE( ID_ELEVATION_FIELDNAME, ImportStructDlgOGR::OnChoiceElevationFieldname )
	EVT_CHOICE( ID_LAYERNAME, ImportStructDlgOGR::OnChoiceLayerName )
END_EVENT_TABLE()

ImportStructDlgOGR::ImportStructDlgOGR( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	ImportStructFuncOGR( this, TRUE ); 

	m_opt.m_HeightType = StructImportOptions::METERS;
	m_opt.m_ElevationType = StructImportOptions::ETMETERS;
	m_opt.bFlip = false;
	m_opt.bInsideOnly = false;
	m_opt.bBuildFoundations = false;;
	m_opt.bUse25DForElevation = false;

	AddValidator(ID_INSIDE_AREA, &m_opt.bInsideOnly);
	AddValidator(ID_FLIP, &m_opt.bFlip);
	AddValidator(ID_BUILD_FOUNDATIONS, &m_opt.bBuildFoundations);
	AddValidator(ID_USE_25D, &m_opt.bUse25DForElevation);
	AddValidator(ID_CHOICE_HEIGHT_TYPE, (int *)&m_opt.m_HeightType);
	AddValidator(ID_ELEVATION_UNITS, (int *)&m_opt.m_ElevationType);
}

bool ImportStructDlgOGR::GetRadio(int id)
{
	wxRadioButton *button = (wxRadioButton*) FindWindow(id);
	if (!button)
		return false;
	return button->GetValue();
}


// WDR: handler implementations for ImportStructDlgOGR

void ImportStructDlgOGR::OnChoiceLayerName( wxCommandEvent &event )
{
	wxString2 str = GetLayername()->GetStringSelection();
	m_opt.m_strLayerName = str.mb_str();
	UpdateFieldNames();
	UpdateEnables();
}

void ImportStructDlgOGR::OnChoiceElevationFieldname( wxCommandEvent &event )
{
	wxString2 str = GetElevationFieldname()->GetStringSelection();
	m_opt.m_strFieldNameElevation = str.mb_str();
}

void ImportStructDlgOGR::OnChoiceFileField( wxCommandEvent &event )
{
	wxString2 str = GetChoiceFileField()->GetStringSelection();
	m_opt.m_strFieldNameFile = str.mb_str();
}

void ImportStructDlgOGR::OnChoiceHeightField( wxCommandEvent &event )
{
	wxString2 str = GetChoiceHeightField()->GetStringSelection();
	m_opt.m_strFieldNameHeight = str.mb_str();
}

void ImportStructDlgOGR::OnRadio( wxCommandEvent &event )
{
	if (GetRadio(ID_TYPE_BUILDING))
		m_iType = 1;
	if (GetRadio(ID_TYPE_LINEAR))
		m_iType = 2;
	if (GetRadio(ID_TYPE_INSTANCE))
		m_iType = 3;
	UpdateEnables();
}

void ImportStructDlgOGR::OnInitDialog(wxInitDialogEvent& event)
{
	// Select one of the radio buttons, whichever is enabled
	m_iType = 1;
	GetTypeBuilding()->SetValue(true);

	UpdateEnables();

	int iNumLayers = m_pDatasource->GetLayerCount();
	for (int i = 0 ; i < iNumLayers; i++)
	{
		wxString2 str = m_pDatasource->GetLayer(i)->GetLayerDefn()->GetName();
		GetLayername()->Append(str);
	}
	GetLayername()->Enable(iNumLayers > 1);
	GetLayername()->SetSelection(0);

	// Pete Willemsen - I'm not sure, but the gcc 3.2.X compilers
	// don't like the form of this that creates a temporary???
	// Replaced: OnChoiceFileField(wxCommandEvent());
	wxCommandEvent wce = wxCommandEvent();
	OnChoiceLayerName( wce );

	UpdateFieldNames();
	UpdateEnables();

	TransferDataToWindow();
}

void ImportStructDlgOGR::UpdateFieldNames()
{
	OGRLayer *pLayer;
	OGRFeatureDefn *pFeatureDefn;
	OGRFieldDefn *pFieldDefn;
	OGRFieldType FieldType;
	int iNumFields;
	int i;
	int iCount;
	wxString2 str;
	
	pLayer = m_pDatasource->GetLayer(GetLayername()->GetSelection());

	if (NULL != pLayer)
	{
		pFeatureDefn = pLayer->GetLayerDefn();

		if (NULL != pFeatureDefn)
		{
			iNumFields = pFeatureDefn->GetFieldCount();

			GetUse25d()->Enable((pFeatureDefn->GetGeomType() & wkb25DBit) > 0);

			iCount = GetChoiceHeightField()->GetCount();
			for (i = 0; i < iCount; i++)
				GetChoiceHeightField()->Delete(0);
			iCount = GetElevationFieldname()->GetCount();
			for (i = 0; i < iCount; i++)
				GetElevationFieldname()->Delete(0);
			iCount = GetChoiceFileField()->GetCount();
			for (i = 0; i < iCount; i++)
				GetChoiceFileField()->Delete(0);

			GetChoiceHeightField()->Append(_("(none)"));
			GetChoiceFileField()->Append(_("(none)"));
			GetElevationFieldname()->Append(_("(none)"));

			for (i = 0; i < iNumFields; i++)
			{
				pFieldDefn = pFeatureDefn->GetFieldDefn(i);
				if (NULL != pFieldDefn)
				{
					FieldType = pFieldDefn->GetType();
					if ((FieldType == OFTInteger) || (FieldType == OFTReal))
					{
						str = pFieldDefn->GetNameRef();
						GetChoiceHeightField()->Append(str);
						str = pFieldDefn->GetNameRef();
						GetElevationFieldname()->Append(str);
					}
					else if ((FieldType == OFTString) || (FieldType == OFTWideString))
					{
						str = pFieldDefn->GetNameRef();
						GetChoiceFileField()->Append(str);
					}
				}
			}
		}
	}

	// TODO: get rid of this yucky hack of passing bogus command events!
	wxCommandEvent wce = wxCommandEvent();

	GetChoiceFileField()->SetSelection(0);
	OnChoiceFileField(wce);
	GetChoiceHeightField()->SetSelection(0);
	OnChoiceHeightField(wce);
	GetElevationFieldname()->SetSelection(0);
	OnChoiceElevationFieldname(wce);
}

void ImportStructDlgOGR::UpdateEnables()
{
	GetBuildFoundations()->Enable(m_iType == 1);
	GetChoiceHeightField()->Enable(GetChoiceHeightField()->GetCount() > 1);
	GetElevationFieldname()->Enable(GetElevationFieldname()->GetCount() > 1);
	GetChoiceFileField()->Enable((GetChoiceFileField()->GetCount() > 1) && (m_iType == 3));
}

