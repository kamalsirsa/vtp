//
// Name: ChunkDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ChunkDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ChunkDlg
//----------------------------------------------------------------------------

// WDR: event table for ChunkDlg

BEGIN_EVENT_TABLE(ChunkDlg,AutoDialog)
END_EVENT_TABLE()

ChunkDlg::ChunkDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function ChunkDialogFunc for ChunkDlg
	ChunkDialogFunc( this, TRUE );
	AddNumValidator(ID_DEPTH, &m_iDepth);
	AddNumValidator(ID_MAXERROR, &m_fMaxError);
}

// WDR: handler implementations for ChunkDlg

