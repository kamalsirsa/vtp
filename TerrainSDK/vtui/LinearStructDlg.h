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
	void SlidersToValues(int which);
	void ValuesToSliders();
	void UpdateTypes();
	void UpdateEnabling();

	// Override this with desired result
	virtual void OnSetOptions(const vtLinearParams &param) = 0;

	// WDR: method declarations for LinearStructureDlg
	wxTextCtrl* GetConnBottomEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_BOTTOM_EDIT ); }
	wxTextCtrl* GetConnTopEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_TOP_EDIT ); }
	wxTextCtrl* GetConnWidthEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_WIDTH_EDIT ); }
	wxTextCtrl* GetPostSizeEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_SIZE_EDIT ); }
	wxTextCtrl* GetPostHeightEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_HEIGHT_EDIT ); }
	wxTextCtrl* GetPostSpacingEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_SPACING_EDIT ); }

	wxSlider* GetConnBottomSlider()  { return (wxSlider*) FindWindow( ID_CONN_BOTTOM_SLIDER ); }
	wxSlider* GetConnTopSlider()  { return (wxSlider*) FindWindow( ID_CONN_TOP_SLIDER ); }
	wxSlider* GetConnWidthSlider()  { return (wxSlider*) FindWindow( ID_CONN_WIDTH_SLIDER ); }
	wxSlider* GetPostSizeSlider()  { return (wxSlider*) FindWindow( ID_POST_SIZE_SLIDER ); }
	wxSlider* GetPostHeightSlider()  { return (wxSlider*) FindWindow( ID_POST_HEIGHT_SLIDER ); }
	wxSlider* GetPostSpacingSlider()  { return (wxSlider*) FindWindow( ID_POST_SPACING_SLIDER ); }

	wxChoice* GetConnType()  { return (wxChoice*) FindWindow( ID_CONN_TYPE ); }
	wxChoice* GetPostType()  { return (wxChoice*) FindWindow( ID_POST_TYPE ); }
	wxChoice* GetStyle()  { return (wxChoice*) FindWindow( ID_LINEAR_STRUCTURE_STYLE ); }

protected:
	// WDR: member variable declarations for LinearStructureDlg
	int	 m_iStyle;
	int	 m_iPostHeight;
	int	 m_iPostSpacing;
	int	 m_iPostSize;
	int	 m_iConnWidth;
	int	 m_iConnTop;
	int	 m_iConnBottom;

	bool m_bSetting;

	vtLinearParams m_param;

private:
	// WDR: handler declarations for LinearStructureDlg
	void OnTextEdit( wxCommandEvent &event );
	void OnConnType( wxCommandEvent &event );
	void OnPostType( wxCommandEvent &event );
	void OnSlider( wxCommandEvent &event );
	void OnStyle( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LinearStruct_H__

