//
// Name: LayerPropDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "LayerPropDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// LayerPropDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerPropDlg

BEGIN_EVENT_TABLE(LayerPropDlg, AutoDialog)
END_EVENT_TABLE()

LayerPropDlg::LayerPropDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	LayerPropDialogFunc( this, TRUE );

	AddNumValidator(this, ID_LEFT, &m_fLeft);
	AddNumValidator(this, ID_TOP, &m_fTop);
	AddNumValidator(this, ID_RIGHT, &m_fRight);
	AddNumValidator(this, ID_BOTTOM, &m_fBottom);
	AddValidator(this, ID_PROPS, &m_strText);
}

// WDR: handler implementations for LayerPropDlg

