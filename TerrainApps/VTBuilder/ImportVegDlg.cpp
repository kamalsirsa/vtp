//
// Name:        ImportVegDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
    #pragma implementation "ImportVegDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "ImportVegDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportVegDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportVegDlg

BEGIN_EVENT_TABLE(ImportVegDlg,wxDialog)
    EVT_BUTTON( wxID_OK, ImportVegDlg::OnOK )
    EVT_INIT_DIALOG (ImportVegDlg::OnInitDialog)
END_EVENT_TABLE()

ImportVegDlg::ImportVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    ImportVegFunc( this ); 
    m_pcbField = GetField();
	m_pDensity = GetDensity();
	m_pBiotype1 = GetBiotype1();
	m_pBiotype2 = GetBiotype2();
}

void ImportVegDlg::SetShapefileName(const char *filename)
{
    m_filename = filename;
}

// WDR: handler implementations for ImportVegDlg

void ImportVegDlg::OnInitDialog(wxInitDialogEvent& event)
{
    // Open the SHP File
    SHPHandle hSHP = SHPOpen(m_filename, "rb");
    if (hSHP == NULL)
    {
        wxMessageBox("Couldn't open shapefile.");
        return;
    }

    // Get number of polys and type of data
    int     nElem, nShapeType;
    double  adfMinBound[4], adfMaxBound[4];
    SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

    // Check Shape Type, Veg Layer should be Poly data
    if (nShapeType != SHPT_POLYGON && nShapeType != SHPT_POINT)
    {
        wxMessageBox("Shapefile must have either point features (for individual\n"
			"plants) or polygon features (for plant distribution areas).");
        return;
    }

    // Open DBF File
    DBFHandle db = DBFOpen(m_filename, "rb");
    if (db == NULL)
    {
        wxMessageBox("Couldn't open DBF file.");
        return;
    }
    wxString str;
    int fields, i, iField, *pnWidth = 0, *pnDecimals = 0;
    DBFFieldType fieldtype;
    char pszFieldName[20];
    fields = DBFGetFieldCount(db);
    for (i = 0; i < fields; i++)
    {
        iField = i;
        fieldtype = DBFGetFieldInfo(db, iField,
            pszFieldName, pnWidth, pnDecimals );
        str.Printf("%d: ", i);
        str += pszFieldName;

        if (fieldtype == FTString)
            str += " (String)";
        if (fieldtype == FTInteger)
            str += " (Integer)";
        if (fieldtype == FTDouble)
            str += " (Double)";
        m_pcbField->Append(str);
    }
    m_pcbField->SetSelection(0);

    DBFClose(db);
    SHPClose(hSHP);

	m_pDensity->SetValue(1);

    wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

void ImportVegDlg::OnOK( wxCommandEvent &event )
{
    m_fieldindex = m_pcbField->GetSelection();

	if (m_pDensity->GetValue())
		m_datatype = 0;
	if (m_pBiotype1->GetValue())
		m_datatype = 1;
	if (m_pBiotype2->GetValue())
		m_datatype = 2;

	wxDialog::OnOK(event);
}


