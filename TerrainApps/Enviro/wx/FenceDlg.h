//
// Name:		FenceDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FenceDlg_H__
#define __FenceDlg_H__

#ifdef __GNUG__
	#pragma interface "FenceDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "AutoDialog.h"
#include "vtlib/core/Fence3d.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// FenceDlg
//----------------------------------------------------------------------------

class FenceDlg: public AutoDialog
{
public:
	// constructors and destructors
	FenceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void OnInitDialog(wxInitDialogEvent& event);
	void SlidersToValues();
	void ValuesToSliders();

	// WDR: method declarations for FenceDlg
	wxSlider* GetSpacingslider()  { return (wxSlider*) FindWindow( ID_SPACINGSLIDER ); }
	wxSlider* GetHeightslider()  { return (wxSlider*) FindWindow( ID_HEIGHTSLIDER ); }
	wxChoice* GetFencetype()  { return (wxChoice*) FindWindow( ID_FENCETYPE ); }

private:
	// WDR: member variable declarations for FenceDlg
	int		m_iType;
	float	m_fHeight;
	float	m_fSpacing;
	int		m_iHeight;
	int		m_iSpacing;
	FenceType m_fencetype;
	wxSlider	*m_pSpacingSlider;
	wxSlider	*m_pHeightSlider;
	wxChoice	*m_pFenceChoice;

private:
	// WDR: handler declarations for FenceDlg
	void OnSpacingSlider( wxCommandEvent &event );
	void OnHeightSlider( wxCommandEvent &event );
	void OnSpacingEdit( wxCommandEvent &event );
	void OnHeightEdit( wxCommandEvent &event );
	void OnFenceType( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
