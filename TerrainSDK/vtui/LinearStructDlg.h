//
// Name:		LinearStructDlg.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LinearStruct_H__
#define __LinearStruct_H__

#include "vtui_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Fence.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LinearStructureDlg
//----------------------------------------------------------------------------

class LinearStructureDlg: public AutoDialog
{
public:
	// constructors and destructors
	LinearStructureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void OnInitDialog(wxInitDialogEvent& event);
	void SlidersToValues();
	void ValuesToSliders();

	// Override this with desired result
	virtual void OnSetOptions(LinStructOptions &opt) = 0;

	// WDR: method declarations for LinearStructureDlg
	wxSlider* GetSpacingslider()  { return (wxSlider*) FindWindow( ID_SPACINGSLIDER ); }
	wxSlider* GetHeightslider()  { return (wxSlider*) FindWindow( ID_HEIGHTSLIDER ); }
	wxChoice* GetFencetype()  { return (wxChoice*) FindWindow( ID_TYPE ); }

private:
	// WDR: member variable declarations for LinearStructureDlg
	int		m_iType;
	int		m_iHeight;
	int		m_iSpacing;
	wxSlider	*m_pSpacingSlider;
	wxSlider	*m_pHeightSlider;
	wxChoice	*m_pFenceChoice;

	LinStructOptions m_opts;
//	FenceType m_fencetype;
//	float	m_fHeight;
//	float	m_fSpacing;

private:
	// WDR: handler declarations for LinearStructureDlg
	void OnSpacingSlider( wxCommandEvent &event );
	void OnHeightSlider( wxCommandEvent &event );
	void OnSpacingEdit( wxCommandEvent &event );
	void OnHeightEdit( wxCommandEvent &event );
	void OnFenceType( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif

