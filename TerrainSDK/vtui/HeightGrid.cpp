/////////////////////////////////////////////////////////////////////////////
// Name:		HeightGrid.cpp
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
	#pragma implementation "HeightGrid.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "HeightGrid.h"
#include "HeightDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CHeightGrid
//----------------------------------------------------------------------------

// WDR: event table for CHeightGrid

BEGIN_EVENT_TABLE(CHeightGrid,wxGrid)
	EVT_GRID_CELL_LEFT_CLICK( CHeightGrid::OnLeftClick )
	EVT_GRID_EDITOR_HIDDEN( CHeightGrid::OnGridEditorHidden )
END_EVENT_TABLE()

CHeightGrid::CHeightGrid( wxWindow *parent, wxWindowID id,
	const wxPoint &position, const wxSize& size, long style ) :
	wxGrid( parent, id, position, size, style )
{
}

void CHeightGrid::Setup(CHeightDialog *pParent)
{
	m_pParentDialog = pParent;
}

// WDR: handler implementations for CHeightGrid

void CHeightGrid::OnGridEditorHidden( wxGridEvent &event )
{
	m_pParentDialog->OnGridEditorHidden( event );
}

void CHeightGrid::OnLeftClick( wxGridEvent &event )
{
	m_pParentDialog->OnLeftClickGrid( event );
}




