//
// Name: LinearStructDlg.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxString2.h"
#include "LinearStructDlg.h"

#define VALUE_MIN  0.2f
#define VALUE_MAX  10.2f
#define SIZE_MIN  0.05f
#define SIZE_MAX  2.55f
#define BOTTOM_MIN  -1.0f
#define BOTTOM_MAX  9.0f
#define WIDTH_MIN	0.0f
#define WIDTH_MAX	10.0f

// WDR: class implementations

//----------------------------------------------------------------------------
// LinearStructureDlg
//----------------------------------------------------------------------------

// WDR: event table for LinearStructureDlg

BEGIN_EVENT_TABLE(LinearStructureDlg, AutoDialog)
	EVT_INIT_DIALOG (LinearStructureDlg::OnInitDialog)
	EVT_CHOICE( ID_LINEAR_STRUCTURE_STYLE, LinearStructureDlg::OnStyle )
	EVT_TEXT( ID_POST_HEIGHT_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_POST_SPACING_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_POST_SIZE_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_WIDTH_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_TOP_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_BOTTOM_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_SLIDER( ID_POST_HEIGHT_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_POST_SPACING_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_POST_SIZE_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_WIDTH_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_TOP_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_BOTTOM_SLIDER, LinearStructureDlg::OnSlider )
	EVT_CHOICE( ID_POST_TYPE, LinearStructureDlg::OnPostType )
	EVT_CHOICE( ID_CONN_TYPE, LinearStructureDlg::OnConnType )
END_EVENT_TABLE()

LinearStructureDlg::LinearStructureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	LinearStructDialogFunc( this, TRUE );
	m_bSetting = false;

	m_iStyle = 0;
	m_param.Defaults();

	AddValidator(ID_LINEAR_STRUCTURE_STYLE, &m_iStyle);

	AddValidator(ID_POST_HEIGHT_SLIDER, &m_iPostHeight);
	AddValidator(ID_POST_SPACING_SLIDER, &m_iPostSpacing);
	AddValidator(ID_POST_SIZE_SLIDER, &m_iPostSize);
	AddValidator(ID_CONN_WIDTH_SLIDER, &m_iConnWidth);
	AddValidator(ID_CONN_TOP_SLIDER, &m_iConnTop);
	AddValidator(ID_CONN_BOTTOM_SLIDER, &m_iConnBottom);

	AddNumValidator(ID_POST_HEIGHT_EDIT, &m_param.m_fPostHeight, 2);
	AddNumValidator(ID_POST_SPACING_EDIT, &m_param.m_fPostSpacing, 2);
	AddNumValidator(ID_POST_SIZE_EDIT, &m_param.m_fPostWidth, 2);
//	AddNumValidator(ID_POST_SIZE_EDIT, &m_param.m_fPostDepth, 2);
	AddNumValidator(ID_CONN_WIDTH_EDIT, &m_param.m_fConnectWidth, 2);
	AddNumValidator(ID_CONN_TOP_EDIT, &m_param.m_fConnectTop, 2);
	AddNumValidator(ID_CONN_BOTTOM_EDIT, &m_param.m_fConnectBottom, 2);

	// NB -- these must match the FS_ enum in order
	GetStyle()->Clear();
	GetStyle()->Append(_("Wooden posts, wire"));
	GetStyle()->Append(_("Metal posts, wire"));
	GetStyle()->Append(_("Metal poles, chain-link"));
	GetStyle()->Append(_("Dry-stone wall"));
	GetStyle()->Append(_("Stone wall"));
	GetStyle()->Append(_("Privet hedge"));
	GetStyle()->Append(_("Railing (Pipe)"));
	GetStyle()->Append(_("Railing (Wire)"));
	GetStyle()->Append(_("Railing (EU)"));
	GetStyle()->Append(_("(custom)"));

	GetPostType()->Append(_("none"));
	GetPostType()->Append(_("wood"));
	GetPostType()->Append(_("steel"));

	GetConnType()->Append(_("none"));
	GetConnType()->Append(_("wire"));
	GetConnType()->Append(_("chain-link"));
	GetConnType()->Append(_("drystone"));
	GetConnType()->Append(_("stone"));
	GetConnType()->Append(_("privet"));
	GetConnType()->Append(_("railing_wire"));
	GetConnType()->Append(_("railing_eu"));
	GetConnType()->Append(_("railing_pipe"));
}

void LinearStructureDlg::UpdateTypes()
{
	wxString2 ws;
	ws = m_param.m_PostType;
	GetPostType()->SetStringSelection(ws);
	ws = m_param.m_ConnectType;
	GetConnType()->SetStringSelection(ws);
}

void LinearStructureDlg::UpdateEnabling()
{
	GetPostSpacingEdit()->Enable(m_param.m_PostType != "none");
	GetPostSpacingSlider()->Enable(m_param.m_PostType != "none");
	GetPostHeightEdit()->Enable(m_param.m_PostType != "none");
	GetPostHeightSlider()->Enable(m_param.m_PostType != "none");
	GetPostSizeEdit()->Enable(m_param.m_PostType != "none");
	GetPostSizeSlider()->Enable(m_param.m_PostType != "none");

	GetConnWidthEdit()->Enable(m_param.m_ConnectType != "none");
	GetConnWidthSlider()->Enable(m_param.m_ConnectType != "none");
	GetConnTopEdit()->Enable(m_param.m_ConnectType != "none");
	GetConnTopSlider()->Enable(m_param.m_ConnectType != "none");
	GetConnBottomEdit()->Enable(m_param.m_ConnectType != "none");
	GetConnBottomSlider()->Enable(m_param.m_ConnectType != "none");
}


// WDR: handler implementations for LinearStructureDlg

void LinearStructureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	ValuesToSliders();
	UpdateTypes();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	UpdateEnabling();
}

void LinearStructureDlg::OnConnType( wxCommandEvent &event )
{
	if (m_bSetting) return;

	wxString2 ws = GetConnType()->GetStringSelection();
	m_param.m_ConnectType = ws.vt_str();
	UpdateEnabling();

	m_iStyle = FS_TOTAL;	// custom
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnPostType( wxCommandEvent &event )
{
	if (m_bSetting) return;

	wxString2 ws = GetPostType()->GetStringSelection();
	m_param.m_PostType = ws.vt_str();
	UpdateEnabling();

	m_iStyle = FS_TOTAL;	// custom
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnStyle( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	vtLinearStyle style = (vtLinearStyle) m_iStyle;
	if (style != FS_TOTAL)
	{
		m_param.ApplyStyle(style);
		UpdateTypes();
		ValuesToSliders();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
		OnSetOptions(m_param);
		UpdateEnabling();
	}
}

void LinearStructureDlg::OnSlider( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	int id = event.GetId();
	SlidersToValues(id);
	m_iStyle = FS_TOTAL;	// custom
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnTextEdit( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_param.m_fPostDepth = m_param.m_fPostWidth;
	ValuesToSliders();
	m_iStyle = FS_TOTAL;	// custom
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::SlidersToValues(int which)
{
	switch (which)
	{
	case ID_POST_SPACING_SLIDER:
		m_param.m_fPostSpacing	 = VALUE_MIN + m_iPostSpacing *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_POST_HEIGHT_SLIDER:
		m_param.m_fPostHeight	 = VALUE_MIN + m_iPostHeight *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_POST_SIZE_SLIDER:
		m_param.m_fPostWidth	 = VALUE_MIN + m_iPostSize *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		m_param.m_fPostDepth	 = VALUE_MIN + m_iPostSize *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_CONN_TOP_SLIDER:
		m_param.m_fConnectTop	 = VALUE_MIN + m_iConnTop *		(VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_CONN_BOTTOM_SLIDER:
		m_param.m_fConnectBottom = BOTTOM_MIN + m_iConnBottom *	(BOTTOM_MAX - BOTTOM_MIN) / 100.0f;
		break;
	case ID_CONN_WIDTH_SLIDER:
		m_param.m_fConnectWidth  = WIDTH_MIN + m_iConnWidth *	(WIDTH_MAX - WIDTH_MIN) / 100.0f;
		break;
	}
}

void LinearStructureDlg::ValuesToSliders()
{
	m_iPostHeight =	 (int) ((m_param.m_fPostHeight - VALUE_MIN) /	 (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iPostSpacing = (int) ((m_param.m_fPostSpacing - VALUE_MIN) /	 (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iPostSize =	 (int) ((m_param.m_fPostWidth - VALUE_MIN) /	 (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iConnTop =	 (int) ((m_param.m_fConnectTop - VALUE_MIN) /	 (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iConnBottom =	 (int) ((m_param.m_fConnectBottom - BOTTOM_MIN) / (BOTTOM_MAX - BOTTOM_MIN) * 100.0f);
	m_iConnWidth =	 (int) ((m_param.m_fConnectWidth - WIDTH_MIN) /  (WIDTH_MAX - WIDTH_MIN) * 100.0f);
}

