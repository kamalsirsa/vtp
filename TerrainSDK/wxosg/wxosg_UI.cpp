///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wxosg_UI.h"

///////////////////////////////////////////////////////////////////////////

SceneGraphDlgBase::SceneGraphDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_scenetree = new wxTreeCtrl( this, ID_SCENETREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxSUNKEN_BORDER );
	bSizer11->Add( m_scenetree, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_enabled = new wxCheckBox( this, ID_ENABLED, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enabled->SetValue(true); 
	bSizer12->Add( m_enabled, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_zoomto = new wxButton( this, ID_ZOOMTO, _("Zoom To"), wxDefaultPosition, wxDefaultSize, 0 );
	m_zoomto->SetDefault(); 
	bSizer12->Add( m_zoomto, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_refresh = new wxButton( this, ID_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
	m_refresh->SetDefault(); 
	bSizer12->Add( m_refresh, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_log = new wxButton( this, ID_LOG, _("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_log->SetDefault(); 
	bSizer12->Add( m_log, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer11->Add( bSizer12, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer11 );
	this->Layout();
	bSizer11->Fit( this );
	
	this->Centre( wxBOTH );
}

SceneGraphDlgBase::~SceneGraphDlgBase()
{
}
