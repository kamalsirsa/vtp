//
// Name: ImageMapDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImageMapDlg.h"
#include "vtdata/Features.h"
#include "vtui/wxString2.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImageMapDlg
//----------------------------------------------------------------------------

// WDR: event table for ImageMapDlg

BEGIN_EVENT_TABLE(ImageMapDlg,wxDialog)
END_EVENT_TABLE()

ImageMapDlg::ImageMapDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_iField = 0;

	// WDR: dialog function ImageMapDialogFunc for ImageMapDlg
	ImageMapDialogFunc( this, TRUE );

	AddValidator(ID_LINK_FIELD, &m_iField);
}

void ImageMapDlg::SetFields(vtFeatureSet *pSet)
{
	for (unsigned int i = 0; i < pSet->GetNumFields(); i++)
	{
		wxString2 str = pSet->GetField(i)->m_name;
		GetLinkField()->Append(str);
	}
}

// WDR: handler implementations for ImageMapDlg

