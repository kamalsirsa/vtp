/////////////////////////////////////////////////////////////////////////////
// Name:		HeightGrid.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __HeightGrid_H__
#define __HeightGrid_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "HeightGrid.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "vtui_wdr.h"

class CHeightDialog;

// WDR: class declarations

//---------------------------------------------------------------------------
// CHeightGrid
//---------------------------------------------------------------------------

class CHeightGrid: public wxGrid
{
public:
	// constructors and destructors
	CHeightGrid( wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0 );

	void Setup(CHeightDialog *pParent);

	// WDR: method declarations for CHeightGrid

private:
	// WDR: member variable declarations for CHeightGrid

private:
	// WDR: handler declarations for CHeightGrid
	void OnGridEditorHidden( wxGridEvent &event );
	void OnLeftClick( wxGridEvent &event );

private:
	CHeightDialog *m_pParentDialog;
	DECLARE_EVENT_TABLE()
};




#endif
