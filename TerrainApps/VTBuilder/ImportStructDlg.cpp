//
// Name: ImportStructDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
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

BEGIN_EVENT_TABLE(ImportStructDlg,AutoDialog)
END_EVENT_TABLE()

ImportStructDlg::ImportStructDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	ImportStructFunc( this, TRUE );
	m_iType = 0;
	m_bFlip = false;
}

// WDR: handler implementations for ImportStructDlg


void ImportStructDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddValidator(ID_TYPE_CENTER, &m_iType);
	AddValidator(ID_FLIP, &m_bFlip);
	AddValidator(ID_INSIDE_AREA, &m_bInsideOnly);

	int nShapeType = GetSHPType(m_filename);
	GetTypeCenter()->Enable(nShapeType == SHPT_POINT);
	GetTypeFootprint()->Enable(nShapeType == SHPT_POLYGON ||
		nShapeType == SHPT_POLYGONZ);
}

